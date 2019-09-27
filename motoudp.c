// Copyright (c) 2019 Sagar Gubbi. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "motoPlus.h"
#include "motoudp.h"

#define UDP_BUFFER_LEN 240

static char udpBuffer[UDP_BUFFER_LEN];
static struct sockaddr_in clientAddr;
static int clientAddrLen;
static int sockfd;

int mpUDPInit(unsigned int port, void (*udpCallback)(char* buf, int buf_len))
{
    int n;
    int rc;
    struct sockaddr_in serverAddr;

    sockfd = mpSocket(AF_INET, SOCK_DGRAM, 0); //socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddrLen = 0;

    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = mpHtons(port);//htons(port);

    rc = mpBind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); //bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(rc != 0) {
        printf("Error binding socket\n");
        return -2;
    }

    if(udpCallback == NULL) {
        return -3;
    }

    while(1) {
        clientAddrLen = sizeof(clientAddr);
        n = mpRecvFrom(sockfd, udpBuffer, UDP_BUFFER_LEN, 0, (struct sockaddr *)&clientAddr, &clientAddrLen); //recvfrom(sockfd, (char *)udpBuffer, UDP_BUFFER_LEN, MSG_WAITALL, (struct sockaddr *)&clientAddr, &clientAddrLen);
        udpCallback(udpBuffer, n);
    }

    return 0;
}

int mpUDPSend(char *buf, int buf_len)
{
    int rc;
    if(sockfd < 0) {
        return -1;
    }
    if(clientAddrLen == 0) {
        return -2;
    }
    rc = mpSendTo(sockfd, buf, buf_len, 0, (struct sockaddr *)&clientAddr, clientAddrLen); // sendto(sockfd, (const char *)buf, buf_len, MSG_CONFIRM, (const struct sockaddr *)&clientAddr, clientAddrLen);
    if(rc < 0) {
        // printf("sendto() returned %d, errno: %s\n", rc, strerror(errno));
        return -3;
    }
    return 0;
}

int mpUDPPrintf(const char* fmt, ...)
{
    va_list argp;
    int n = 0;
    char out_buf[240];
    va_start(argp, fmt);
    n = vsnprintf(out_buf, sizeof(out_buf)-1, fmt, argp);
    va_end(argp);
    return mpUDPSend(out_buf, n);
}

