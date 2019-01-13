#include <stdlib.h>
#include "phev_model.h"


phevModel_t * phev_model_create(void)
{
    phevModel_t * model = malloc(sizeof(phevModel_t));

    for(int i=0;i<256;i++)
    {
        model->registers[i] = NULL;
    }

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
    phevRegister_t * out = model->registers[reg];
    if(out == NULL)
    {   
        return NULL;
    } else {
        if(out->length == 0)
        {
            return NULL;
        }
    }

    return out;
}
int phev_model_compareRegister(phevModel_t * model, uint8_t reg , const uint8_t * data)
{
    phevRegister_t * out = phev_model_getRegister(model,reg);
    if(out)
    {
        return memcmp(data,out->data,out->length);
    }
    return -1;
}