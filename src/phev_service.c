#include <stdint.h>
#include "phev_pipe.h"
#include "phev_service.h"
#include "msg_utils.h"
#include "logger.h"
#include "cJSON.h"

const static char *APP_TAG = "PHEV_SERVICE";

int phev_service_eventHandler(phev_pipe_ctx_t * ctx, phevPipeEvent_t * event)
{
    switch(event->event) 
    {
        case PHEV_PIPE_CONNECTED: {
            break;
        }
        default: {

        }
    }
    return 0;
}

phevServiceCtx_t * phev_service_create(phevServiceSettings_t settings)
{
    phevServiceCtx_t * ctx = NULL;

    if(settings.registerDevice) 
    {
        ctx = phev_service_initForRegistration(settings.in,settings.out);
    } else {
        ctx = phev_service_init(settings.in,settings.out);    
    }

    ctx->yieldHandler = settings.yieldHandler;
    ctx->exit = false;
    memcpy(ctx->mac,settings.mac,MAC_ADDR_SIZE);
    if(settings.eventHandler)
    {
        phev_pipe_registerEventHandler(ctx->pipe,settings.eventHandler);    
    }
    
    phev_pipe_registerEventHandler(ctx->pipe,phev_service_eventHandler);

    return ctx;
}

void phev_service_start(phevServiceCtx_t * ctx)
{

    phev_pipe_start(ctx->pipe,ctx->mac);

    while(!ctx->exit)
    {
        phev_service_loop(ctx);
        if(ctx->yieldHandler)
        {
            ctx->yieldHandler(ctx);
        }
    }
}
phevServiceCtx_t * phev_service_init(messagingClient_t *in, messagingClient_t *out)
{
    phevServiceCtx_t * ctx = malloc(sizeof(phevServiceCtx_t));
    
    ctx->model = phev_model_create();
    ctx->pipe = phev_service_createPipe(ctx, in, out);
    ctx->pipe->ctx = ctx;

    return ctx;
}
phevServiceCtx_t * phev_service_initForRegistration(messagingClient_t *in, messagingClient_t *out)
{
    phevServiceCtx_t * ctx = malloc(sizeof(phevServiceCtx_t));
    
    ctx->model = phev_model_create();
    ctx->pipe = phev_service_createPipeRegister(ctx, in, out);

    return ctx;
}
messageBundle_t * phev_service_inputSplitter(void * ctx, message_t * message)
{
    messageBundle_t * messages = malloc(sizeof(messageBundle_t));
    messages->numMessages = 0;
    cJSON * command = NULL;

    cJSON * json = cJSON_Parse(message->data);

    if(!json)
    {
        LOG_W(APP_TAG,"Not valid JSON");
        return NULL;
    }

    cJSON * requests = cJSON_GetObjectItemCaseSensitive(json,"requests");

    if(!requests)
    {
        LOG_W(APP_TAG,"Not valid JSON requests");
        return NULL;
    }
    
    
    cJSON_ArrayForEach(command, requests)
    {   
        const char * out = cJSON_Print(command);
        messages->messages[messages->numMessages++] = msg_utils_createMsg(out,strlen(out)+1);
    }

    return messages;
}
bool phev_service_outputFilter(void *ctx, message_t * message)
{
    
    phevServiceCtx_t * serviceCtx = ((phev_pipe_ctx_t *) ctx)->ctx;

    phevMessage_t phevMessage;
    
    phev_core_decodeMessage(message->data,message->length, &phevMessage);

    phevRegister_t * reg = phev_model_getRegister(serviceCtx->model, phevMessage.reg);
    
    if(reg)
    {
        return phev_model_compareRegister(serviceCtx->model,phevMessage.reg, phevMessage.data) != 0;
    }
    
    return true;
}
phev_pipe_ctx_t *phev_service_createPipe(phevServiceCtx_t * ctx, messagingClient_t *in, messagingClient_t *out)
{
    LOG_V(APP_TAG, "START - createPipe");

    phev_pipe_settings_t settings = {
        .ctx = ctx,
        .in = in,
        .out = out,
        .inputInputTransformer = phev_service_jsonInputTransformer,
        .inputOutputTransformer = NULL, 
        .inputSplitter = phev_service_inputSplitter,
        .inputAggregator = NULL,
        .outputAggregator = phev_service_jsonResponseAggregator,
        .outputSplitter = phev_pipe_outputSplitter,
        .outputFilter = phev_service_outputFilter,
        .inputResponder = NULL,
        .outputResponder = phev_pipe_commandResponder,
        .preConnectHook = NULL,
        .outputInputTransformer = phev_pipe_outputChainInputTransformer,
        .outputOutputTransformer = phev_service_jsonOutputTransformer,
    };

    phev_pipe_ctx_t * pipe = phev_pipe_createPipe(settings);

    LOG_V(APP_TAG, "END - createPipe");
    return pipe;
}

