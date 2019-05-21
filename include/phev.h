#ifndef _PHEV_H_
#define _PHEV_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct phevCtx_t phevCtx_t;

typedef enum {
    PHEV_CONNECTED,
    PHEV_REGISTER_UPDATE,
    PHEV_DISCONNECTED,
} phevEventTypes_t;

typedef struct phevEvent_t {
    phevEventTypes_t type;
} phevEvent_t;

typedef void (* phevEventHandler_t)(phevEvent_t *);

typedef struct phevSettings_t {
    char * host;
    uint16_t port;
    bool registerDevice;
    uint8_t * mac;
    phevEventHandler_t handler;
    void * ctx;
} phevSettings_t;

phevCtx_t * phev_init(phevSettings_t settings);
void phev_updateRegister(uint8_t reg, uint8_t * data, size_t length);

#endif