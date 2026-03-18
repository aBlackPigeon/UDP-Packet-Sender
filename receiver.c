#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char*argv[]){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // 1. create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("Socket Failed");
        return 1;
    }

    // 2. setup the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // 3. bind

    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind Failed");
        return 1;
    }

    printf("Listening on port %d ... \n" , port);

    while(1){
        // receiver message
        int n = recvfrom(sockfd,buffer, BUFFER_SIZE,0 , (struct sockaddr*)&client_addr, &addr_len);

        buffer[n] = '\0';

        printf("\nReceived message %s\n", buffer);
        printf("From ip: %s\n" , inet_ntoa(client_addr.sin_addr));
        printf("From port : %d\n", ntohs(client_addr.sin_port));
    }

    close(sockfd);
    return 0;
}