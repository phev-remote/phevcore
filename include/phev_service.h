#ifndef _PHEV_SERVICE_H_
#define _PHEV_SERVICE_H_
#include <stdbool.h>

#define PHEV_SERVICE_UPDATE_REGISTER_JSON "updateRegister"
#define PHEV_SERVICE_OPERATION_JSON "operation"
#define PHEV_SERVICE_UPDATE_REGISTER_REG_JSON "reg"
#define PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON "value"

bool phev_service_validateCommand(const char * command);


#endif