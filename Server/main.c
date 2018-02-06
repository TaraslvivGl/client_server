#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>

#define BUFLEN (512)
#define PORT (8888)

int running = 1;

void errorLogExit(char *s) {
    perror(s);
    exit(1);
}

void *exitOnRequest(void* param) {
    char input[6];
    memset(input, 0 , sizeof(input));

    do {
        fgets(input, 7, stdin);
    }while((strlen(input) != 5) || (strncmp(input, "exit", 4) != 0));

    printf("Server is shutting down...\n");

    running = 0;
}

int main(void) {
    pthread_t tid;

    struct sockaddr_in server;
    struct sockaddr_in client;
    int sockedFd;
    int slen = sizeof(client);
    int ret = 0;
    size_t needed_mem = 0;
    char* buf = malloc(BUFLEN);
    char* printBuf = NULL;

    if ((sockedFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        errorLogExit("Socket creation failed");
    }

    pthread_create(&tid, NULL, exitOnRequest, NULL);

    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockedFd, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        close(sockedFd);
        errorLogExit("Socket is used already\n");
    }

    memset(buf, 0, BUFLEN);

    struct pollfd outFd[1];
    outFd->fd = sockedFd;
    outFd->events = POLLIN;

    printf("Running Server...\n");
    while(running) {
        if (poll(outFd, 1, 1000) && (outFd[0].revents & POLLIN)) {
            if ((ret = recvfrom(sockedFd, buf, BUFLEN, 0, (struct sockaddr *) &client, &slen)) < 0) {
                close(sockedFd);
                errorLogExit("Receiving package failed");
            }

            needed_mem = snprintf(NULL, 0, "%s", buf) + 1;
            printBuf = realloc(printBuf, needed_mem);
            snprintf(printBuf, BUFLEN, "%s", buf);
            printf("Received package: %s\n", buf);

            memset(buf, 0, strlen(buf));
            memset(printBuf, 0, strlen(printBuf));
        }
    }

    if(printBuf != NULL){
        free(printBuf);
    }
    free(buf);
    close(sockedFd);
    return 0;
}