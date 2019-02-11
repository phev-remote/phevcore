#include <stdlib.h>
#include "phev_model.h"
#include "logger.h"

const static char * TAG = "PHEV_MODEL";

phevModel_t * phev_model_create(void)
{
    phevModel_t * model = malloc(sizeof(phevModel_t));

    for(int i=0;i<256;i++)
    {
        model->registers[i] = NULL;
    }
    LOG_I(TAG,"Model created and initialised");
    
    return model;
}

int phev_model_setRegister(phevModel_t * model, uint8_t reg, const uint8_t * data, size_t length)
{
    phevRegister_t * out = malloc(sizeof(phevRegister_t) + length);
    out->length = length;
    memcpy(out->data,data,length);
    model->registers[reg] = out;
    return 1;
}
phevRegister_t * phev_model_getRegister(phevModel_t * model, uint8_t reg)
{
    if(model)
    {
        phevRegister_t * out = model->registers[reg];
        if(out == NULL)
        {   
            LOG_I(TAG,"Register %d is not set",reg);
            return NULL;
        } else {
            if(out->length == 0)
            {
                LOG_I(TAG,"Register data length is zero");
                return NULL;
            } else {
                phevRegister_t * ret = malloc(sizeof(phevRegister_t) + out->length);
                memcpy(ret, out, sizeof(phevRegister_t) + out->length);
                return ret;
            }
        }
    } else {
        LOG_E(TAG,"Model is not initialised");
        return NULL;
    }
    return NULL;
}
int phev_model_compareRegister(phevModel_t * model, uint8_t reg , const uint8_t * data)
{
    if(model)
    {
        phevRegister_t * out = phev_model_getRegister(model,reg);
        if(out && data)
        {
            return memcmp(data,out->data,out->length);
        }
        return -1;
    } else {
        LOG_E(TAG,"Model is not initialised");
        return -1;
    }
}