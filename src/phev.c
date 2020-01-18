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

void * phev_getUserCtx(phevCtx_t * ctx)
{
    return ctx->ctx;
}

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
                .ctx =  phevCtx,
            };
            
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_START_ACK: {
            phevEvent_t ev = {
                .type = PHEV_STARTED,
                .ctx =phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_REG_UPDATE: {
            phevEvent_t ev = {
                .type = PHEV_REGISTER_UPDATE,
                .reg = ((phevMessage_t *) event->data)->reg,
                .data = ((phevMessage_t *) event->data)->data,
                .length = ((phevMessage_t *) event->data)->length,
                .ctx =  phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_GOT_VIN:
        {
            phevVinEvent_t * vinEv = (phevVinEvent_t *) event->data; 
            char * vin = malloc(19);
          
            strncpy(vin,vinEv->vin,18);
            
            phevEvent_t ev = {
                .type = PHEV_VIN,
                .data = (unsigned char *) vin,
                .length = strlen(vin),
                .ctx =  phevCtx,

            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_ECU_VERSION2:
        {
            char * version = malloc(11);

            strncpy(version,event->data,10);
            phevEvent_t ev = {
                .type = PHEV_ECU_VERSION,
                .data = (uint8_t *) version,
                .length = strlen(version),
                .ctx =  phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_DATE_INFO:
        {
            phevEvent_t ev = {
                .type = PHEV_DATE_SYNC,
                .data = event->data,
                .length = event->length,
                .ctx = phevCtx,
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

    phevServiceSettings_t * serviceSettings;
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
    
    if(settings.registerDevice) 
    {
        phevServiceSettings_t s = {
            .in = in,
            .out = out,
            .mac = settings.mac, 
            .registerDevice = true,
            .eventHandler = phev_pipeEventHandler,
            .errorHandler = NULL,
            .yieldHandler = NULL,
            .my18 = settings.my18,
            .ctx = ctx, 
        };

        serviceSettings = &s;
    } else {
        phevServiceSettings_t s = {
            .in = in,
            .out = out,
            .mac = settings.mac, 
            .registerDevice = false,
            .eventHandler = phev_pipeEventHandler,
            .errorHandler = NULL,
            .yieldHandler = NULL,
            .my18 = settings.my18,
            .ctx = ctx,
        };
        serviceSettings = &s;
    }
    LOG_D(TAG,"Settings event handler %p", phev_pipeEventHandler);
    ctx->eventHandler = settings.handler;
    ctx->ctx = settings.ctx;

    phevServiceCtx_t * srvCtx = phev_service_create(*serviceSettings);

    ctx->serviceCtx = srvCtx;
    //ctx->ctx = srvCtx;

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
    
    phevCtx_t * ctx = phev_init(settings);
    
    phev_service_register((const char *) settings.mac, ctx->serviceCtx, phev_registrationComplete);

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
    LOG_V(TAG,"START - exit");
    
    ctx->serviceCtx->exit = true;

    LOG_V(TAG,"START - exit");
    
}
bool phev_running(phevCtx_t * ctx)
{
    return !ctx->serviceCtx->exit;
}
static void phev_headLightsCallback(phev_pipe_ctx_t *ctx, uint8_t reg, void * customCtx)
{
    phevCallBackCtx_t * cbCtx = (phevCallBackCtx_t *) customCtx;

    cbCtx->callback(cbCtx->ctx,NULL);

}
void phev_headLights(phevCtx_t * ctx, bool on, phevCallBack_t callback)
{
    LOG_V(TAG,"START - headLights");
    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    if(on)
    {
        LOG_D(TAG,"Switching on head lights");
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_H_LAMP_CONT_SP, 1,phev_headLightsCallback,cbCtx);
    } else {
        LOG_D(TAG,"Switching off head lights"); 
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_H_LAMP_CONT_SP, 2,phev_headLightsCallback,cbCtx);
    }
    LOG_V(TAG,"END - headLights");
    
}
void phev_airCon(phevCtx_t * ctx, bool on, phevCallBack_t callback)
{
    LOG_V(TAG,"START - airCon");
        phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    if(on)
    {
        LOG_D(TAG,"Switching on air conditioning");
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_MANUAL_AC_ON_RQ_SP, 2,phev_headLightsCallback,cbCtx);
    } else {
        LOG_D(TAG,"Switching off air conditioning"); 
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_MANUAL_AC_ON_RQ_SP, 1,phev_headLightsCallback,cbCtx);
    }
    LOG_V(TAG,"END - airCon");
    
}
int phev_batteryLevel(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - batteryLevel");

    int level = phev_service_getBatteryLevel(ctx->serviceCtx);

    LOG_V(TAG,"END - batteryLevel");
    return level;
}
phevData_t * phev_getRegister(phevCtx_t * ctx, uint8_t reg)
{
    return (phevData_t *) phev_service_getRegister(ctx->serviceCtx, reg); 
}

char * phev_statusAsJson(phevCtx_t * ctx)
{
    return phev_service_statusAsJson(ctx->serviceCtx);
}