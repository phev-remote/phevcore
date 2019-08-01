#include <stdlib.h>
#include <stdio.h>
#include "phev.h"
#include "phev_pipe.h"
#include "phev_tcpip.h"
#include "phev_service.h"
#include "phev_register.h"

#include "msg_tcpip.h"
#include "logger.h"

const static char *TAG = "PHEV";

typedef struct phevCtx_t {
    phevServiceCtx_t * serviceCtx;
    phevEventHandler_t eventHandler;
    void * ctx;
} phevCtx_t;


int phev_pipeEventHandler(phev_pipe_ctx_t *ctx, phevPipeEvent_t *event)
{
    LOG_V(TAG,"START - pipeEventHandler");
    
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
        }
        case PHEV_PIPE_GOT_VIN:
        {
            phevVinEvent_t * vinEv = (phevVinEvent_t *) event; 
            char * vin = malloc(19);
            strncpy(vin,vinEv->vin,18);
            phevEvent_t ev = {
                .type = PHEV_VIN,
                .data = vin,
                .length = strlen(vin),

            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_ECU_VERSION2:
        {
            phevEvent_t ev = {
                .type = PHEV_ECU_VERSION,
            };
            return phevCtx->eventHandler(&ev);
        }
    }
    
    
    LOG_V(TAG,"END - pipeEventHandler");
    
    return 0;
}
void phev_outgoingHandler(messagingClient_t *client, message_t *message)
{
    //LOG_I(APP_TAG, "Outgoing Handler data");
    //printf("%s\n",message->data);
}
message_t * phev_incomingHandler(messagingClient_t *client)
{
    return NULL;
}
messagingClient_t * phev_createIncomingMessageClient(void)
{
    LOG_V(TAG,"START - createIncomingMessageClient");
    
    messagingSettings_t inSettings = {
        .incomingHandler = phev_incomingHandler,
        .outgoingHandler = phev_outgoingHandler,
    };

    messagingClient_t *in = msg_core_createMessagingClient(inSettings);
    LOG_V(TAG,"END - createIncomingMessageClient");
    
    return in;
}

messagingClient_t * phev_createOutgoingMessageClient(const char * host, const uint16_t port)
{
    LOG_V(TAG,"START - createOutgoingMessageClient");
    
    tcpIpSettings_t outSettings = {
        .connect = phev_tcpClientConnectSocket,
        .read = phev_tcpClientRead,
        .write = phev_tcpClientWrite,
        .host = strdup(host),
        .port = port,
    };
    messagingClient_t *out = msg_tcpip_createTcpIpClient(outSettings);

    LOG_V(TAG,"END - createOutgoingMessageClient");
    
    return out;
}

phevCtx_t * phev_init(phevSettings_t settings)
{
    LOG_V(TAG,"START - init");
    
    phevCtx_t * ctx = malloc(sizeof(phevCtx_t)); 

    messagingClient_t * in = NULL;
    messagingClient_t * out = NULL;

    if(settings.in) 
    {
        in = settings.in;
    } else {
        in = phev_createIncomingMessageClient();
    }
    
    if(settings.out) 
    {
        out = settings.out;
    } else {
        out = phev_createOutgoingMessageClient(settings.host,settings.port);
    }
    
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
    LOG_D(TAG,"Settings event handler %p", phev_pipeEventHandler);
    ctx->eventHandler = settings.handler;
    ctx->ctx = settings.ctx;

    phevServiceCtx_t * srvCtx = phev_service_create(serviceSettings);

    ctx->serviceCtx = srvCtx;

    LOG_V(TAG,"END - init");
    
    return ctx;
}

void phev_registrationComplete(phevRegisterCtx_t * ctx)
{
    phevCtx_t * phevCtx = (phevCtx_t *) ((phevServiceCtx_t *) ctx->ctx)->ctx;

    phevEvent_t ev = {
        .type = PHEV_REGISTRATION_COMPLETE,
    };
    phevCtx->eventHandler(&ev);

}
phevCtx_t * phev_registerDevice(phevSettings_t settings)
{
    LOG_V(TAG,"START - registerDevice");
    
    phevCtx_t * ctx = malloc(sizeof(phevCtx_t)); 

    messagingClient_t * in = phev_createIncomingMessageClient();
    messagingClient_t * out = phev_createOutgoingMessageClient(settings.host,settings.port);

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

    phev_service_register(serviceSettings.mac, srvCtx, phev_registrationComplete);

    LOG_V(TAG,"END - registerDevice");
    
    return ctx;
} 
void phev_start(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - start");
    phev_service_start(ctx->serviceCtx);
    LOG_V(TAG,"END - start");
}
void phev_exit(phevCtx_t * ctx)
{
    ctx->serviceCtx->exit = true;
}