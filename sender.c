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
    if(inet_pton(AF_INET,ip,&server_addr.sin_addr) <= 0){
        perror("Invalid Address");
        return 1;
    }

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

    // ======= loop without retransimission =======

    // send 100 packets
    // int count = 100;

    // for(int i = 0;i<count;i++){
    //     // Packet pkt;
    //     pkt.sequence = i + 1;
    //     // pkt.timestamp = time(NULL); // this returns current time in seconds

    //     // time in microseconds
    //     struct timeval tv;
    //     gettimeofday(&tv,NULL);
    //     pkt.timestamp = tv.tv_sec * 1000000 + tv.tv_usec;

    //     snprintf(pkt.message,MAX_MSG_SIZE, "%s %d",message,i+1);

    //     sendto(sockfd,&pkt,sizeof(pkt),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

    //     //wait for ack
    //     AckPacket ack;
    //     socklen_t len = sizeof(server_addr);

    //     int n = recvfrom(sockfd, &ack, sizeof(ack),0,(struct sockaddr*)&server_addr,&len);

    //     if(n < 0){
    //         printf("Ack not received for seq %d\n", pkt.sequence);
    //     }else{
    //         printf("Ack received for seq %d\n", ack.ack_sequence);
    //     }

    //     printf("Sent Packet %d\n",pkt.sequence);

    //     // sleep(1);
    // }

    // ===== loop with retransmission =====

    // int count = 50;

    // for(int i = 0 ; i<count; i++){
    //     Packet pkt;
    //     pkt.sequence = i + 1;

    //     struct timeval tv;
    //     gettimeofday(&tv,NULL);
    //     pkt.timestamp = tv.tv_sec * 1000000 + tv.tv_usec;

    //     snprintf(pkt.message,MAX_MSG_SIZE, "%s %d", message,i+1);

    //     int ack_received = 0;

    //     while(!ack_received){
    //         // send packet
    //         sendto(sockfd,&pkt,sizeof(pkt),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

    //         printf("Sent Packet %d\n",pkt.sequence);

    //         // 2. wait for ack
    //         AckPacket ack;
    //         socklen_t len = sizeof(server_addr);

    //         int n = recvfrom(sockfd,&ack,sizeof(ack),0,(struct sockaddr*)&server_addr,&len);

    //         if(n < 0){
    //             printf("Timeout -> Resending Packet %d\n",pkt.sequence);
    //         }else if(ack.ack_sequence == pkt.sequence){
    //             printf("Ack receiver for %d\n",pkt.sequence);
    //             ack_received = 1;
    //         }
    //     }
    // }

    // ===== sliding window for transmission

    // int window_size = 5; // fixed
    
    // additive increase , multiplicative decrease

    int window_size = 1;
    int max_window = 50;

    int base = 1;
    int next_seq = 1;

    int total_packets = 70;

    Packet window[1000];

    while(base <= total_packets){
        // send packets within the window
        while(next_seq < base + window_size && next_seq <= total_packets){
            Packet pkt;
            pkt.sequence = next_seq;

            struct timeval tv;
            gettimeofday(&tv,NULL);
            pkt.timestamp = tv.tv_sec * 1000000 + tv.tv_usec;

            snprintf(pkt.message,MAX_MSG_SIZE, "%s %d",message, next_seq);

            window[next_seq - 1] = pkt;

            sendto(sockfd,&pkt,sizeof(pkt),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

            printf("📤 Sent %d\n",next_seq);

            next_seq++;
        }

        // receive ack
        AckPacket ack;
        socklen_t len = sizeof(server_addr);

        int n = recvfrom(sockfd,&ack,sizeof(ack),0,(struct sockaddr*)&server_addr,&len);

        if(n>=0){
            printf("✅ Ack %d\n", ack.ack_sequence);

            if(ack.ack_sequence >= base){
                base = ack.ack_sequence + 1;

                // increase window
                if(window_size < max_window){
                    window_size++;
                }

                printf("Window increased to %d\n", window_size);
            }
        }else{
            // timeout -> resend all un-acked packets
            printf("Timeout Resending window\n");

            // decrease window
            window_size = window_size / 2;
            if(window_size < 1){
                window_size = 1;
            }

            printf("Window reduced to %d\n", window_size);

            for(int i = base;i<next_seq;i++){
                sendto(sockfd,&window[i-1],sizeof(Packet),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

                printf("🔁 Resent %d\n",i);
            }
        }
    }

    printf("Message send\n");

    close(sockfd);
    return 0;

}