#include "phev_pipe.h"
#include "phev_core.h"
#include "msg_utils.h"
#include "logger.h"

//#define NO_PING
//#define NO_CMD_RESP
//#define NO_TIME_SYNC

const static char *APP_TAG = "PHEV_PIPE";


void phev_pipe_resetPing(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG, "START - resetPing");
    time_t now;

    time(&now);
    ctx->currentPing = 1;
    ctx->lastPingTime = now;
    LOG_V(APP_TAG, "END - resetPing");
}

void phev_pipe_destroyEvent(phevPipeEvent_t * event)
{
    if(event != NULL)
    {
        if(event->data != NULL)
        {
            free(event->data);
        }
        free(event);
        event = NULL;
    }

}
void phev_pipe_disconnectInput(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG,"START - disconnectInput");
    
    msg_pipe_in_disconnect(ctx->pipe);
    
    LOG_V(APP_TAG,"END - disconnectInput");
}
void phev_pipe_disconnectOutput(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG,"START - disconnectOutput");

    msg_pipe_out_disconnect(ctx->pipe);

    ctx->connected = false;

    phev_pipe_resetPing(ctx);

    ctx->currentXOR = 0;
    ctx->pingXOR = 0;
    ctx->commandXOR = 0;
    ctx->encrypt = false;
    ctx->pingResponse = 0;
    
    LOG_V(APP_TAG,"END - disconnectOutput");
}
void phev_pipe_waitForConnection(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG, "START - waitForConnection");
    ctx->connected = false;
    int retries = 0;

    if (!ctx->pipe->in->connected)
    {
        LOG_V(APP_TAG, "Calling in connect");
        msg_pipe_in_connect(ctx->pipe);
    }
    if (!ctx->pipe->out->connected)
    {
        LOG_V(APP_TAG, "Calling out connect");
        msg_pipe_out_connect(ctx->pipe);
    }
    while (!(ctx->pipe->in->connected && ctx->pipe->out->connected))
    {
        LOG_I(APP_TAG, "Not connected waiting...");
        SLEEP(PHEV_CONNECT_WAIT_TIME);
        if (retries > PHEV_CONNECT_MAX_RETRIES)
        {
            LOG_E(APP_TAG, "Max retries reached");
            return;
        }

        if (!ctx->pipe->in->connected)
        {
            LOG_V(APP_TAG, "Calling in connect");
            msg_pipe_in_connect(ctx->pipe);
        }
        if (!ctx->pipe->out->connected)
        {
            LOG_V(APP_TAG, "Calling out connect");
            msg_pipe_out_connect(ctx->pipe);
        }
        retries++;    
    }

    ctx->connected = true;
    LOG_V(APP_TAG, "END - waitForConnection");
}
void phev_pipe_loop(phev_pipe_ctx_t *ctx)
{
    time_t now;

    if (ctx->pipe->in->connected && ctx->pipe->out->connected)
    {
        ctx->connected = true;
        msg_pipe_loop(ctx->pipe);
    }
    else
    {
        phev_pipe_waitForConnection(ctx);
    }

    if (ctx->pipe->out->connected)
    {
        LOG_V(APP_TAG, "Sending ping");
        time(&now);
        if (now > ctx->lastPingTime)
        {
            phev_pipe_ping(ctx);
            time(&ctx->lastPingTime);
        }
    }
}
void phev_pipe_sendMac(phev_pipe_ctx_t *ctx, uint8_t *mac)
{
    LOG_V(APP_TAG, "START - sendMac");

    message_t *message = phev_core_startMessageEncoded(mac);
    phev_pipe_outboundPublish(ctx, message);

    LOG_V(APP_TAG, "END - sendMac");
}
void phev_pipe_start(phev_pipe_ctx_t *ctx, uint8_t *mac)
{
    LOG_V(APP_TAG, "START - start");

    phev_pipe_waitForConnection(ctx);

    phev_pipe_sendMac(ctx, mac);
    LOG_V(APP_TAG, "END - start");
}
phev_pipe_ctx_t *phev_pipe_createPipe(phev_pipe_settings_t settings)
{
    LOG_V(APP_TAG, "START - createPipe");

    phev_pipe_ctx_t *ctx = malloc(sizeof(phev_pipe_ctx_t));

    msg_pipe_chain_t *inputChain = malloc(sizeof(msg_pipe_chain_t));
    msg_pipe_chain_t *outputChain = malloc(sizeof(msg_pipe_chain_t));

    inputChain->inputTransformer = settings.inputInputTransformer;
    inputChain->splitter = settings.inputSplitter;
    inputChain->aggregator = settings.inputAggregator;
    inputChain->filter = settings.inputFilter;
    inputChain->outputTransformer = settings.inputOutputTransformer;
    inputChain->responder = settings.inputResponder;
    inputChain->respondOnce = true;

    outputChain->inputTransformer = settings.outputInputTransformer;
    outputChain->splitter = settings.outputSplitter;
    outputChain->aggregator = settings.outputAggregator;
    outputChain->filter = settings.outputFilter;
    outputChain->outputTransformer = settings.outputOutputTransformer;
    outputChain->responder = settings.outputResponder;
    outputChain->respondOnce = false;

    msg_pipe_settings_t pipe_settings = {
        .in = settings.in,
        .out = settings.out,
        .lazyConnect = false,
        .user_context = ctx,
        .in_chain = inputChain,
        .out_chain = outputChain,
        .preOutConnectHook = settings.preConnectHook,
        .preInConnectHook = NULL,
    };

    ctx->pipe = msg_pipe(pipe_settings);

    ctx->errorHandler = settings.errorHandler;
    ctx->eventHandlers = 0;

    for (int i = 0; i < PHEV_PIPE_MAX_EVENT_HANDLERS; i++)
    {
        ctx->eventHandler[i] = NULL;
    }
    
    ctx->updateRegisterCallbacks = malloc(sizeof(phev_pipe_updateRegisterCtx_t));
    ctx->updateRegisterCallbacks->numberOfCallbacks = 0;

    for (int i = 0; i < PHEV_PIPE_MAX_UPDATE_CALLBACKS; i++)
    {
        ctx->updateRegisterCallbacks->callbacks[i] = NULL;
        ctx->updateRegisterCallbacks->used[i] = false;
    }
    ctx->connected = false;
    ctx->ctx = settings.ctx;
    ctx->currentXOR = 0;
    ctx->pingXOR = 0;
    ctx->commandXOR = 0;
    ctx->encrypt = false;
    ctx->pingResponse = 0;
    ctx->registerDevice = settings.registerDevice;

    phev_pipe_resetPing(ctx);

    LOG_V(APP_TAG, "END - createPipe");

    return ctx;
}
static bool waiting = false;
static bool bb_waiting = false;

