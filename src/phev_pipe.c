#include "phev_pipe.h"
#include "phev_core.h"
#include "msg_utils.h"
#include "logger.h"

//#define NO_PING
#define NO_ENC_CMD_RESP
//#define NO_TIME_SYNC

const static char *APP_TAG = "PHEV_PIPE";

void phev_pipe_waitForConnection(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG, "START - waitForConnection");
    ctx->connected = false;
    int retries = 0;

    while (!(ctx->pipe->in->connected && ctx->pipe->out->connected))
    {
        LOG_I(APP_TAG, "Not connected waiting...");
        if (!ctx->pipe->in->connected)
        {
            msg_pipe_in_connect(ctx->pipe);
        }
        if (!ctx->pipe->out->connected)
        {
            msg_pipe_out_connect(ctx->pipe);
        }
        SLEEP(PHEV_CONNECT_WAIT_TIME);
        retries++;
        if (retries > PHEV_CONNECT_MAX_RETRIES)
        {
            LOG_E(APP_TAG, "Max retries reached");
            return;
        }
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

    //free(message);
    LOG_V(APP_TAG, "END - sendMac");
}
void phev_pipe_start(phev_pipe_ctx_t *ctx, uint8_t *mac)
{
    LOG_V(APP_TAG, "START - start");

    phev_pipe_waitForConnection(ctx);

    phev_pipe_sendMac(ctx, mac);
    LOG_V(APP_TAG, "END - start");
}

phev_pipe_ctx_t *phev_pipe_create(messagingClient_t *in, messagingClient_t *out)
{
    LOG_V(APP_TAG, "START - create");

    phev_pipe_settings_t settings = {
        .ctx = NULL,
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = phev_pipe_outputSplitter,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t)phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t)phev_pipe_outputEventTransformer,
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t)phev_pipe_outputChainInputTransformer,
    };

    phev_pipe_ctx_t *ctx = phev_pipe_createPipe(settings);

    LOG_V(APP_TAG, "END - create");

    return ctx;
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
    }
    ctx->connected = false;
    ctx->ctx = settings.ctx;
    ctx->currentXOR = 0;

    phev_pipe_resetPing(ctx);

    LOG_V(APP_TAG, "END - createPipe");

    return ctx;
}

