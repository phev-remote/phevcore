#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include "phev_core.h"
#include "phev_pipe.h"
#include "phev_register.h"
#include "phev_service.h"

const char * TAG = "PHEV_REGISTER";

void phev_register_sendMac(phev_pipe_ctx_t * ctx)
{
    LOG_V(TAG, "START - sendMac");
    //phev_pipe_sendMac(ctx, ((phevServiceCtx_t *) ctx->ctx)->mac);
    LOG_V(TAG,"END - sendMac");
}
phevRegisterCtx_t * phev_register_init(phevRegisterSettings_t settings)
{
    LOG_V(TAG,"START - init");
    phevRegisterCtx_t * ctx = malloc(sizeof(phevRegisterCtx_t));
    memset(ctx,0,sizeof(phevRegisterCtx_t));

    ctx->pipe = settings.pipe;
    ctx->complete = settings.complete;

    memcpy(ctx->mac,settings.mac,MAC_ADDR_SIZE);

    ctx->vin = NULL;
    ctx->pipe->ctx = settings.ctx;
    ctx->startAck = false;
    ctx->aaAck = false;
    ctx->registrationRequest = false;
    ctx->ecu = false;
    ctx->remoteSecurity = false;
    ctx->registrationAck = false;
    ctx->registrationComplete = false;
    ctx->errorHandler = settings.errorHandler;    
    ctx->pipe->errorHandler = settings.errorHandler;
    ctx->ctx = settings.ctx;
    ctx->pipe->registrationCompleteCallback = settings.complete;

    if(settings.eventHandler)
    {
        phev_pipe_registerEventHandler(ctx->pipe, settings.eventHandler);    
    }
    else
    {
        //phev_pipe_registerEventHandler(ctx->pipe, phev_register_eventHandler);
    }
    LOG_V(TAG,"END - init");
    return ctx;
}
void phev_register_sendRegister(phev_pipe_ctx_t * ctx)
{
    LOG_V(TAG,"START - sendRegister");
    phevMessage_t * reg = phev_core_simpleRequestCommandMessage(KO_WF_REG_DISP_SP,1);
    message_t * message = phev_core_convertToMessage(reg);

    phev_pipe_commandOutboundPublish(ctx,  message);
    LOG_V(TAG,"END - sendRegister");
}
int phev_register_eventHandler(phev_pipe_ctx_t * ctx, phevPipeEvent_t * event)
{
    LOG_V(TAG,"START - eventHandler");
    phevRegisterCtx_t * regCtx = ((phevServiceCtx_t *) ctx->ctx)->registrationCtx;

    if(regCtx->registrationComplete) 
    {
        return 0;
    }
    switch(event->event)
    {
        case PHEV_PIPE_GOT_VIN: {
            char * vin = ((phevVinEvent_t *) event->data)->vin;
            phev_register_sendRegister(ctx);
            LOG_I(TAG,"Got VIN %s",vin);
            if(vin) regCtx->vin = strdup(vin);
            //phev_register_sendMac(ctx);
            break;
        }
        case PHEV_PIPE_START_ACK: {
            LOG_I(TAG,"Start acknowledged");
            regCtx->startAck = true;
            phev_register_sendRegister(ctx);
            break;
        }
        case PHEV_PIPE_CONNECTED: {
            LOG_I(TAG,"AA acknowledged");
            regCtx->aaAck = true;
            break;
        }
        case PHEV_PIPE_REGISTRATION: {
            LOG_I(TAG,"Registration");
            regCtx->registrationRequest = true;
            phev_register_sendRegister(ctx);
            break;
        }
        case PHEV_PIPE_ECU_VERSION2: {
            LOG_I(TAG,"ECU version");
            regCtx->ecu = true;
            phev_register_sendRegister(ctx);
            break;
        };
        case PHEV_PIPE_REMOTE_SECURTY_PRSNT_INFO: {
            LOG_I(TAG,"Remote security present info");
            regCtx->remoteSecurity = true;
            phev_register_sendRegister(ctx);
            break;
        }
        case PHEV_PIPE_REG_DISP: {
            LOG_I(TAG,"Registration Acknowledged");

            regCtx->registrationAck = true;   
            regCtx->complete(ctx);
            LOG_I(TAG,"REGISTERED");
            while(true);
            break;
        }
        case PHEV_PIPE_MAX_REGISTRATIONS: {
            LOG_E(TAG,"Max number of allowed registrations");
            phevError_t error = {
                .message = "Maximum number of registrations"
            };
            regCtx->errorHandler(&error);
            return 1;
        }
        default : {
            LOG_W(TAG, "Unknown event %d\n",event->event);
        }
    }
    if(regCtx->registrationAck &&
        regCtx->vin != NULL)
    {
        LOG_I(TAG,"Registration Complete for VIN %s",regCtx->vin);
        regCtx->registrationComplete = true;
        if(regCtx->complete != NULL)
        {
                LOG_D(TAG,"Calling callback");

                regCtx->complete(ctx);
        }
    }
    LOG_V(TAG,"END - eventHandler");
    return 0;
}
