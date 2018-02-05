#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#define SERVER "127.0.0.1"
#define BUFLEN (512)
#define PORT (8888)
#define DATA_COUTN (1)

int running = 1;

void errorLogExit(const char* const s, const int socket, char** data)
{
    int i;
    for (i = 0; i < DATA_COUTN; i++)
    {
        free(data[i]);
    }
    free(data);

    if (socket > 0)
    {
        close(socket);
    }
    perror(s);
    exit(1);
}

void exitOnRequest(char** data, const int socket)
{
    int i;
    for (i = 0; i < DATA_COUTN; i++)
    {
        if(data[i] != NULL)
        {
            free(data[i]);
        }
    }
    free(data);

    printf("\nConnection closed\n");

    if (socket > 0)
    {
        close(socket);
    }

    running = 0;
}

int main(void)
{
    struct sockaddr_in server;
    int servLen = sizeof(server);
    int sockedFd = -1;
    char* message = malloc(BUFLEN);
    char** data = malloc(DATA_COUTN * sizeof(char*));
    int ret = 0;

    //data which will be cleaned on termination
    data[0] = message;

    /*Ctrl+C exit*/
    void exitHadler()
    {
        exitOnRequest(data, sockedFd);
    }
    signal(SIGINT, exitHadler);

    if ((sockedFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        errorLogExit("Error socket initialization", sockedFd, data);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    /*connection to server*/
    if (connect(sockedFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        errorLogExit("Cannot connect to server\n", sockedFd, data);
    }

    if(!inet_aton(SERVER, &server.sin_addr))
    {
        errorLogExit("Invalid IP Address \n", sockedFd, data);
    }

    struct pollfd outFd[1];
    outFd->fd = fileno(stdin);
    outFd->events = POLLIN;

    printf("Message to sent:\n");
    while (running) {
        if ((poll(outFd, 1, 1000)) && (outFd[0].revents & POLLIN)) {
            fgets(message, BUFLEN, stdin);
            //trim character '\n' from the last buffer from stdin
            if (message[strlen(message) - 1] == '\n') {
                message[strlen(message) - 1] = '\0';
            }

            if ((ret = send(sockedFd, message, strlen(message), 0)) < 0) {
                errorLogExit("Cannot send message\n", sockedFd, data);
            } else {
                printf("%d bytes have been sent\n", ret);
            }
        }
    }
    return 0;
}