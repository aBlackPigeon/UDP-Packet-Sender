#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"
#include <sys/time.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd;
    // char buffer[BUFFER_SIZE];
    Packet pkt;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // 1. create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket Failed");
        return 1;
    }

    // setting up log file
    FILE *log_file = fopen("logs.txt","w");
    if(log_file == NULL){
        perror("Failed to open log file");
        return 1;
    }

    setvbuf(log_file,NULL,_IOFBF,1024*1024);
    fprintf(log_file,"Timestamp,sequence,Latency(us),SourceIp,SourcePort\n");

    // 2. setup the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // 3. bind

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind Failed");
        return 1;
    }

    printf("Listening on port %d ... \n", port);

    int expected_seq = 1;
    srand(time(NULL) + rand());

    // throughput system
    long start_time = 0;
    long end_time = 0;

    int packet_count = 0;
    long total_bytes = 0;

    while (1)
    {
        // receiver message
        // int n = recvfrom(sockfd,buffer, BUFFER_SIZE,0 , (struct sockaddr*)&client_addr, &addr_len);
        int n = recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&client_addr, &addr_len);

        if (n < 0)
        {
            printf("recvfrom failed");
            continue;
        }

        int drop_simulate = 0; // 0-> false 1-> true

        // packet drop
        if (drop_simulate){
            int drop = rand() % 100;

            if (drop < 90){
                //printf("Simulating packet drop for seq %d\n", pkt.sequence);
                continue;
            }
        }

        // latency measurement
        // long receive_time = time(NULL);

        struct timeval tv;
        gettimeofday(&tv,NULL);

        long receive_time = tv.tv_sec * 1000000 + tv.tv_usec;
        long latency = receive_time - pkt.timestamp;
        //printf("Latency %ld microseconds\n" , latency);

        fprintf(log_file,"%ld,%d,%ld,%s,%d\n",
        pkt.timestamp,pkt.sequence,latency,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        if(packet_count % 1000 == 0){
            fflush(log_file);
        }

        // throughput system
        if(packet_count == 0){
            start_time = receive_time;
        }

        packet_count++;
        total_bytes += sizeof(pkt);

        end_time = receive_time;

        if (pkt.sequence != expected_seq)
        {
            //printf("Packet Loss detected! Expected %d but got %d\n", expected_seq, pkt.sequence);
            expected_seq = pkt.sequence + 1;
        }
        else
        {
            expected_seq++;
        }

        // throughput system prints
        long duration = end_time - start_time; // microseconds

        double seconds = duration / 1000000.0;

        double pps = packet_count / seconds;
        double bps = total_bytes / seconds;

        // Receiver send ACK
        AckPacket ack;
        ack.ack_sequence = pkt.sequence;

        sendto(sockfd,&ack,sizeof(ack),0,(struct sockaddr*)&client_addr,addr_len);

        // printf("\n Throughput stats\n");
        // printf("Packets/sec : %.2f\n", pps);
        // printf("Bytes/sec: %.2f\n",bps);

        printf("\nReceived Packet\n");
        printf("Sequence %d\n", pkt.sequence);
        // printf("Timestamp %ld\n", pkt.timestamp);
        printf("Message %s\n", pkt.message);

        // printf("From ip: %s\n", inet_ntoa(client_addr.sin_addr));
        // printf("From port : %d\n", ntohs(client_addr.sin_port));
    }


    close(sockfd);
    return 0;
}