phev_pipe_ctx_t *phev_service_createPipeRegister(phevServiceCtx_t * ctx, messagingClient_t *in, messagingClient_t *out)
{
    LOG_V(APP_TAG, "START - createPipeRegister");

    phev_pipe_settings_t settings = {
        .ctx = ctx,
        .in = in,
        .out = out,
        .inputInputTransformer = NULL,
        .inputOutputTransformer = NULL, 
        .inputSplitter = NULL,
        .inputAggregator = NULL,
        .outputAggregator = NULL,
        .outputSplitter = phev_pipe_outputSplitter,
        .outputFilter = NULL,
        .inputResponder = NULL,
        .outputResponder = phev_pipe_commandResponder,
        .preConnectHook = NULL,
        .outputInputTransformer = phev_pipe_outputChainInputTransformer,
        .outputOutputTransformer = phev_pipe_outputEventTransformer,
    };

    phev_pipe_ctx_t * pipe = phev_pipe_createPipe(settings);

    LOG_V(APP_TAG, "END - createPipeRegister");
    return pipe;
}

bool phev_service_checkByte(uint16_t num)
{
    return (num >= 0 && num < 256);
}
uint8_t phev_service_getJsonByte(cJSON *json, char *option)
{
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, option);

    if (value != NULL)
    {
        return ((uint8_t)value->valueint) & 0xff;
    }
    return 0;
}

uint16_t phev_service_getJsonInt(cJSON *json, char *option)
{
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, option);

    if (value != NULL)
    {
        return value->valueint;
    }
    return 0;
}
bool phev_service_validateCheckOnOrOff(const char *str)
{
    if (str)
    {
        if (strcmp(str, PHEV_SERVICE_ON_JSON) == 0)
        {
            return true;
        }

        if (strcmp(str, PHEV_SERVICE_OFF_JSON) == 0)
        {
            return true;
        }
    }

    return false;
}
bool phev_service_validateCommand(const char *command)
{
    cJSON *json = cJSON_Parse(command);

    if (json == NULL)
    {
        return false;
    }

    cJSON *update = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_UPDATE_REGISTER_JSON);

    cJSON *operation = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_OPERATION_JSON);

    if (update != NULL)
    {
        cJSON *reg = cJSON_GetObjectItemCaseSensitive(update, PHEV_SERVICE_UPDATE_REGISTER_REG_JSON);

        cJSON *value = cJSON_GetObjectItemCaseSensitive(update, PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON);

        if (value != NULL && reg != NULL)
        {
            return phev_service_checkByte(reg->valueint) && phev_service_checkByte(value->valueint);
        }
    }

    if (operation != NULL)
    {
        cJSON *headLights = cJSON_GetObjectItemCaseSensitive(operation, PHEV_SERVICE_OPERATION_HEADLIGHTS_JSON);

        if (headLights)
        {
            return phev_service_validateCheckOnOrOff(headLights->valuestring);
        }
        cJSON *airCon = cJSON_GetObjectItemCaseSensitive(operation, PHEV_SERVICE_OPERATION_AIRCON_JSON);

        if (airCon)
        {
            return phev_service_validateCheckOnOrOff(airCon->valuestring);
        }
    }

    return false;
}
phevMessage_t *phev_service_operationHandler(cJSON *operation)
{
    cJSON *headLights = cJSON_GetObjectItemCaseSensitive(operation, PHEV_SERVICE_OPERATION_HEADLIGHTS_JSON);

    if (headLights)
    {
        if (strcmp(headLights->valuestring, PHEV_SERVICE_ON_JSON) == 0)
        {
            return phev_core_simpleRequestCommandMessage(KO_WF_H_LAMP_CONT_SP, 1);
        }
        if (strcmp(headLights->valuestring, PHEV_SERVICE_OFF_JSON) == 0)
        {
            return phev_core_simpleRequestCommandMessage(KO_WF_H_LAMP_CONT_SP, 2);
        }
        return NULL;
    }

    cJSON *airCon = cJSON_GetObjectItemCaseSensitive(operation, PHEV_SERVICE_OPERATION_AIRCON_JSON);

    if (airCon)
    {
        if (strcmp(airCon->valuestring, PHEV_SERVICE_ON_JSON) == 0)
        {
            return phev_core_simpleRequestCommandMessage(KO_WF_MANUAL_AC_ON_RQ_SP, 2);
        }
        if (strcmp(airCon->valuestring, PHEV_SERVICE_OFF_JSON) == 0)
        {
            return phev_core_simpleRequestCommandMessage(KO_WF_MANUAL_AC_ON_RQ_SP, 1);
        }
        return NULL;
    }
    return NULL;
}
phevMessage_t *phev_service_jsonCommandToPhevMessage(const char *command)
{
    if (phev_service_validateCommand(command))
    {
        cJSON *json = cJSON_Parse(command);

        cJSON *update = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_UPDATE_REGISTER_JSON);

        cJSON *operation = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_OPERATION_JSON);

        if (update)
        {
            return phev_core_simpleRequestCommandMessage(phev_service_getJsonByte(update, PHEV_SERVICE_UPDATE_REGISTER_REG_JSON), phev_service_getJsonByte(update, PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON));
        }
        if (operation)
        {
            return phev_service_operationHandler(operation);
        }
    }
    else
    {
        return NULL;
    }
}
message_t *phev_service_jsonInputTransformer(void *ctx, message_t *message)
{
    if (message)
    {
        return phev_core_convertToMessage(phev_service_jsonCommandToPhevMessage(message->data));
    }
    return NULL;
}

