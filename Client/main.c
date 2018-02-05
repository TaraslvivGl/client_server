#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER "127.0.0.1"
#define BUFLEN (512)
#define PORT (8888)
#define DATA_COUTN (1)

void errorLog(const char* const s, const int socket, char** data)
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

    exit(0);
}

int main(void)
{
    struct sockaddr_in server;
    int servLen = sizeof(server);
    int sockedFd = -1;
    char* message = malloc(BUFLEN);
    char** data = malloc(DATA_COUTN * sizeof(char*));
    int ret = 0;

    data[0] = message;

    /*Ctrl+C exit*/
    void exitHadler()
    {
        exitOnRequest(data, sockedFd);
    }
    signal(SIGINT, exitHadler);

    if ((sockedFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        errorLog("Error socket initialization", sockedFd, data);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    /*connection to server*/
    if (connect(sockedFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        errorLog("Cannot connect to server\n", sockedFd, data);
    }

    if(!inet_aton(SERVER, &server.sin_addr))
    {
        errorLog("Invalid IP Address \n", sockedFd, data);
    }

    size_t msgCount = 1;
    int lastPkg = 1;
    while (1)
    {
        //last package has been sent
        if (lastPkg)
        {
            printf("Enter message: ");
            msgCount = 1;
        }

        if ((fgets(message, BUFLEN, stdin)) != NULL)
        {
            //trim character '\n' from the last buffer from stdin
            if(message[strlen(message) - 1] == '\n')
            {
                message[strlen(message) - 1] = '\0';
                lastPkg = 1;
            }
            else
            {
                lastPkg = 0;
            }

            if ((strlen(message) != 0) && ((ret = send(sockedFd, message, strlen(message), 0)) < 0))
            {
                errorLog("Cannot send message\n", sockedFd, data);
            }
            else if ((strlen(message) != 0))
            {
                printf("Package(%lu) has been sent %d bytes of %lu\n", msgCount, ret, strlen(message));
                msgCount++;
            }
        }
    }
}