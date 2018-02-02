#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <arpa/inet.h>
//#include <sys/socket.h>
#include<unistd.h>


#define SERVER "127.0.0.1"
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
    char message[BUFLEN];

    if ( (sockedFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        errorLog("Error socket initialization");
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sockedFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        errorLog("Cannot connect to server");
    }

    if(!inet_aton(SERVER, &server.sin_addr))
    {
        errorLog("Cannot convert address into network byte order\n");
    }

    while(1)
    {
        printf("Enter message : ");
        scanf("%s", message);

        if (send(sockedFd, message, strlen(message), 0) < 0)
        {
            errorLog("Cannot send message\n");
        }
    }

    close(sockedFd);
    return 0;
}