#ifndef _PHEV_SERVICE_H_
#define _PHEV_SERVICE_H_
#include <stdbool.h>
#include "phev_core.h"

#define PHEV_SERVICE_UPDATE_REGISTER_JSON "updateRegister"
#define PHEV_SERVICE_OPERATION_JSON "operation"
#define PHEV_SERVICE_UPDATE_REGISTER_REG_JSON "reg"
#define PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON "value"

#define PHEV_SERVICE_OPERATION_HEADLIGHTS_JSON "headLights"

#define PHEV_SERVICE_ON_JSON "on"
#define PHEV_SERVICE_OFF_JSON "off"


bool phev_service_validateCommand(const char * command);
phevMessage_t * phev_service_jsonCommandToPhevMessage(const char * command);

#endif