cJSON * phev_service_updatedRegister(cJSON * json, phevMessage_t * phevMessage)
{
    cJSON * updatedRegister = cJSON_CreateObject();
    if(updatedRegister == NULL) 
    {
        return NULL;
    } 
    
    cJSON_AddItemToObject(json, PHEV_SERVICE_UPDATED_REGISTER_JSON, updatedRegister);
    
    cJSON * reg = cJSON_CreateNumber(phevMessage->reg);
    if(reg == NULL) 
    {
        return NULL;
    }
    
    cJSON_AddItemToObject(updatedRegister, "register", reg);  
    
    cJSON * length = cJSON_CreateNumber(phevMessage->length);
    if(length == NULL) 
    {
        return NULL;
    }

    cJSON_AddItemToObject(updatedRegister, "length", length);  

    
    cJSON * data = cJSON_CreateArray();
    if(data == NULL) 
    {
        return NULL;
    }
    cJSON_AddItemToObject(updatedRegister, "data", data);  

    for(int i=0; i < phevMessage->length; i++)
    {
        cJSON * item = cJSON_CreateNumber(phevMessage->data[i]);
        if (item == NULL)
        {
            return NULL;
        }
        cJSON_AddItemToArray(data, item);
    }
    return json;
}
cJSON * phev_service_updateRegisterAck(cJSON * json, phevMessage_t * phevMessage)
{
    cJSON * updatedRegisterAck = cJSON_CreateObject();
    if(updatedRegisterAck == NULL) 
    {
        return NULL;
    } 
    
    cJSON_AddItemToObject(json, PHEV_SERVICE_UPDATED_REGISTER_ACK_JSON, updatedRegisterAck);
    
    cJSON * reg = cJSON_CreateNumber(phevMessage->reg);
    if(reg == NULL) 
    {
        return NULL;
    }
    
    cJSON_AddItemToObject(updatedRegisterAck, "register", reg);  
    
    return json;
}

