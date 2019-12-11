#ifndef _PHEV_SERVICE_H_
#define _PHEV_SERVICE_H_
#include <stdbool.h>
#include "phev_core.h"
#include "phev_pipe.h"
#include "phev_model.h"
#include "phev_register.h"

#define PHEV_SERVICE_UPDATE_REGISTER_JSON "updateRegister"
#define PHEV_SERVICE_OPERATION_JSON "operation"
#define PHEV_SERVICE_UPDATE_REGISTER_REG_JSON "register"
#define PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON "value"

#define PHEV_SERVICE_OPERATION_HEADLIGHTS_JSON "headLights"
#define PHEV_SERVICE_OPERATION_AIRCON_JSON "airCon"
#define PHEV_SERVICE_OPERATION_UPDATE_JSON "update"


#define PHEV_SERVICE_UPDATED_REGISTER_JSON "updatedRegister"
#define PHEV_SERVICE_UPDATED_REGISTER_ACK_JSON "updateRegisterAck"

#define PHEV_SERVICE_ON_JSON "on"
#define PHEV_SERVICE_OFF_JSON "off"

#define PHEV_SERVICE_STATUS_JSON "status"
#define PHEV_SERVICE_BATTERY_JSON "battery"
#define PHEV_SERVICE_BATTERY_SOC_JSON "soc"

#define PHEV_SERVICE_REGISTER_JSON "register"
#define PHEV_SERVICE_REGISTER_DATA_JSON "data"


typedef struct phevServiceCtx_t phevServiceCtx_t;

typedef void (* phevServiceYieldHandler_t)(phevServiceCtx_t *);

typedef struct phevServiceSettings_t {
    messagingClient_t * in;
    messagingClient_t * out;
    uint8_t * mac;
    phevPipeEventHandler_t eventHandler;
    phevErrorHandler_t errorHandler;
    bool registerDevice;
    phevServiceYieldHandler_t yieldHandler;
    void * ctx;

} phevServiceSettings_t;

typedef struct phevServiceCtx_t {
    phevModel_t * model;
    phev_pipe_ctx_t * pipe;
    phevRegistrationComplete_t registrationCompleteCallback;
    phevServiceYieldHandler_t yieldHandler;
    uint8_t mac[6];
    bool exit;
    phevRegisterCtx_t * registrationCtx;
    void * ctx;
} phevServiceCtx_t;

phevServiceCtx_t * phev_service_create(phevServiceSettings_t settings);
void phev_service_start(phevServiceCtx_t * ctx);
phevServiceCtx_t * phev_service_init(messagingClient_t *in, messagingClient_t *out);
phevServiceCtx_t * phev_service_initForRegistration(messagingClient_t *in, messagingClient_t *out);
phevRegisterCtx_t * phev_service_register(const char * mac, phevServiceCtx_t * ctx, phevRegistrationComplete_t complete);
phevServiceCtx_t * phev_service_resetPipeAfterRegistration(phevServiceCtx_t * ctx);
bool phev_service_validateCommand(const char * command);
phevMessage_t * phev_service_jsonCommandToPhevMessage(const char * command);
phev_pipe_ctx_t * phev_service_createPipe(phevServiceCtx_t * ctx, messagingClient_t * in, messagingClient_t * out);
phev_pipe_ctx_t * phev_service_createPipeRegister(phevServiceCtx_t * ctx, messagingClient_t * in, messagingClient_t * out);
message_t * phev_service_jsonInputTransformer(void *, message_t *);
message_t * phev_service_jsonOutputTransformer(void *, message_t *);
int phev_service_getBatteryLevel(phevServiceCtx_t * ctx);
char * phev_service_statusAsJson(phevServiceCtx_t * ctx);
bool phev_service_outputFilter(void *ctx, message_t * message);
messageBundle_t * phev_service_inputSplitter(void * ctx, message_t * message);
void phev_service_loop(phevServiceCtx_t * ctx);
message_t * phev_service_jsonResponseAggregator(void * ctx, messageBundle_t * bundle);
phevRegister_t * phev_service_getRegister(const phevServiceCtx_t * ctx, const uint8_t reg);
void phev_service_setRegister(const phevServiceCtx_t * ctx, const uint8_t reg, const uint8_t * data, const size_t length);
char * phev_service_getRegisterJson(const phevServiceCtx_t * ctx, const uint8_t reg);
char * phev_service_getDateSync(const phevServiceCtx_t * ctx);
#endif