message_t *phev_pipe_outputChainInputTransformer(void *ctx, message_t *message)
{
    LOG_V(APP_TAG, "START - outputChainInputTransformer");
    phevMessage_t *phevMessage = malloc(sizeof(phevMessage_t));
    phev_pipe_ctx_t *pipeCtx = (phev_pipe_ctx_t *)ctx;

    int ret = phev_core_decodeMessage(message->data, message->length, phevMessage);
    
    if (ret == 0)
    {
        LOG_E(APP_TAG, "Invalid message received");
        LOG_BUFFER_HEXDUMP(APP_TAG, message->data, message->length, LOG_ERROR);

        return NULL;
    }

    if(phevMessage->command == 0xbb)
    {
        pipeCtx->currentXOR = phevMessage->data[0];
        LOG_D(APP_TAG,"XOR changed to %02X",pipeCtx->currentXOR);
    }

    LOG_D(APP_TAG, "Command %02x Register %d Length %d Type %d XOR %02X", phevMessage->command, phevMessage->reg, phevMessage->length, phevMessage->type, phevMessage->XOR);
    LOG_BUFFER_HEXDUMP(APP_TAG, phevMessage->data, phevMessage->length, LOG_DEBUG);
    message_t * out = phev_core_XORInboundMessage(message,phevMessage->XOR);
    
    LOG_V(APP_TAG, "END - outputChainInputTransformer");

    return out;
}
message_t *phev_pipe_commandResponder(void *ctx, message_t *message)
{
    LOG_V(APP_TAG, "START - commandResponder");
    phev_pipe_ctx_t *pipeCtx = (phev_pipe_ctx_t *)ctx;

    LOG_D(APP_TAG, "CTX XOR %02x", pipeCtx->currentXOR);
    message_t *out = NULL;

    if (message != NULL)
    {

        phevMessage_t phevMsg;

        phev_core_decodeMessage(message->data, message->length, &phevMsg);

        LOG_D(APP_TAG, "Decoded message XOR %02x", phevMsg.XOR);
        if (phevMsg.command == PING_RESP_CMD || phevMsg.command == PING_RESP_CMD_MY18 || phevMsg.command == 0xbb || phevMsg.command == 0xcd || phevMsg.command == 0xcc)
        {
            LOG_D(APP_TAG, "Ignoring ping");
            LOG_V(APP_TAG, "END - commandResponder");
            return NULL;
        }
        LOG_D(APP_TAG, "Responding to %02X %02X", phevMsg.command, phevMsg.type);
        if (phevMsg.type == REQUEST_TYPE)
        {
            phevMessage_t *msg = phev_core_responseHandler(&phevMsg);
            LOG_D(APP_TAG, "Responded with command %02X  type %d", phevMsg.command,phevMsg.type);
            out = phev_core_convertToMessage(msg);
        }
    }
    if (out)
    {
        LOG_D(APP_TAG, "Responding with");
        LOG_BUFFER_HEXDUMP(APP_TAG, out->data, out->length, LOG_DEBUG);

        //if(pipeCtx->currentXOR == 0)
        //{
            return out;
        //}
        message_t * encoded = phev_core_XOROutboundMessage(out,pipeCtx->currentXOR);
        //pipeCtx->currentXOR = rand();
        if(encoded) 
        {
            LOG_D(APP_TAG, "Responding with encrpyted");
            LOG_BUFFER_HEXDUMP(APP_TAG, encoded->data, encoded->length, LOG_DEBUG);
            LOG_V(APP_TAG, "END - commandResponder");
            
#ifndef NO_ENC_CMD_RESP
            return encoded;
#else
            LOG_D(APP_TAG,"Response not sent as configured off");
            return NULL;
#endif
        }
    }
    else
    {
        LOG_D(APP_TAG, "No response");
    }
    LOG_V(APP_TAG, "END - commandResponder");
    return out;
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
    event->length = 0;
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
phevPipeEvent_t *phev_pipe_messageToEvent(phev_pipe_ctx_t *ctx, phevMessage_t *phevMessage)
{
    LOG_V(APP_TAG, "START - messageToEvent");
    LOG_D(APP_TAG, "Message to Event Reg %d Len %d Type %d", phevMessage->reg, phevMessage->length, phevMessage->type);
    phevPipeEvent_t *event = NULL;
 
    if (phevMessage->command == PING_RESP_CMD || phevMessage->command == PING_RESP_CMD_MY18 || phevMessage->command == 0xbb || phevMessage->command == 0xcd)
    {
        LOG_D(APP_TAG, "Ignoring ping");
        return NULL;
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
            event = phev_pipe_registrationEvent();
        }
        break;
    }
    case KO_WF_ECU_VERSION2_EVR:
    {
        if (phevMessage->type == REQUEST_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
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
    case KO_WF_REG_DISP_SP:
    {
        if (phevMessage->type == RESPONSE_TYPE && (phevMessage->command == RESP_CMD || phevMessage->command == RESP_CMD_MY18))
        {
            event = phev_pipe_regDispEvent();
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
    if (phevCtx->eventHandlers > 0)
    {

        phevPipeEvent_t *registerEvent = phev_pipe_createRegisterEvent(phevCtx, phevMessage);

        LOG_D(APP_TAG, "START - sending register event to handler");
        phev_pipe_sendEventToHandlers(phevCtx, registerEvent);

        phevPipeEvent_t *evt = phev_pipe_messageToEvent(phevCtx, phevMessage);
        LOG_D(APP_TAG, "START - sending message event to handler");

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

    message_t *ret = phev_core_convertToMessage(phevMessage);

    phev_core_destroyMessage(phevMessage);

    LOG_V(APP_TAG, "END - outputEventTransformer");

    return ret;
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

messageBundle_t *phev_pipe_outputSplitter(void *ctx, message_t *message)
{
    LOG_V(APP_TAG, "START - outputSplitter");

    LOG_BUFFER_HEXDUMP(APP_TAG, message->data, message->length, LOG_DEBUG);
    message_t *out = phev_core_extractMessage(message->data, message->length);

    if (out == NULL)
        return NULL;
    messageBundle_t *messages = malloc(sizeof(messageBundle_t));

    messages->numMessages = 0;
    messages->messages[messages->numMessages++] = out;

    int total = out->length;

    while (message->length > total)
    {
        out = phev_core_extractMessage(message->data + total, message->length - total);
        if (out != NULL)
        {
            total += out->length;
            messages->messages[messages->numMessages++] = out;
        }
        else
        {
            break;
        }
    }
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
    phev_pipe_outboundPublish(ctx, message);
#endif
    //msg_utils_destroyMsg(message);
    //phev_core_destroyMessage(dateCmd);
    LOG_V(APP_TAG, "END - sendTimeSync");
}
void phev_pipe_ping(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG, "START - ping");
    //printf("PING\n");
    if (((ctx->currentPing) % 30) == 0)
    {
        phev_pipe_sendTimeSync(ctx);
    }
    phevMessage_t *ping = phev_core_pingMessage(ctx->currentPing++);
    //ping->XOR = 0;
    //ctx->currentXOR = ctx->currentXOR ^ 1;
    message_t *message = phev_core_convertToMessage(ping);
    
#ifndef NO_PING
    phev_pipe_outboundPublish(ctx, message);
#endif
    //msg_utils_destroyMsg(message);
    //phev_core_destroyMessage(ping);
    LOG_V(APP_TAG, "END - ping");
}
void phev_pipe_resetPing(phev_pipe_ctx_t *ctx)
{
    LOG_V(APP_TAG, "START - resetPing");
    time_t now;

    time(&now);
    ctx->currentPing = 0;
    ctx->lastPingTime = now;
    LOG_V(APP_TAG, "END - resetPing");
}
void phev_pipe_updateRegister(phev_pipe_ctx_t *ctx, const uint8_t reg, const uint8_t value)
{
    LOG_V(APP_TAG, "START - updateRegister");
    phevMessage_t *update = phev_core_simpleRequestCommandMessage(reg, value);
    
    message_t *message = phev_core_convertToMessage(update);

    //message_t *encMessage = phev_core_XOROutboundMessage(message, ctx->currentXOR);
    
    phev_pipe_outboundPublish(ctx, message);

    LOG_V(APP_TAG, "END - updateRegister");
}

int phev_pipe_updateRegisterEventHandler(phev_pipe_ctx_t *ctx, phevPipeEvent_t *event)
{
    LOG_V(APP_TAG, "START - updateRegisterEventHandler");

    if (event->event == PHEV_PIPE_REG_UPDATE_ACK)
    {
        for (int i = 0; i < PHEV_PIPE_MAX_UPDATE_CALLBACKS; i++)
        {
            if (ctx->updateRegisterCallbacks->callbacks[i] != NULL && ctx->updateRegisterCallbacks->registers[i] == ((phevMessage_t *)event->data)->reg)
            {
                ctx->updateRegisterCallbacks->callbacks[i](ctx, ctx->updateRegisterCallbacks->registers[i], ctx->updateRegisterCallbacks->ctx[i]);
                ctx->updateRegisterCallbacks->callbacks[i] = NULL;
                ctx->updateRegisterCallbacks->registers[i] = 0;

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

    for (int i = 0; i < PHEV_PIPE_MAX_UPDATE_CALLBACKS; i++)
    {
        if (ctx->updateRegisterCallbacks->callbacks[i] == NULL)
        {
            ctx->updateRegisterCallbacks->callbacks[i] = callback;
            ctx->updateRegisterCallbacks->registers[i] = reg;
            ctx->updateRegisterCallbacks->ctx[i] = customCtx;

            ctx->updateRegisterCallbacks->numberOfCallbacks++;

            phev_pipe_registerEventHandler(ctx, (phevPipeEventHandler_t)phev_pipe_updateRegisterEventHandler);

            phev_pipe_updateRegister(ctx, reg, value);

            LOG_V(APP_TAG, "END - updateRegisterWithCallback");
            return;
        }
    }

    LOG_W(APP_TAG, "Cannot add update register handler too many allocated");
}
void phev_pipe_outboundPublish(phev_pipe_ctx_t * ctx, message_t * message)
{
    LOG_V(APP_TAG,"START - outboundPublish");

    if((message->data[2] < 2) && (ctx->currentXOR != 0))
    {
        message_t * encoded = phev_core_XOROutboundMessage(message, ctx->currentXOR);
        
        msg_pipe_outboundPublish(ctx->pipe, encoded);
        LOG_D(APP_TAG,"Encoded publish");
        LOG_V(APP_TAG,"END - outboundPublish");
        return;
    }
    LOG_D(APP_TAG,"Message already encoded publish");
            
    msg_pipe_outboundPublish(ctx->pipe, message);
    
    LOG_V(APP_TAG,"END - outboundPublish");

}