message_t * phev_service_jsonOutputTransformer(void *ctx, message_t * message)
{
    LOG_V(APP_TAG,"START - jsonOutputTransformer");

    phevServiceCtx_t * serviceCtx = NULL;

    if(ctx != NULL)
    {
        serviceCtx = ((phev_pipe_ctx_t *) ctx)->ctx;
    
        phev_pipe_outputEventTransformer(serviceCtx->pipe, message);
    
    }
    phevMessage_t * phevMessage = malloc(sizeof(phevMessage_t));
    
    phev_core_decodeMessage(message->data,message->length, phevMessage);
    char * output;
    cJSON * out = NULL;

    if(phevMessage->command != 0x6f) 
    {
        return NULL;
    }
    
    if(serviceCtx)
    {
        phevRegister_t * reg = phev_model_getRegister(serviceCtx->model, phevMessage->reg);
    
        if(reg)
        {
            bool changed = phev_model_compareRegister(serviceCtx->model,phevMessage->reg, phevMessage->data);

            if(!changed) 
            {
                return NULL;
            }
        }
    }
    
    cJSON * response = cJSON_CreateObject();
    if(response == NULL) 
    {
        return NULL;
    } 
    
    if(phevMessage->type == REQUEST_TYPE)
    {
        out = phev_service_updatedRegister(response,phevMessage);
    } else {
        out = phev_service_updateRegisterAck(response,phevMessage);
    }
    
    if(!out) 
    {
        return NULL;
    }
    
    output = cJSON_Print(out); 

    message_t * outputMessage = msg_utils_createMsg((uint8_t *) output, strlen(output)+1);
    LOG_BUFFER_HEXDUMP(APP_TAG,outputMessage->data,outputMessage->length,LOG_DEBUG);
    cJSON_Delete(response);

    free(output);
    LOG_V(APP_TAG,"END - jsonOutputTransformer");
    
    return outputMessage;
}
int phev_service_getBatteryLevel(phevServiceCtx_t * ctx)
{
    phevRegister_t * reg = phev_model_getRegister(ctx->model, KO_WF_BATT_LEVEL_INFO_REP_EVR);

    if(reg) 
    {
        return (int) reg->data[0];
    } else {
        return -1;
    }
}
char * phev_service_statusAsJson(phevServiceCtx_t * ctx)
{

    cJSON * json = cJSON_CreateObject();
    cJSON * status = cJSON_CreateObject();
    cJSON * battery = cJSON_CreateObject();

    if(json && status && battery)
    {
        int battLevel = phev_service_getBatteryLevel(ctx);

        if(battLevel >= 0)
        {
            cJSON * level = cJSON_CreateNumber((double) battLevel);
            cJSON_AddItemToObject(battery, PHEV_SERVICE_BATTERY_LEVEL_JSON,level);
        }
        cJSON_AddItemToObject(status, PHEV_SERVICE_BATTERY_JSON,battery);
        cJSON_AddItemToObject(json, PHEV_SERVICE_STATUS_JSON,status);
        
    }

    char * out = cJSON_Print(json);

    return out;
}

void phev_service_loop(phevServiceCtx_t * ctx)
{
    phev_pipe_loop(ctx->pipe);
}

message_t * phev_service_jsonResponseAggregator(void * ctx, messageBundle_t * bundle)
{
    cJSON * out = cJSON_CreateObject();

    cJSON * responses = cJSON_CreateArray();
    
    cJSON_AddItemToObject(out, "responses", responses);

    for(int i=0;i<bundle->numMessages;i++)
    {
        cJSON * next = cJSON_Parse(bundle->messages[i]->data);

        cJSON_AddItemToArray(responses, next);
    }
    
    char * str = cJSON_Print(out);
    message_t * message = msg_utils_createMsg(str,strlen(str)+1);

    return message;
}

void phev_service_errorHandler(phevError_t * error)
{

}
void phev_service_registrationCompleteCallback(phevRegisterCtx_t * ctx)
{
    phevServiceCtx_t * serviceCtx = (phevServiceCtx_t *) ctx->ctx;
    
    if(serviceCtx->registrationCompleteCallback) 
    {
        serviceCtx->registrationCompleteCallback(ctx);
    }
    
}

phevServiceCtx_t * phev_service_resetPipeAfterRegistration(phevServiceCtx_t * ctx)
{
    phev_pipe_ctx_t * pipe = phev_service_createPipe(ctx,ctx->pipe->pipe->in,ctx->pipe->pipe->out);
    
    ctx->pipe = pipe;
    
    return ctx;
}
phevRegisterCtx_t * phev_service_register(const char * mac, phevServiceCtx_t * ctx, phevRegistrationComplete_t complete)
{
    phevRegisterSettings_t settings = {
        .pipe = ctx->pipe,
        .complete = (phevRegistrationComplete_t) phev_service_registrationCompleteCallback,
        .errorHandler = (phevErrorHandler_t) phev_service_errorHandler,
        .ctx = ctx,
    };
    
    memcpy(&settings.mac, mac, 6);
    ctx->registrationCompleteCallback = complete;
    phevRegisterCtx_t * regCtx = phev_register_init(settings);

    return regCtx;
}
