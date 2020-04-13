#include <stdlib.h>
#include "phev_model.h"
#include "logger.h"

const static char * TAG = "PHEV_MODEL";

phevModel_t * phev_model_create(void)
{
    LOG_V(TAG, "START - create");
    phevModel_t * model = malloc(sizeof(phevModel_t));

    for(int i=0;i<256;i++)
    {
        model->registers[i] = NULL;
    }
    LOG_I(TAG,"Model created and initialised");
    LOG_V(TAG, "END - createModel");
    return model;
}

int phev_model_setRegister(phevModel_t * model, uint8_t reg, const uint8_t * data, size_t length)
{
    LOG_V(TAG, "START - setRegister");
    phevRegister_t * out = malloc(sizeof(phevRegister_t) + length);
    out->length = length;
    memcpy(out->data,data,length);
    model->registers[reg] = out;
    LOG_V(TAG, "END - setRegister");
    return 1;
}
phevRegister_t * phev_model_getRegister(phevModel_t * model, uint8_t reg)
{
    phevRegister_t * ret = NULL;

    LOG_V(TAG, "START - getRegister");
    if(model)
    {
        phevRegister_t * out = model->registers[reg];
        if(out == NULL)
        {   
            LOG_I(TAG,"Register %d is not set",reg);
            goto phev_model_getRegister_end;
        } else {
            if(out->length == 0)
            {
                LOG_I(TAG,"Register data length is zero");
                goto phev_model_getRegister_end;
            } else {
                ret = malloc(sizeof(phevRegister_t) + out->length);
                memcpy(ret, out, sizeof(phevRegister_t) + out->length);
               goto phev_model_getRegister_end;
            }
        }
    } else {
        LOG_E(TAG,"Model is not initialised");
        return NULL;
    }
phev_model_getRegister_end:
    LOG_V(TAG, "END - getRegister");
    
    return ret;
}
int phev_model_compareRegister(phevModel_t * model, uint8_t reg , const uint8_t * data)
{
    if(model)
    {
        phevRegister_t * out = phev_model_getRegister(model,reg);
        
        
        if(out && data)
        {
            printf("%02X %02X",data[0],out->data[0]);
        
            return memcmp(data,out->data,out->length);
        }
        return -1;
    } else {
        LOG_E(TAG,"Model is not initialised");
        return -1;
    }
}