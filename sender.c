#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"

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

    // socket timeout
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    setsockopt(sockfd, SOL_SOCKET,SO_RCVTIMEO, &timeout,sizeof(timeout));

    // 2. setup the destination
    memset(&server_addr, 0 , sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET,ip,&server_addr.sin_addr);

    //Packet structure
    Packet pkt;
    // pkt.sequence = 1;
    // static int seq = 1;
    // pkt.sequence = seq++;
    // pkt.timestamp = time(NULL);
    
    strncpy(pkt.message,message,MAX_MSG_SIZE);

    // 3. send message
    // sendto(sockfd,message, strlen(message), 0 , (struct sockaddr*)&server_addr, sizeof(server_addr));

    // sending packet
    //sendto(sockfd,&pkt,sizeof(pkt),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

    // send 100 packets
    int count = 100;

    for(int i = 0;i<count;i++){
        // Packet pkt;
        pkt.sequence = i + 1;
        // pkt.timestamp = time(NULL); // this returns current time in seconds

        // time in microseconds
        struct timeval tv;
        gettimeofday(&tv,NULL);
        pkt.timestamp = tv.tv_sec * 1000000 + tv.tv_usec;

        snprintf(pkt.message,MAX_MSG_SIZE, "%s %d",message,i+1);

        sendto(sockfd,&pkt,sizeof(pkt),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

        //wait for ack
        AckPacket ack;
        socklen_t len = sizeof(server_addr);

        int n = recvfrom(sockfd, &ack, sizeof(ack),0,(struct sockaddr*)&server_addr,&len);

        if(n < 0){
            printf("Ack not received for seq %d\n", pkt.sequence);
        }else{
            printf("Ack receiver for seq %d\n", ack.ack_sequence);
        }

        printf("Sent Packet %d\n",pkt.sequence);

        // sleep(1);
    }

    printf("Message send\n");

    close(sockfd);
    return 0;

}