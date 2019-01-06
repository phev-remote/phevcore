#include <stdint.h>
#include "phev_service.h"
#include "cJSON.h"


bool phev_service_checkByte(uint16_t num)
{
    return (num >= 0 && num < 256);
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
        return true;
    }

    return false;
}