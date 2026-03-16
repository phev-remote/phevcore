#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __LINUX__
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "msg_tcpip.h"
#include "msg_utils.h"
#include "logger.h"

const static char *APP_TAG = "MSG_TCPIP";

int msg_tcpip_start(messagingClient_t *client)
{
    return 0;
}
int msg_tcpip_stop(messagingClient_t *client)
{
    return 0;
}
int msg_tcpip_disconnect(messagingClient_t *client)
{
    LOG_V(APP_TAG,"START - disconnect");
    tcpip_ctx_t * ctx = (tcpip_ctx_t *) client->ctx;
    
    ctx->disconnect(ctx->socket);

    client->connected = 0;
    ctx->socket = -1;
    LOG_V(APP_TAG,"END - disconnect");
    return 0;
}
int msg_tcpip_connect(messagingClient_t *client)
{
    LOG_V(APP_TAG,"START - connect");
    tcpip_ctx_t * ctx = (tcpip_ctx_t *) client->ctx;
    LOG_D(APP_TAG,"Context Host %s Port %d",ctx->host,ctx->port);
    int s = 0;
    if((s = ctx->connect(ctx->host,ctx->port)) > -1) 
    {
        LOG_D(APP_TAG,"Connected to server");
    
        ctx->socket = s;
        client->connected = 1;
        LOG_V(APP_TAG,"END - connect");
    
        return 0;
    } else {
        LOG_E(APP_TAG,"Failed to connected to server : %d",s);
    
        ctx->socket = -1;
        client->connected = 0;
        LOG_V(APP_TAG,"END - connect");
    
        return -1;
    }
}
message_t *msg_tcpip_incomingHandler(messagingClient_t *client)
{
    //LOG_V(APP_TAG,"START - incomingHandler");


    if (!client->connected)
    {
        int ret = msg_tcpip_connect(client);
        if(ret < 0) 
        {
            LOG_E(APP_TAG,"Cannot connect");
            return NULL;
        }
    }
    tcpip_ctx_t *ctx = (tcpip_ctx_t *)client->ctx;
    int len = ctx->read(ctx->socket, ctx->readBuffer, TCPIP_CLIENT_READ_BUF_SIZE);

    if (len > 0 && len < TCPIP_CLIENT_READ_BUF_SIZE)
    {
        LOG_D(APP_TAG,"Got message size %d",len);
    
        return msg_utils_createMsg(ctx->readBuffer,len);
    } 
    if(len < 0) {
        LOG_E(APP_TAG,"Read returned error : %d",len);
    
        client->connected = 0;
    }
    

    //LOG_V(APP_TAG,"END - incomingHandler");
    return NULL;
}
void msg_tcpip_outgoingHandler(messagingClient_t *client, message_t *message)
{
    LOG_V(APP_TAG,"START - outgoingHandler");
    
    tcpip_ctx_t *ctx = (tcpip_ctx_t *)client->ctx;
    if(client->connected)
    {
        if (message->data && message->length)
        {
            int num = ctx->write(ctx->socket, message->data, message->length);
            if(num != message->length)
            {
                LOG_E(APP_TAG,"Write returned error : %d",num);
                client->connected = 0;
            } else {
               // msg_utils_destroyMsg(message);
            }
        }
    }
    LOG_V(APP_TAG,"END - outgoingHandler");
    
}
messagingClient_t *msg_tcpip_createTcpIpClient(tcpIpSettings_t settings)
{
    messagingSettings_t clientSettings;

    tcpip_ctx_t *ctx = malloc(sizeof(tcpip_ctx_t));

    ctx->read = settings.read;
    ctx->write = settings.write;
    ctx->connect = settings.connect;
    ctx->disconnect = settings.disconnect;

    ctx->host = settings.host;
    ctx->port = settings.port;

    ctx->readBuffer = malloc(TCPIP_CLIENT_READ_BUF_SIZE);

    clientSettings.incomingHandler = msg_tcpip_incomingHandler;
    clientSettings.outgoingHandler = msg_tcpip_outgoingHandler;

    clientSettings.start = msg_tcpip_start;
    clientSettings.stop = msg_tcpip_stop;
    clientSettings.connect = msg_tcpip_connect;
    clientSettings.disconnect = msg_tcpip_disconnect;

    clientSettings.ctx = (void *)ctx;

    return msg_core_createMessagingClient(clientSettings);
}