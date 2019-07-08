#include <stdlib.h>
#include <stdio.h>
#include "phev.h"
#include "phev_pipe.h"
#include "phev_tcpip.h"
#include "phev_service.h"
#include "msg_tcpip.h"

typedef struct phevCtx_t {
    phevServiceCtx_t * serviceCtx;
    phevEventHandler_t eventHandler;
    void * ctx;
} phevCtx_t;


int phev_pipeEventHandler(phev_pipe_ctx_t *ctx, phevPipeEvent_t *event)
{
    phevCtx_t * phevCtx = (phevCtx_t *) ((phevServiceCtx_t *) ctx->ctx)->ctx;
    
    
    if(!phevCtx->eventHandler)
    {
        return 0;
    }

    switch(event->event) {
        case PHEV_PIPE_CONNECTED: {
            phevEvent_t ev = {
                .type = PHEV_CONNECTED,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_REG_UPDATE: {
            phevEvent_t ev = {
                .type = PHEV_REGISTER_UPDATE,
                .reg = ((phevMessage_t *) event->data)->reg,
                .data = ((phevMessage_t *) event->data)->data,
                .length = ((phevMessage_t *) event->data)->length,
            };
            return phevCtx->eventHandler(&ev);
        };
        
    }
    
    
    
    return 0;
}
void outgoingHandler(messagingClient_t *client, message_t *message)
{
    //LOG_I(APP_TAG, "Outgoing Handler data");
    //printf("%s\n",message->data);
}
message_t *incomingHandler(messagingClient_t *client)
{
    return NULL;
}
messagingClient_t * createIncomingMessageClient(void)
{
    messagingSettings_t inSettings = {
        .incomingHandler = incomingHandler,
        .outgoingHandler = outgoingHandler,
    };

    messagingClient_t *in = msg_core_createMessagingClient(inSettings);

    return in;
}

messagingClient_t * createOutgoingMessageClient(const char * host, const uint16_t port)
{
    tcpIpSettings_t outSettings = {
        .connect = phev_tcpClientConnectSocket,
        .read = phev_tcpClientRead,
        .write = phev_tcpClientWrite,
        .host = strdup(host),
        .port = port,
    };
    messagingClient_t *out = msg_tcpip_createTcpIpClient(outSettings);

    return out;
}
phevCtx_t * phev_init(phevSettings_t settings)
{
    phevCtx_t * ctx = malloc(sizeof(phevCtx_t)); 

    messagingClient_t * in = createIncomingMessageClient();
    messagingClient_t * out = createOutgoingMessageClient(settings.host,settings.port);

    phevServiceSettings_t serviceSettings = {
        .in = in,
        .out = out,
        .mac = settings.mac, 
        .registerDevice = false,
        .eventHandler = phev_pipeEventHandler,
        .errorHandler = NULL,
        .yieldHandler = NULL,
        .ctx = ctx,
     
    };

    ctx->eventHandler = settings.handler;
    ctx->ctx = settings.ctx;

    phevServiceCtx_t * srvCtx = phev_service_create(serviceSettings);

    ctx->serviceCtx = srvCtx;

    return ctx;
} /*
phevCtx_t * phev_registerDevice(phevSettings_t settings)
{
    phevCtx_t * ctx = malloc(sizeof(phevCtx_t)); 

    messagingClient_t * in = createIncomingMessageClient();
    messagingClient_t * out = createOutgoingMessageClient(settings.host,settings.port);

    phevServiceSettings_t serviceSettings = {
        .in = in,
        .out = out,
        .mac = settings.mac, 
        .registerDevice = true,
        .eventHandler = phev_pipeEventHandler,
        .errorHandler = NULL,
        .yieldHandler = NULL,
     
    };
 
    phevServiceCtx_t * srvCtx = phev_service_create(serviceSettings);

    ctx->serviceCtx = srvCtx;

    return ctx;
} */
void phev_start(phevCtx_t * ctx)
{
    phev_service_start(ctx->serviceCtx);
}