#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char*argv[]){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd;
    //char buffer[BUFFER_SIZE];
    Packet pkt;
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

    int expected_seq = 1;
    srand(time(NULL) + rand());

    while(1){
        // receiver message
        //int n = recvfrom(sockfd,buffer, BUFFER_SIZE,0 , (struct sockaddr*)&client_addr, &addr_len);
        int n = recvfrom(sockfd,&pkt,sizeof(pkt),0,(struct sockaddr*)&client_addr,&addr_len);

        if(n < 0){
            printf("recvfrom failed");
            continue;
        }

        // packet drop
        int drop = rand() % 100;

        if(drop < 90){
            printf("Simulating packet drop for seq %d\n",pkt.sequence);
            continue;   
        }
        

        if(pkt.sequence != expected_seq){
            printf("Packet Loss detected! Expected %d but got %d\n",expected_seq,pkt.sequence);
            expected_seq = pkt.sequence + 1;
        }else{
            expected_seq++;
        }

        printf("\nReceived Packet\n");
        printf("Sequence %d\n", pkt.sequence);
        printf("Timestamp %ld\n", pkt.timestamp);
        printf("Message %s\n", pkt.message);

        printf("From ip: %s\n" , inet_ntoa(client_addr.sin_addr));
        printf("From port : %d\n", ntohs(client_addr.sin_port));
    }

    close(sockfd);
    return 0;
}