#include <stdlib.h>
#include <stdio.h>
#include "phev.h"
#include "phev_pipe.h"
#include "phev_tcpip.h"
#include "phev_service.h"
#include "phev_register.h"

#include "msg_tcpip.h"
#include "logger.h"

const static char *TAG = "PHEV";

void * phev_getUserCtx(phevCtx_t * ctx)
{
    if(ctx) return ctx->ctx;

    return NULL;
}

int phev_pipeEventHandler(phev_pipe_ctx_t *ctx, phevPipeEvent_t *event)
{
    LOG_V(TAG,"START - pipeEventHandler");

    phevCtx_t * phevCtx = (phevCtx_t *) ((phevServiceCtx_t *) ctx->ctx)->ctx;

    if(!phevCtx->eventHandler)
    {
        return 0;
    }

    switch(event->event) {
        case PHEV_PIPE_CONNECTED: {
            phevEvent_t ev = {
                .type = PHEV_CONNECTED,
                .ctx =  phevCtx,
            };

            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_START_ACK: {
            phevEvent_t ev = {
                .type = PHEV_STARTED,
                .ctx =phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_REG_UPDATE: {
            phevEvent_t ev = {
                .type = PHEV_REGISTER_UPDATE,
                .reg = ((phevMessage_t *) event->data)->reg,
                .data = ((phevMessage_t *) event->data)->data,
                .length = ((phevMessage_t *) event->data)->length,
                .ctx =  phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_GOT_VIN:
        {
            phevVinEvent_t * vinEv = (phevVinEvent_t *) event->data;
            char * vin = malloc(19);

            strncpy(vin,vinEv->vin,18);

            phevEvent_t ev = {
                .type = PHEV_VIN,
                .data = (unsigned char *) vin,
                .length = strlen(vin),
                .ctx =  phevCtx,

            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_ECU_VERSION2:
        {
            char * version = malloc(11);

            strncpy(version,event->data,10);
            phevEvent_t ev = {
                .type = PHEV_ECU_VERSION,
                .data = (uint8_t *) version,
                .length = strlen(version),
                .ctx =  phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_DATE_INFO:
        {
            phevEvent_t ev = {
                .type = PHEV_DATE_SYNC,
                .data = event->data,
                .length = event->length,
                .ctx = phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_BB:
        {
            //LOG_I(TAG,"BB Event");
            return 0;
        }
        case PHEV_PIPE_PING_RESP:
        {
            phevEvent_t ev = {
                .type = PHEV_PING_RESPONSE,
                .data = event->data,
                .length = event->length,
                .ctx = phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
        case PHEV_PIPE_FILTERED_MESSAGE:
        {
            phevEvent_t ev = {
                .type = PHEV_FILTERED_MESSAGE,
                .data = event->data,
                .length = event->length,
                .ctx = phevCtx,
            };
            return phevCtx->eventHandler(&ev);
        }
    }


    LOG_V(TAG,"END - pipeEventHandler");

    return 0;
}
void phev_outgoingHandler(messagingClient_t *client, message_t *message)
{
    //LOG_I(APP_TAG, "Outgoing Handler data");
    //printf("%s\n",message->data);
}
message_t * phev_incomingHandler(messagingClient_t *client)
{
    return NULL;
}
messagingClient_t * phev_createIncomingMessageClient(void)
{
    LOG_V(TAG,"START - createIncomingMessageClient");

    messagingSettings_t inSettings = {
        .incomingHandler = phev_incomingHandler,
        .outgoingHandler = phev_outgoingHandler,
        .connect = NULL,
        .disconnect = NULL,
        .start = NULL,
        .stop = NULL,
    };

    messagingClient_t *in = msg_core_createMessagingClient(inSettings);

    LOG_V(TAG,"END - createIncomingMessageClient");

    return in;
}

messagingClient_t * phev_createOutgoingMessageClient(const char * host, const uint16_t port)
{
    LOG_V(TAG,"START - createOutgoingMessageClient");

    tcpIpSettings_t outSettings = {
        .connect = phev_tcpClientConnectSocket,
        .disconnect = phev_tcpClientDisconnectSocket,
        .read = phev_tcpClientRead,
        .write = phev_tcpClientWrite,
        .host = strdup(host),
        .port = port,
    };
    messagingClient_t *out = msg_tcpip_createTcpIpClient(outSettings);

    LOG_V(TAG,"END - createOutgoingMessageClient");

    return out;
}

phevCtx_t * phev_init(phevSettings_t settings)
{
    LOG_V(TAG,"START - init");

    phevCtx_t * ctx = malloc(sizeof(phevCtx_t));
    phevServiceCtx_t * srvCtx = NULL;
    phevServiceSettings_t * serviceSettings;
    messagingClient_t * in = NULL;
    messagingClient_t * out = NULL;

    if(settings.in)
    {
        LOG_D(TAG,"Using passed in incoming messaging client");

        in = settings.in;
    } else {
        LOG_D(TAG,"Using default incoming messaging client");

        in = phev_createIncomingMessageClient();
    }

    if(settings.out)
    {
        LOG_D(TAG,"Using passed in messaging client");

        out = settings.out;
    } else {
        LOG_D(TAG,"Using default outgoing messaging client");

        out = phev_createOutgoingMessageClient(settings.host,settings.port);
    }

    LOG_D(TAG,"Settings event handler %p", phev_pipeEventHandler);
    ctx->eventHandler = settings.handler;
    ctx->ctx = settings.ctx;

    phevServiceSettings_t s = {
        .in = in,
        .out = out,
        .mac = settings.mac,
        .registerDevice = settings.registerDevice,
        .eventHandler = phev_pipeEventHandler,
        .errorHandler = NULL,
        .yieldHandler = NULL,
        .my18 = settings.my18,
        .ctx = ctx,
    };
    ctx->serviceCtx = phev_service_create(s);

    LOG_V(TAG,"END - init");

    return ctx;
}

static phevCtx_t * glob_phev_ctx = NULL;

void phev_registrationComplete(phev_pipe_ctx_t * ctx)
{
    phevCtx_t * phevCtx = glob_phev_ctx;

    phevEvent_t ev = {
        .type = PHEV_REGISTRATION_COMPLETE,
    };
    phevCtx->eventHandler(&ev);

}
phevCtx_t * phev_registerDevice(phevSettings_t settings)
{
    LOG_V(TAG,"START - registerDevice");

    phevCtx_t * ctx = phev_init(settings);

    glob_phev_ctx = ctx;

    phev_service_register((const char *) settings.mac, ctx->serviceCtx, phev_registrationComplete);

    LOG_V(TAG,"END - registerDevice");

    return ctx;
}
void phev_start(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - start");
    phev_service_start(ctx->serviceCtx);
    LOG_V(TAG,"END - start");
}
void phev_exit(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - exit");

    ctx->serviceCtx->exit = true;

    LOG_V(TAG,"START - exit");

}

bool phev_running(phevCtx_t * ctx)
{
    return !ctx->serviceCtx->exit;
}

static void phev_registerUpdateCallback(phev_pipe_ctx_t *ctx, uint8_t reg, void * customCtx)
{
    phevCallBackCtx_t * cbCtx = (phevCallBackCtx_t *) customCtx;

    cbCtx->callback(cbCtx->ctx, NULL);
    free(cbCtx);
}

void phev_headLights(phevCtx_t * ctx, bool on, phevCallBack_t callback)
{
    LOG_V(TAG,"START - headLights");
    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    LOG_D(TAG,"Switching %s head lights", on ? "ON" : "OFF");
    if (callback) {
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe, KO_WF_H_LAMP_CONT_SP, (on ? 1 : 2), phev_registerUpdateCallback, cbCtx);
    } else {
        phev_pipe_updateRegister(ctx->serviceCtx->pipe, KO_WF_H_LAMP_CONT_SP, (on ? 1 : 2));
    }

    LOG_V(TAG,"END - headLights");
}

void phev_parkingLights(phevCtx_t * ctx, bool on, phevCallBack_t callback)
{
    LOG_V(TAG,"START - parkingLights");
    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    LOG_D(TAG,"Switching %s parking lights", on ? "ON" : "OFF");
    if (callback) {
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe, KO_WF_P_LAMP_CONT_SP, (on ? 1 : 2), phev_registerUpdateCallback, cbCtx);
    } else {
        phev_pipe_updateRegister(ctx->serviceCtx->pipe, KO_WF_P_LAMP_CONT_SP, (on ? 1 : 2));
    }

    LOG_V(TAG,"END - parkingLights");
}

void phev_airCon(phevCtx_t * ctx, bool on, phevCallBack_t callback)
{
    LOG_V(TAG,"START - airCon");
    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    LOG_D(TAG,"Switching %s air conditioning", on ? "ON" : "OFF");

    if (callback) {
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_MANUAL_AC_ON_RQ_SP, (on ? 2 : 1), phev_registerUpdateCallback, cbCtx);
    } else {
        phev_pipe_updateRegister(ctx->serviceCtx->pipe,KO_WF_MANUAL_AC_ON_RQ_SP, (on ? 1 : 2));
    }
    LOG_V(TAG,"END - airCon");

}

void phev_updateAll(phevCtx_t * ctx, phevCallBack_t callback)
{
    LOG_V(TAG,"START - updateAll");
    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    LOG_D(TAG,"Start Update All");

    if (callback) {
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_EV_UPDATE_SP, 3, phev_registerUpdateCallback, cbCtx);
    } else {
        phev_pipe_updateRegister(ctx->serviceCtx->pipe,KO_WF_EV_UPDATE_SP, 3);
    }
    LOG_V(TAG,"END - updateAll");

}

void phev_removeACError(phevCtx_t * ctx, phevCallBack_t callback)
{
    LOG_V(TAG,"START - remove ACError");
    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;


    if (callback) {
        phev_pipe_updateRegisterWithCallback(ctx->serviceCtx->pipe,19, 1, phev_registerUpdateCallback, cbCtx);
    } else {
        phev_pipe_updateRegister(ctx->serviceCtx->pipe,19, 1);
    }
    LOG_V(TAG,"END - remove ACError");

}


void phev_airConMY19(phevCtx_t * ctx, phevAirConMode_t mode, phevAirConTime_t time,phevCallBack_t callback)
{
    LOG_V(TAG,"START - airConMY19");

    uint8_t val = mode;
    uint8_t val0 = 1;

    switch(time)
    {
        case T10MIN:
         case 10:
         val0 = 0; break;
        case T20MIN:
        case 20:
         val0 = 1; break;
        case T30MIN:
        case 30:
         val0 = 2; break;
    }

    uint8_t data[] = {02, val, val0, 00};

    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    LOG_D(TAG,"Switching air conditioning mode %d", val);

    if (callback) {
        phev_pipe_updateComplexRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_AC_SCH_SP_MY19, data, sizeof(data), phev_registerUpdateCallback, cbCtx);
    } else {
        phev_pipe_updateComplexRegister(ctx->serviceCtx->pipe,KO_WF_AC_SCH_SP_MY19, data, sizeof(data));
    }

    LOG_V(TAG,"END - airConMY19");
}

void phev_airConMode(phevCtx_t * ctx, phevAirConMode_t mode, phevAirConTime_t time,phevCallBack_t callback)
{
    LOG_V(TAG,"START - airConMode");

    uint8_t val = mode;

    switch(time)
    {
        case T10MIN: val |= 0; break;
        case T20MIN: val |= 16; break;
        case T30MIN: val |= 32; break;
    }

    uint8_t data[] = {0, 0, 255, 255, 255, 255, val, 255, 255, 255, 255, 255, 255, 255, 255};

    phevCallBackCtx_t * cbCtx = malloc(sizeof(phevCallBackCtx_t));

    cbCtx->callback = callback;
    cbCtx->ctx = ctx;

    LOG_D(TAG,"Switching air conditioning mode %d", val);

    if (callback) {
        phev_pipe_updateComplexRegisterWithCallback(ctx->serviceCtx->pipe,KO_WF_AC_SCH_SP, data, sizeof(data), phev_registerUpdateCallback, cbCtx);
    } else {
        phev_pipe_updateComplexRegister(ctx->serviceCtx->pipe,KO_WF_AC_SCH_SP, data, sizeof(data));
    }

    LOG_V(TAG,"END - airConMode");
}

int phev_batteryLevel(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - batteryLevel");

    int level = phev_service_getBatteryLevel(ctx->serviceCtx);

    LOG_V(TAG,"END - batteryLevel");
    return level;
}
int phev_batteryWarning(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - batteryWarning");

    int warningMessage = phev_service_getBatteryWarning(ctx->serviceCtx);
    /*
    0 - NONE
    1 - topbattwarminginfo_plugin_1 - The vehicle can't run due to a low drive battery temperature. The drive battery is now warming up, please wait.
    2 - topbattwarminginfo_plugin_2 - The vehicle can't run due to a low drive battery temperature. Heating the drive battery is required in order to run, but conditions for heating have not been met. Heating of the drive battery will start when the outside temperature rises and the vehicle is plugged-in, please wait.
    3 - topbattwarminginfo_warm_1 - The vehicle can now run because the temperature of the drive battery rose.
    4 - topbattwarminginfo_warm_2 - The vehicle can't run due to a low drive battery temperature. The drive battery is now warming up, please wait.
    5 - topbattwarminginfo_stop_1 - There is a possibility that the low temperature of the drive battery is causing it not to run. To heat the drive battery, please put the vehicle in the READY state or plug-in the charger.
    6 - topbattwarminginfo - Please insert the charge cable to prevent low drive battery temp. Warm-up will start upon insertion.
    */
    LOG_V(TAG,"END - batteryWarning");
    return warningMessage;
}

bool phev_isACError(phevCtx_t * ctx)
{

    LOG_V(TAG,"START - errACinfo");

    int error = phev_service_getACError(ctx->serviceCtx);
    LOG_V(TAG,"END - errACinfo");
    return error == 3; // errACinfo. True if Door is Open or Main battery level is Low.
}


int phev_isLocked(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - batteryLevel");

    int isLocked = phev_service_doorIsLocked(ctx->serviceCtx);

    LOG_V(TAG,"END - batteryLevel");
    if (isLocked == 1){
      return 1; // locked
    } else if (isLocked == 2){
       return 0; //unlocked
    } else{
      return -1; //unknown
    }
}


int phev_chargingStatus(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - Charging Status");

    int level = phev_service_getChargingStatus(ctx->serviceCtx);

    LOG_V(TAG,"END - Charging Status");
    return level;
}

int phev_remainingChargeTime(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - Charging Status");

    int level = phev_service_getRemainingChargeTime(ctx->serviceCtx);

    LOG_V(TAG,"END - Charging Status");
    return level;
}

phevServiceHVAC_t* phev_HVACStatus(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - HVAC Status");

    phevServiceHVAC_t * ph = phev_service_getHVACStatus(ctx->serviceCtx);

    LOG_V(TAG,"END - HVAC Status");
    return ph;
}

phevData_t * phev_getRegister(phevCtx_t * ctx, uint8_t reg)
{
    return (phevData_t *) phev_service_getRegister(ctx->serviceCtx, reg);
}

char * phev_statusAsJson(phevCtx_t * ctx)
{
    return phev_service_statusAsJson(ctx->serviceCtx);
}

void phev_disconnectCar(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - disconnectCar");
    phev_service_disconnectOutput(ctx->serviceCtx);
    LOG_V(TAG,"END - disconnectCar");
}

void phev_disconnect(phevCtx_t * ctx)
{
    LOG_V(TAG,"START - disconnect");
    phev_service_disconnect(ctx->serviceCtx);
    LOG_V(TAG,"END - disconnect");
}
