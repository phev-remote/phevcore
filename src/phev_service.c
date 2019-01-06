#include <stdint.h>
#include "phev_service.h"
#include "cJSON.h"


bool phev_service_checkByte(uint16_t num)
{
    return (num >= 0 && num < 256);
}
uint8_t phev_service_getJsonByte(cJSON * json, char * option) 
{
    cJSON * value = cJSON_GetObjectItemCaseSensitive(json, option);
    
    if(value != NULL)
    {
        return ((uint8_t) value->valueint) & 0xff;
    }
    return 0;
}

uint16_t phev_service_getJsonInt(cJSON * json, char * option) 
{
    cJSON * value = cJSON_GetObjectItemCaseSensitive(json, option);
    
    if(value != NULL)
    {
        return value->valueint;
    }
    return 0;
}
bool phev_service_validateCheckOnOrOff(const char * str)
{
    if(str)
    {
        if(strcmp(str, PHEV_SERVICE_ON_JSON) == 0)
        {
            return true;
        }
        
        if(strcmp(str, PHEV_SERVICE_OFF_JSON) == 0)
        {
            return true;
        }
    }
    
    return false;
}
bool phev_service_validateCommand(const char * command)
{
    cJSON * json = cJSON_Parse(command);
    
    if(json == NULL)
    {
        return false;
    }

    cJSON * update = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_UPDATE_REGISTER_JSON);

    cJSON * operation = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_OPERATION_JSON);

    if(update != NULL)
    {
        cJSON * reg = cJSON_GetObjectItemCaseSensitive(update, PHEV_SERVICE_UPDATE_REGISTER_REG_JSON);

        cJSON * value = cJSON_GetObjectItemCaseSensitive(update, PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON);
    

        if(value != NULL && reg != NULL)
        {
            return phev_service_checkByte(reg->valueint) && phev_service_checkByte(value->valueint);
        }
        
    }

    if(operation != NULL)
    {
        cJSON * headLights = cJSON_GetObjectItemCaseSensitive(operation, PHEV_SERVICE_OPERATION_HEADLIGHTS_JSON);
        
        if(headLights)
        {
            return phev_service_validateCheckOnOrOff(headLights->valuestring);
        }
        return true;
    }

    return false;
}
phevMessage_t * phev_service_operationHandler(cJSON * operation)
{
    cJSON * headLights = cJSON_GetObjectItemCaseSensitive(operation, PHEV_SERVICE_OPERATION_HEADLIGHTS_JSON);

    if(headLights)
    {
        if(strcmp(headLights->valuestring, PHEV_SERVICE_ON_JSON) == 0)
        {
            return phev_core_simpleRequestCommandMessage(KO_WF_H_LAMP_CONT_SP, 1);
        }
        if(strcmp(headLights->valuestring, PHEV_SERVICE_OFF_JSON) == 0)
        {
            return phev_core_simpleRequestCommandMessage(KO_WF_H_LAMP_CONT_SP, 2);
        }
        return NULL;
    }
    return NULL;
}
phevMessage_t * phev_service_jsonCommandToPhevMessage(const char * command)
{
    if(phev_service_validateCommand(command))
    {
        cJSON * json = cJSON_Parse(command);
    
        cJSON * update = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_UPDATE_REGISTER_JSON);

        cJSON * operation = cJSON_GetObjectItemCaseSensitive(json, PHEV_SERVICE_OPERATION_JSON);

        if(update) 
        {
            return phev_core_simpleRequestCommandMessage(phev_service_getJsonByte(update,PHEV_SERVICE_UPDATE_REGISTER_REG_JSON)
                                                    ,phev_service_getJsonByte(update,PHEV_SERVICE_UPDATE_REGISTER_VALUE_JSON));
        }
        if(operation)
        {
            return phev_service_operationHandler(operation);
        }
    } else {
        return NULL;
    }
}