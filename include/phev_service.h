#ifndef _PHEV_SERVICE_H_
#define _PHEV_SERVICE_H_
#include <stdbool.h>
#include "phev_core.h"
#include "phev_pipe.h"
#include "phev_model.h"
#include "cJSON.h"

#define PHEV_SERVICE_UPDATE_REGISTER_JSON "updateRegister"
#define PHEV_SERVICE_OPERATION_JSON "operation"
#define PHEV_SERVICE_UPDATE_REGISTER_REG_JSON "reg"
#define PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON "value"

#define PHEV_SERVICE_OPERATION_HEADLIGHTS_JSON "headLights"
#define PHEV_SERVICE_OPERATION_AIRCON_JSON "airCon"

#define PHEV_SERVICE_UPDATED_REGISTER_JSON "updatedRegister"
#define PHEV_SERVICE_UPDATED_REGISTER_ACK_JSON "updateRegisterAck"

#define PHEV_SERVICE_ON_JSON "on"
#define PHEV_SERVICE_OFF_JSON "off"

#define PHEV_SERVICE_STATUS_JSON "status"
#define PHEV_SERVICE_BATTERY_JSON "battery"
#define PHEV_SERVICE_BATTERY_LEVEL_JSON "level"


typedef struct phevServiceCtx_t {
    phevModel_t * model;
    phev_pipe_ctx_t * pipe;
} phevServiceCtx_t;

phevServiceCtx_t * phev_service_init(messagingClient_t *in, messagingClient_t *out);
bool phev_service_validateCommand(const char * command);
phevMessage_t * phev_service_jsonCommandToPhevMessage(const char * command);
phev_pipe_ctx_t * phev_service_createPipe(messagingClient_t * in, messagingClient_t * out);
message_t * phev_service_jsonInputTransformer(void *, message_t *);
message_t * phev_service_jsonOutputTransformer(void *, message_t *);
int phev_service_getBatteryLevel(phevServiceCtx_t * ctx);
char * phev_service_statusAsJson(phevServiceCtx_t * ctx);
bool phev_service_outputFilter(void *ctx, message_t * message);
messageBundle_t * phev_service_inputSplitter(void * ctx, message_t * message);
#endif