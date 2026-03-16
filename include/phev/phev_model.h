#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#ifndef _PHEV_MODEL_H_
#define _PHEV_MODEL_H_

#include <stdint.h>
#include <string.h>

typedef struct phevRegister_t
{
    size_t length;
    uint8_t data[]; 
} phevRegister_t;

typedef struct phevModel_t
{
    phevRegister_t * registers[256];

} phevModel_t;


phevModel_t * phev_model_create(void);
int phev_model_setRegister(phevModel_t *, uint8_t, const uint8_t *, size_t);
phevRegister_t * phev_model_getRegister(phevModel_t *, uint8_t);
int phev_model_compareRegister(phevModel_t *, uint8_t, const uint8_t *);
#endif