void refreshCallback2(phev_pipe_ctx_t *ctx, uint8_t reg, void *customCtx)
{
    waiting = false;
    LOG_I(APP_TAG,"                        REFRESH ACK");
}
message_t *phev_pipe_outputChainInputTransformer(void *ctx, message_t *message)
{
    LOG_V(APP_TAG, "START - outputChainInputTransformer");
    LOG_D(APP_TAG,"Incoming message");
    LOG_BUFFER_HEXDUMP(APP_TAG, message->data, message->length, LOG_DEBUG);

    phevMessage_t *phevMessage = malloc(sizeof(phevMessage_t));
    phev_pipe_ctx_t *pipeCtx = (phev_pipe_ctx_t *)ctx;

    int ret = phev_core_decodeMessage(message->data, message->length, phevMessage);
    
    if (ret == 0)
    {
        LOG_E(APP_TAG, "Invalid message received");

        msg_utils_destroyMsg(message);
        return NULL;
    }
    if(message->ctx != NULL) 
    {
        uint8_t xor = phev_core_getMessageXOR(message);
        //LOG_I(APP_TAG,"Command received XOR changed to %02X",xor);
        pipeCtx->currentXOR = xor;
        pipeCtx->commandXOR = xor;
        pipeCtx->pingXOR = xor;

    }
    if(phevMessage->command == 0xbb) 
    {
        pipeCtx->commandXOR = phevMessage->data[0];
        pipeCtx->pingXOR = phevMessage->data[0];
        
        //LOG_I(APP_TAG,"%02X command recieved XOR changed to %02X",phevMessage->command, pipeCtx->commandXOR);
        
    }
    if(phevMessage->command == 0xcc) 
    {
        // NOT WORKING HERE
        
        pipeCtx->pingXOR = phevMessage->data[0];
        //pipeCtx->commandXOR = phevMessage->data[0];
        //LOG_I(APP_TAG,"%02X command recieved XOR changed to %02X",phevMessage->command, pipeCtx->pingXOR);
        // NOT WORKING HERE  
    } 
    if(phevMessage->command == 0x3f)
    {
        pipeCtx->pingResponse = phevMessage->reg;    
        LOG_D(APP_TAG,"Server Ping %d\n",phevMessage->reg);
        
    }
    
    LOG_D(APP_TAG, "Command %02x Register %d Length %d Type %d XOR %02X", phevMessage->command, phevMessage->reg, phevMessage->length, phevMessage->type, phevMessage->XOR);
    LOG_BUFFER_HEXDUMP(APP_TAG, phevMessage->data, phevMessage->length, LOG_DEBUG);
    
    phev_core_destroyMessage(phevMessage);

    //free(phevMessage);
    return message;
    
}
message_t *phev_pipe_commandResponder(void *ctx, message_t *message)
{
    LOG_V(APP_TAG, "START - commandResponder");
    phev_pipe_ctx_t *pipeCtx = (phev_pipe_ctx_t *)ctx;

    message_t *out = NULL;
    message_t *ret = NULL;

    if (message != NULL)
    {

        phevMessage_t phevMsg;

        phev_core_decodeMessage(message->data, message->length, &phevMsg);

        LOG_D(APP_TAG, "Decoded message XOR %02x", phevMsg.XOR);
        if (phevMsg.command == PING_RESP_CMD || phevMsg.command == PING_RESP_CMD_MY18 || phevMsg.command == 0xbb || phevMsg.command == 0xcd || phevMsg.command == 0xcc)
        {
            LOG_D(APP_TAG, "Ignoring ping");
            LOG_V(APP_TAG, "END - commandResponder");
            free(phevMsg.data);
            return NULL;
        }
        if(phevMsg.command == 0x4e || phevMsg.command == 0x5e)
        {
            LOG_D(APP_TAG, "%02X Command does not get encrypted response",phevMsg.command);
            LOG_BUFFER_HEXDUMP(APP_TAG,phevMsg.data,phevMsg.length,LOG_DEBUG);
            phevMessage_t *msg = phev_core_responseHandler(&phevMsg);
            LOG_D(APP_TAG, "Responded with command %02X  type %d", phevMsg.command,phevMsg.type);
            out = phev_core_convertToMessage(msg);
            pipeCtx->encrypt = true;
            free(phevMsg.data);
            return out;
        }
        if(pipeCtx->registerDevice == true) 
        {
            //This is a hack to keep registration working
            LOG_I(APP_TAG,"Not responding to command for registration");
            free(phevMsg.data);
            return NULL;
        } 
        
        LOG_D(APP_TAG, "Responding to %02X %02X", phevMsg.command, phevMsg.type);
        if (phevMsg.type == REQUEST_TYPE)
        {
            phevMessage_t *msg = phev_core_responseHandler(&phevMsg);
            LOG_D(APP_TAG, "Responded with command %02X  type %d", phevMsg.command,phevMsg.type);
        
            out = phev_core_convertToMessage(msg);
            free(phevMsg.data);
        }
    }
    if (out)
    {            
        message_t * encoded = phev_core_XOROutboundMessage(out, phev_core_getMessageXOR(message));
        
        ret = msg_utils_copyMsg(encoded);
        msg_utils_destroyMsg(encoded);
        msg_utils_destroyMsg(out);
    
    } 

#ifndef NO_CMD_RESP
    return ret;
#else
    return NULL;
#endif
}

