#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <netinet/in.h>
//#include<sys/socket.h>
#include<unistd.h>

#define BUFLEN 512
#define PORT 8888

void errorLog(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    struct sockaddr_in server;
    int sockedFd;
    char buf[BUFLEN];
    memset(&buf[0], 0, sizeof(buf));
    if ((sockedFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        errorLog("Socket creation failed");
    }

    memset((char *) &server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockedFd, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        errorLog("Socket is used already");
    }

    printf("Running Server...\n");
    while(1)
    {
        fflush(stdout);
        if (recv(sockedFd, buf, BUFLEN, 0) < 0)
        {
            errorLog("Receiving package failed");
        }

        printf("Received package: %s\n" , buf);
        memset(&buf[0], 0, sizeof(buf));
    }
    close(sockedFd);
    return 0;
}