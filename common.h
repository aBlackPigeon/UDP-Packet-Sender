#ifndef COMMON_H
#define COMMON_H

#define MAX_MSG_SIZE 256

typedef struct{
    int sequence;
    long timestamp;
    char message[MAX_MSG_SIZE];
} Packet;

typedef struct{
    int ack_sequence;
} AckPacket;

#endif