#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("Usage %s <IP> <PORT> <MESSAGE>\n" , argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *message = argv[3];

    int sockfd;
    struct sockaddr_in server_addr;

    // 1. create socket
    sockfd = socket(AF_INET,SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("Socket failed");
        return 1;
    }

    // 2. setup the destination
    memset(&server_addr, 0 , sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET,ip,&server_addr.sin_addr);

    // 3. send message
    sendto(sockfd,message, strlen(message), 0 , (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Message send\n");

    close(sockfd);
    return 0;

}