#ifndef _MSG_TCPIP_H_
#define _MSG_TCPIP_H_

#include "msg_core.h"

#define TCPIP_CLIENT_READ_BUF_SIZE 2048
//#define OK 0

typedef struct tcpIpSettings_t {
    char * host;
    uint16_t port;
    int (* connect)(const char*, uint16_t);
    int (* disconnect)(int);
    int (* read)(int,uint8_t*,size_t);
    int (* write)(int,uint8_t*,size_t);
} tcpIpSettings_t;
typedef struct tcpip_ctx_t {
    char *host;
    uint16_t port;
    int socket;
    uint8_t * readBuffer;
    int (* connect)(const char*, uint16_t);
    int (* disconnect)(int);
    int (* read)(int,uint8_t*,size_t);
    int (* write)(int,uint8_t*,size_t);
} tcpip_ctx_t;

messagingClient_t * msg_tcpip_createTcpIpClient(tcpIpSettings_t);

int msg_tcpip_start(messagingClient_t *client);
int msg_tcpip_stop(messagingClient_t *client);
int msg_tcpip_connect(messagingClient_t *client);
int msg_tcpip_disconnect(messagingClient_t *client);

#endif