phevPipeEvent_t *phev_pipe_createVINEvent(uint8_t *data)
{
    LOG_V(APP_TAG, "START - createVINEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));
    LOG_BUFFER_HEXDUMP(APP_TAG,data,17,LOG_INFO);

    if (data[19] < 3)
    {
        phevVinEvent_t *vinEvent = malloc(sizeof(phevVinEvent_t));
        event->event = PHEV_PIPE_GOT_VIN,
        event->data = (uint8_t *)vinEvent;
        event->length = sizeof(phevVinEvent_t);
        memcpy(vinEvent->vin, data + 1, VIN_LEN);
        //vinEvent->vin[VIN_LEN + 1] = 0;
        vinEvent->registrations = data[19];
    }
    else
    {
        event->event = PHEV_PIPE_MAX_REGISTRATIONS,
        event->data = NULL;
        event->length = 0;
    }

    LOG_D(APP_TAG, "Created Event ID %d", event->event);
    LOG_BUFFER_HEXDUMP(APP_TAG, event->data, event->length, LOG_DEBUG);
    LOG_V(APP_TAG, "END - createVINEvent");

    return event;
}

phevPipeEvent_t *phev_pipe_AAResponseEvent(void)
{
    LOG_V(APP_TAG, "START - AAResponseEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));

    event->event = PHEV_PIPE_CONNECTED,
    event->data = NULL;
    event->length = 0;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - AAResponseEvent");

    return event;
}
phevPipeEvent_t *phev_pipe_startResponseEvent(void)
{
    LOG_V(APP_TAG, "START - startResponseEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));

    event->event = PHEV_PIPE_START_ACK,
    event->data = NULL;
    event->length = 0;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - startResponseEvent");

    return event;
}

phevPipeEvent_t *phev_pipe_registrationEvent(void)
{
    LOG_V(APP_TAG, "START - registrationEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));

    event->event = PHEV_PIPE_REGISTRATION,
    event->data = NULL;
    event->length = 0;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - registrationEvent");

    return event;
}
phevPipeEvent_t *phev_pipe_ecuVersion2Event(uint8_t *data)
{
    LOG_V(APP_TAG, "START - ecuVersion2Event");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));

    event->event = PHEV_PIPE_ECU_VERSION2;
    event->data = malloc(PHEV_PIPE_ECU_VERSION_SIZE);
    memcpy(event->data, data, PHEV_PIPE_ECU_VERSION_SIZE);
    event->length = PHEV_PIPE_ECU_VERSION_SIZE;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - ecuVersion2Event");

    return event;
}
phevPipeEvent_t *phev_pipe_remoteSecurityPresentInfoEvent(void)
{
    LOG_V(APP_TAG, "START - remoteSecurityPresentInfoEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));

    event->event = PHEV_PIPE_REMOTE_SECURTY_PRSNT_INFO,
    event->data = NULL;
    event->length = 0;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - remoteSecurityPresentInfoEvent");

    return event;
}
phevPipeEvent_t *phev_pipe_regDispEvent(void)
{
    LOG_V(APP_TAG, "START - regDispEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));

    event->event = PHEV_PIPE_REG_DISP,
    event->data = NULL;
    event->length = 0;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - regDispEvent");

    return event;
}
phevPipeEvent_t *phev_pipe_dateInfoEvent(uint8_t *data)
{
    LOG_V(APP_TAG, "START - dateInfoEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));
    event->data = malloc(PHEV_PIPE_DATE_INFO_SIZE);
    memcpy(event->data, data, PHEV_PIPE_DATE_INFO_SIZE);
    event->event = PHEV_PIPE_DATE_INFO,
    event->length = PHEV_PIPE_DATE_INFO_SIZE;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - dateInfoEvent");

    return event;
}
phevPipeEvent_t *phev_pipe_registrationCompleteEvent(phev_pipe_ctx_t * ctx)
{
    LOG_V(APP_TAG, "START - registrationCompleteEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));
    event->event = PHEV_PIPE_REGISTRATION_COMPLETE;
    event->data = NULL;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - registrationCompleteEvent");
    return event;
}
void phev_pipe_sendRegister(phev_pipe_ctx_t * ctx)
{
    LOG_V(APP_TAG,"START - sendRegister");
    phevMessage_t * reg = phev_core_simpleRequestCommandMessage(KO_WF_REG_DISP_SP,1);
    message_t * message = phev_core_convertToMessage(reg);

    phev_pipe_commandOutboundPublish(ctx,  message);

    LOG_V(APP_TAG,"END - sendRegister");
    
}
phevPipeEvent_t *phev_pipe_createBBEvent(const uint8_t * data)
{
    LOG_V(APP_TAG, "START - BBEvent");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));
    event->data = malloc(1);
    memcpy(event->data, data, 1);
    event->event = PHEV_PIPE_BB,
    event->length = 1;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - BBEvent");

    return event;
}
phevPipeEvent_t *phev_pipe_createPingEvent(const uint8_t reg)
{
    LOG_V(APP_TAG, "START - Ping Event");
    phevPipeEvent_t *event = malloc(sizeof(phevPipeEvent_t));
    event->data = malloc(1);
    memcpy(event->data, &reg, 1);
    event->event = PHEV_PIPE_PING_RESP;
    event->length = 1;
    LOG_D(APP_TAG, "Created Event ID %d", event->event);

    LOG_V(APP_TAG, "END - Ping Event");

    return event;

}
phevPipeEvent_t *phev_pipe_messageToEvent(phev_pipe_ctx_t *ctx, phevMessage_t *phevMessage)
{
    LOG_V(APP_TAG, "START - messageToEvent");
    LOG_D(APP_TAG, "Message to Event Reg %d Len %d Type %d", phevMessage->reg, phevMessage->length, phevMessage->type);
    phevPipeEvent_t *event = NULL;
 
    if(phevMessage->command == 0xbb || phevMessage->command == 0xcc)
    {
        event = phev_pipe_createBBEvent(phevMessage->data);
        return event;
    }
    if (phevMessage->command == PING_RESP_CMD || phevMessage->command == PING_RESP_CMD_MY18)
    {
        event = phev_pipe_createPingEvent(phevMessage->reg);
        return event;
    }

    switch (phevMessage->reg)
    {
    case KO_WF_VIN_INFO_EVR:
    {
        LOG_D(APP_TAG, "KO_WF_VIN_INFO_EVR");
        if (phevMessage->type == REQUEST_TYPE)
        {
            event = phev_pipe_createVINEvent(phevMessage->data);
        }
        break;
    }
    case KO_WF_REG_DISP_SP: 
    {
        LOG_D(APP_TAG, "KO_WF_REG_DISP_SP");
        if (phevMessage->type == RESPONSE_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            LOG_I(APP_TAG,"Registration Acknowledged");
            event = phev_pipe_registrationCompleteEvent(ctx);
            LOG_I(APP_TAG,"REGISTERED");
        }
        
        break;
    }
    case KO_WF_CONNECT_INFO_GS_SP:
    {
        if (phevMessage->type == RESPONSE_TYPE && (phevMessage->command == START_RESP || phevMessage->command == START_RESP_MY18))
        {
            LOG_D(APP_TAG, "KO_WF_CONNECT_INFO_GS_SP");
            event = phev_pipe_startResponseEvent();
        }
        break;
    }
    case KO_WF_START_AA_EVR:
    {
        if (phevMessage->type == RESPONSE_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            LOG_D(APP_TAG, "KO_WF_START_AA_EVR");
            event = phev_pipe_AAResponseEvent();
        }
        break;
    }
    case KO_WF_REGISTRATION_EVR:
    {
        if (phevMessage->type == REQUEST_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            LOG_D(APP_TAG,"KO_WF_REGISTRATION_EVR");
            event = phev_pipe_registrationEvent();
        }
        break;
    }
    case KO_WF_ECU_VERSION2_EVR:
    {
        if (phevMessage->type == REQUEST_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            LOG_D(APP_TAG,"KO_WF_ECU_VERSION2_EVR");
            event = phev_pipe_ecuVersion2Event(phevMessage->data);
        }
        break;
    }
    case KO_WF_REMOTE_SECURTY_PRSNT_INFO:
    {

        if (phevMessage->type == REQUEST_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            event = phev_pipe_remoteSecurityPresentInfoEvent();
        }
        break;
    }
    case KO_WF_DATE_INFO_SYNC_EVR:
    {
        if (phevMessage->type == REQUEST_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            event = phev_pipe_dateInfoEvent(phevMessage->data);
        }
        break;
    }
    case KO_WF_BATT_LEVEL_INFO_REP_EVR:
    {
        if(phevMessage->type == REQUEST_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            LOG_D(APP_TAG,"Battery level %d", phevMessage->data[0]);
        }
        break;
    }
    default:
    {
        LOG_D(APP_TAG, "Command %02X Register not handled %02X by pipe event loop", phevMessage->command, phevMessage->reg);
    }
    }

    LOG_V(APP_TAG, "END - messageToEvent");
    return event;
}
void phev_pipe_sendEventToHandlers(phev_pipe_ctx_t *ctx, phevPipeEvent_t *event)
{
    LOG_V(APP_TAG, "START - sendEventToHandlers");

    if (event != NULL)
    {
        LOG_D(APP_TAG, "Sending event ID %d", event->event);
        if (ctx->eventHandlers > 0)
        {
            LOG_D(APP_TAG, "Event handers %d", ctx->eventHandlers);
            for (int i = 0; i < ctx->eventHandlers; i++)
            {
                LOG_D(APP_TAG, "Event handler num %d", i);

                if (ctx->eventHandler[i] != NULL)
                {
                    LOG_D(APP_TAG, "Calling event handler %d pointer %p", i, ctx->eventHandler[i]);
                    ctx->eventHandler[i](ctx, event);
                }
            }
        }
        phev_pipe_destroyEvent(event);
    }
    else
    {
        LOG_D(APP_TAG, "Not sending NULL event");
    }
    LOG_V(APP_TAG, "END - sendEventToHandlers");
}
phevPipeEvent_t *phev_pipe_createRegisterEvent(phev_pipe_ctx_t *phevCtx, phevMessage_t *phevMessage)
{
    phevPipeEvent_t *event = NULL;

    if (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18)
    {
        event = malloc(sizeof(phevPipeEvent_t));
        event->data = (void *)phev_core_copyMessage(phevMessage);
        event->length = sizeof(phevMessage_t);
        event->ctx = phevCtx;
        if (phevMessage->type == RESPONSE_TYPE)
        {
            event->event = PHEV_PIPE_REG_UPDATE_ACK;
        }
        else
        {
            event->event = PHEV_PIPE_REG_UPDATE;
        }
    }

    return event;
}
void phev_pipe_sendEvent(void *ctx, phevMessage_t *phevMessage)
{
    LOG_V(APP_TAG, "START - sendEvent");

    phev_pipe_ctx_t *phevCtx = (phev_pipe_ctx_t *)ctx;

    if (phevCtx == NULL)
    {
        LOG_W(APP_TAG, "Context not passed");
        return;
    }
     LOG_D(APP_TAG, "Number of event handlers %d",phevCtx->eventHandlers);
    if (phevCtx->eventHandlers > 0)
    {

        phevPipeEvent_t *registerEvent = phev_pipe_createRegisterEvent(phevCtx, phevMessage);

        LOG_D(APP_TAG, "Sending register event to handler");
        phev_pipe_sendEventToHandlers(phevCtx, registerEvent);
        
        phevPipeEvent_t *evt = phev_pipe_messageToEvent(phevCtx, phevMessage);
        LOG_D(APP_TAG, "Sending message event to handler");

        phev_pipe_sendEventToHandlers(phevCtx, evt);

    }

    LOG_V(APP_TAG, "END - sendEvent");
}
message_t *phev_pipe_outputEventTransformer(void *ctx, message_t *message)
{
    LOG_V(APP_TAG, "START - outputEventTransformer");

    phevMessage_t *phevMessage = malloc(sizeof(phevMessage_t));

    int length = phev_core_decodeMessage(message->data, message->length, phevMessage);

    if (length == 0)
    {
        LOG_E(APP_TAG, "Invalid message received - something serious happened here as we should only have a valid message at this point");
        LOG_BUFFER_HEXDUMP(APP_TAG, message->data, message->length, LOG_DEBUG);
        
        return NULL;
    }

    phev_pipe_sendEvent(ctx, phevMessage);

//    message_t *ret = phev_core_convertToMessage(phevMessage);

    phev_core_destroyMessage(phevMessage);

//    LOG_V(APP_TAG, "END - outputEventTransformer");

    return NULL; //ret;
}

void phev_pipe_registerEventHandler(phev_pipe_ctx_t *ctx, phevPipeEventHandler_t eventHandler)
{
    LOG_V(APP_TAG, "START - registerEventHandler");

    if (ctx->eventHandlers < PHEV_PIPE_MAX_EVENT_HANDLERS)
    {
        for (int i = 0; i < PHEV_PIPE_MAX_EVENT_HANDLERS; i++)
        {
            if (ctx->eventHandler[i] == NULL)
            {
                LOG_D(APP_TAG, "Registered handler %p", eventHandler);
                ctx->eventHandler[ctx->eventHandlers++] = eventHandler;
                return;
            }
        }
        LOG_E(APP_TAG, "Cannot register handler no free slots found");
    }
    else
    {
        LOG_E(APP_TAG, "Cannot register handler max handlers %d reached", PHEV_PIPE_MAX_EVENT_HANDLERS);
    }
    LOG_V(APP_TAG, "END - registerEventHandler");
}
void phev_pipe_deregisterEventHandler(phev_pipe_ctx_t *ctx, phevPipeEventHandler_t eventHandler)
{
    LOG_V(APP_TAG, "START - deregisterEventHandler");

    for (int i = 0; i < PHEV_PIPE_MAX_EVENT_HANDLERS; i++)
    {
        if (ctx->eventHandler[i] == eventHandler)
        {
            LOG_D(APP_TAG, "Deregistered handler");
            ctx->eventHandler[i--] = NULL;
            return;
        }
    }

    LOG_V(APP_TAG, "END - deregisterEventHandler");
}

void phev_pipe_checkXORChanged(phev_pipe_ctx_t * ctx, message_t * message)
{
    if(message->ctx != NULL)
    {
        uint8_t xor =phev_core_getMessageXOR(message);
    
        if(xor == ctx->currentXOR)
        {
            ctx->currentXOR = xor;
            LOG_D(APP_TAG,"XOR changed to %02X from %02X",xor,ctx->currentXOR);
        }
    }
}
messageBundle_t *phev_pipe_outputSplitter(void *ctx, message_t *message)
{
    LOG_V(APP_TAG, "START - outputSplitter");

    phev_pipe_ctx_t * pipeCtx  = (phev_pipe_ctx_t *) ctx;

    if(ctx == NULL)
    {
        LOG_E(APP_TAG,"Pipe context not passed to splitter");
        return NULL;
    }

    if(message == NULL)
    {
        LOG_E(APP_TAG,"Message not passed to splitter");
        return NULL;
    }
    LOG_BUFFER_HEXDUMP(APP_TAG, message->data, message->length, LOG_DEBUG);
    
    message_t * out = phev_core_extractIncomingMessageAndXOR(message->data);

    if (out == NULL)
    {
        LOG_E(APP_TAG,"Could not extract message");
        return NULL;
    }
    LOG_D(APP_TAG,"Extract message output");
    LOG_BUFFER_HEXDUMP(APP_TAG, out->data, out->length, LOG_DEBUG);

    phev_pipe_checkXORChanged(pipeCtx, out);

    messageBundle_t *messages = malloc(sizeof(messageBundle_t));

    messages->numMessages = 0;
    messages->messages[messages->numMessages++] = msg_utils_copyMsg(out);

    int total = out->length;

    msg_utils_destroyMsg(out);

    while (message->length > total)
    {
        out = phev_core_extractIncomingMessageAndXOR(message->data + total);
        if (out == NULL) {
            break;
        }

        LOG_D(APP_TAG,"Extract message output");
        LOG_BUFFER_HEXDUMP(APP_TAG, out->data, out->length, LOG_DEBUG);
        phev_pipe_checkXORChanged(pipeCtx,out);
        total += out->length;
        messages->messages[messages->numMessages++] = msg_utils_copyMsg(out);
        msg_utils_destroyMsg(out);
    }

    //msg_utils_destroyMsg(message); // Cannot destroy until tests are fixed
    LOG_D(APP_TAG, "Split messages into %d", messages->numMessages);
    LOG_MSG_BUNDLE(APP_TAG, messages);
    LOG_V(APP_TAG, "END - outputSplitter");
    return messages;
}

void phev_pipe_sendTimeSync(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG, "START - sendTimeSync");

    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    const uint8_t pingTime[] = {
        timeinfo->tm_year - 100,
        timeinfo->tm_mon + 1,
        timeinfo->tm_mday,
        timeinfo->tm_hour,
        timeinfo->tm_min,
        timeinfo->tm_sec,
        1};
    LOG_D(APP_TAG, "Year %d Month %d Date %d Hour %d Min %d Sec %d\n", pingTime[0], pingTime[1], pingTime[2], pingTime[3], pingTime[4], pingTime[5]);

    phevMessage_t *dateCmd = phev_core_commandMessage(KO_WF_DATE_INFO_SYNC_SP, pingTime, sizeof(pingTime));
    message_t *message = phev_core_convertToMessage(dateCmd);
    
#ifndef NO_TIME_SYNC
    phev_pipe_commandOutboundPublish(ctx, message);
#endif

    LOG_V(APP_TAG, "END - sendTimeSync");
}
void phev_pipe_ping(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG, "START - ping");

    if(ctx->encrypt && ctx->pingXOR == 0)
    {
        LOG_I(APP_TAG,"Not sending ping after start message recieved if not got XOR");
        return;
    }
    if (((ctx->currentPing) % 30) == 0)
    {
        if(!ctx->registerDevice)
        {   
            phev_pipe_sendTimeSync(ctx);
        } 
        else
        {
            LOG_D(APP_TAG,"Not sending time sync in register device mode");
        }
    }
    phevMessage_t *ping = phev_core_pingMessage(ctx->currentPing++);
    ctx->currentPing %= 0x30;
    LOG_D(APP_TAG,"Client Ping %d\n",ctx->currentPing);
    message_t *message = phev_core_convertToMessage(ping);
    
#ifndef NO_PING
    if(!ctx->registerDevice)
    {
        phev_pipe_pingOutboundPublish(ctx, message);
    } 
    else 
    {
        LOG_I(APP_TAG,"Not sending ping in register device mode");
    }

#endif
    //msg_utils_destroyMsg(message);
    //phev_core_destroyMessage(ping);
    LOG_V(APP_TAG, "END - ping");
}
void phev_pipe_updateComplexRegister(phev_pipe_ctx_t *ctx, const uint8_t reg, const uint8_t * data, const size_t length)
{
    phev_pipe_updateComplexRegisterWithCallback(ctx, reg, data, length, NULL, NULL);
}
void phev_pipe_updateRegister(phev_pipe_ctx_t *ctx, const uint8_t reg, const uint8_t value)
{
    phev_pipe_updateRegisterWithCallback(ctx, reg, value, NULL, NULL);
}

void phev_pipe_updateRegisterNoRetry(phev_pipe_ctx_t *ctx, const uint8_t reg, const uint8_t * data, const size_t length)
{
    LOG_V(APP_TAG, "START - updateRegister");

    phevMessage_t *update = NULL;

    if(data == NULL)
    {
        LOG_W(APP_TAG,"Cannot send data with no data");
        return;
    }
    
    if(length == 1)
    {
        update = phev_core_simpleRequestCommandMessage(reg, data[0]);
    } 
    else
    {
        update = phev_core_commandMessage(reg,data,length);
    } 
    
    message_t *message = phev_core_convertToMessage(update);
    
    phev_pipe_commandOutboundPublish(ctx, message);

    LOG_V(APP_TAG, "END - updateRegister");
}

int phev_pipe_updateRegisterEventHandler(phev_pipe_ctx_t *ctx, phevPipeEvent_t *event)
{
    LOG_V(APP_TAG, "START - updateRegisterEventHandler");

    
    if(!event)
    { 
        return 0;
    }

    LOG_D(APP_TAG, "Register callbacks %d",ctx->updateRegisterCallbacks->numberOfCallbacks);

    if(ctx->updateRegisterCallbacks->numberOfCallbacks == 0)
    {
        LOG_D(APP_TAG,"No register events");
        return 0;
    }
    if (event->event == PHEV_PIPE_BB && ctx->updateRegisterCallbacks->numberOfCallbacks > 0)
    {
        LOG_D(APP_TAG,"Resending commands");
        for(int i=0; i< PHEV_PIPE_MAX_UPDATE_CALLBACKS; i++)
        {
            if(ctx->updateRegisterCallbacks->used[i])
            {
                phev_pipe_updateRegisterNoRetry(ctx, ctx->updateRegisterCallbacks->registers[i], ctx->updateRegisterCallbacks->values[i],ctx->updateRegisterCallbacks->lengths[i]);
            }
        }
        
    }
    if (event->event == PHEV_PIPE_REG_UPDATE_ACK)
    {
        for (int i = 0; i < PHEV_PIPE_MAX_UPDATE_CALLBACKS; i++)
        {
            if (ctx->updateRegisterCallbacks->used[i] && ctx->updateRegisterCallbacks->registers[i] == ((phevMessage_t *)event->data)->reg)
            {
                if(ctx->updateRegisterCallbacks->callbacks[i]!= NULL)
                {
                    ctx->updateRegisterCallbacks->callbacks[i](ctx, ctx->updateRegisterCallbacks->registers[i], ctx->updateRegisterCallbacks->ctx[i]);
                }
                ctx->updateRegisterCallbacks->callbacks[i] = NULL;
                ctx->updateRegisterCallbacks->registers[i] = 0;
                free(ctx->updateRegisterCallbacks->values[i]);
                ctx->updateRegisterCallbacks->values[i] = NULL;
                ctx->updateRegisterCallbacks->lengths[i] = 0;
                
                ctx->updateRegisterCallbacks->used[i] = false;

                ctx->updateRegisterCallbacks->numberOfCallbacks--;
            }
        }
    }
    LOG_V(APP_TAG, "END - updateRegisterEventHandler");

    return 0;
}
void phev_pipe_updateRegisterWithCallback(phev_pipe_ctx_t *ctx, const uint8_t reg, const uint8_t value, phev_pipe_updateRegisterCallback_t callback, void *customCtx)
{
    LOG_V(APP_TAG, "START - updateRegisterWithCallback");

    uint8_t * data = malloc(1);
    data[0] = value;
            
    phev_pipe_updateComplexRegisterWithCallback(ctx, reg, data, 1, callback, customCtx);

    LOG_V(APP_TAG, "END - updateRegisterWithCallback");

}
void phev_pipe_updateComplexRegisterWithCallback(phev_pipe_ctx_t *ctx, const uint8_t reg, const uint8_t * data, const size_t length, phev_pipe_updateRegisterCallback_t callback, void *customCtx)
{
    LOG_V(APP_TAG, "START - updateRegisterWithCallback");

    for (int i = 0; i < PHEV_PIPE_MAX_UPDATE_CALLBACKS; i++)
    {
        if (ctx->updateRegisterCallbacks->used[i] == false)
        {
            uint8_t * dataCopy = malloc(length);
            memcpy(dataCopy, data, length);
            ctx->updateRegisterCallbacks->used[i] = true;
            ctx->updateRegisterCallbacks->callbacks[i] = callback;
            ctx->updateRegisterCallbacks->registers[i] = reg;
            ctx->updateRegisterCallbacks->values[i] = dataCopy;
            ctx->updateRegisterCallbacks->lengths[i] = length;
            ctx->updateRegisterCallbacks->ctx[i] = customCtx;

            ctx->updateRegisterCallbacks->numberOfCallbacks++;

            phev_pipe_registerEventHandler(ctx, (phevPipeEventHandler_t)phev_pipe_updateRegisterEventHandler);

            phev_pipe_updateRegisterNoRetry(ctx, reg, data, length);

            LOG_V(APP_TAG, "END - updateRegisterWithCallback");
            return;
        }
    }

    LOG_W(APP_TAG, "Cannot add update register handler too many allocated %d",ctx->updateRegisterCallbacks->numberOfCallbacks);
}

void phev_pipe_pingOutboundPublish(phev_pipe_ctx_t * ctx, message_t * message)
{
    LOG_V(APP_TAG,"START - pingOutboundPublish");

    message_t * encoded = phev_core_XOROutboundMessage(message, ctx->pingXOR);
        
    msg_pipe_outboundPublish(ctx->pipe, encoded);
    
    msg_utils_destroyMsg(message);

    LOG_V(APP_TAG,"END - pingOutboundPublish");

    return;
}
void phev_pipe_commandOutboundPublish(phev_pipe_ctx_t * ctx, message_t * message)
{
    LOG_V(APP_TAG,"START - commandOutboundPublish");

    message_t * encoded = phev_core_XOROutboundMessage(message, ctx->commandXOR);
        
    msg_pipe_outboundPublish(ctx->pipe, encoded);
    
    msg_utils_destroyMsg(message);

    LOG_V(APP_TAG,"END - commandOutboundPublish");

    return;
}
void phev_pipe_outboundPublish(phev_pipe_ctx_t * ctx, message_t * message)
{
    LOG_V(APP_TAG,"START - outboundPublish");

    message_t * encoded = phev_core_XOROutboundMessage(message, ctx->currentXOR);
        
    msg_pipe_outboundPublish(ctx->pipe, encoded);

    msg_utils_destroyMsg(message);

    LOG_V(APP_TAG,"END - outboundPublish");

    return;
}
