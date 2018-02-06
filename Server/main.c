#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <sys/eventfd.h>

#define BUFLEN (512)
#define PORT (8888)

int exitEventFd;

void errorLogExit(char *s) {
    perror(s);
    exit(1);
}

void *exitOnRequest(void* args) {
    char input[sizeof("exit")];
    memset(input, 0 , sizeof(input));

    do {
        fgets(input, sizeof("exit\n"), stdin);
    } while(strncmp(input, "exit\n", strlen("exit\n")) != 0);

    printf("Server is shutting down...\n");

    uint64_t trigger = 1;
    write(exitEventFd, &trigger, sizeof(trigger));
}

int main(void) {

    pthread_t tid;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sockedFd;
    int clientLen = sizeof(client);
    char* buf = malloc(BUFLEN);
    char* printBuf = malloc(BUFLEN);;

    if ((sockedFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        errorLogExit("Socket creation failed");
    }

    pthread_create(&tid, NULL, exitOnRequest, NULL);

    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockedFd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        close(sockedFd);
        errorLogExit("Socket is used already\n");
    }

    memset(buf, 0, BUFLEN);

    exitEventFd = eventfd(0, 0);
    struct pollfd outFd[2];
    outFd[0].fd = sockedFd;
    outFd[0].events = POLLIN;

    outFd[1].fd = exitEventFd;
    outFd[1].events = POLLIN;

    printf("Running Server...\n");
    while(1) {

        if (poll(outFd, 2, -1)) {

            if (outFd[1].revents & POLLIN) {
                break;
            }

            if (outFd[0].revents & POLLIN) {
                if ((recvfrom(sockedFd, buf, BUFLEN, 0, (struct sockaddr *) &client, &clientLen)) < 0) {
                    close(sockedFd);
                    errorLogExit("Receiving package failed");
                }

                snprintf(printBuf, BUFLEN, "%s", buf);
                printf("Received package: %s\n", buf);

                memset(buf, 0, strlen(buf));
                memset(printBuf, 0, strlen(printBuf));
            }
        }
    }

    free(printBuf);
    free(buf);
    close(sockedFd);
    close(exitEventFd);
    return 0;
}