#ifndef _MSG_GCP_MQTT_H_
#define _MSG_GCP_MQTT_H_
//#include "sdkconfig.h"
typedef void *QueueHandle_t;
//#ifndef TEST
//#include "mqtt_client.h"
//#endif
#include "msg_core.h"
#include "msg_mqtt.h"
#include "config.h"

#define MSG_GCP_OK 0
#define MSG_GCP_FAIL -1

#define GCP_CLIENT_READ_BUF_SIZE 2048
//#define CONFIG_MQTT_SECURITY_ON
typedef uint32_t msg_gcp_err_t;
typedef struct msg_gcp_mqtt_t msg_gcp_mqtt_t;
typedef struct msg_gcp_client_handle_t msg_gcp_client_handle_t;

typedef struct event_t event_t;

typedef event_t* event_handle_t; 
typedef char * (* msg_gcp_jwt_t)(void);
typedef struct msg_gcp_mqtt_t
{
    void * handle;
    void (* publish)(msg_gcp_mqtt_t *, uint8_t *, size_t);
    uint8_t * (* receive)(void);
} msg_gcp_mqtt_t;

typedef struct gcpSettings_t
{
    char * host;
    char * uri;
    uint16_t port;
    char * clientId;
    char * device;
    char * projectId;
    msg_mqtt_t * mqtt;
    char * topic;
    char *stateTopic;
    char *eventTopic;
    char *commandsTopic;
    char * configTopic;
    
    char * (* createJwt)(const char *,const char *);
    void (* published)(void);
    
} gcpSettings_t;

typedef struct gcp_ctx_t
{
    char * host;
    char * uri;
    char * clientId;
    uint16_t port;
    uint8_t *readBuffer;
    char * (* createJwt)(const char *,const char *);
    char * device;
    char * projectId;
    void * client;
    char * topic;
    char * stateTopic;
    char * eventTopic;
    char * commandsTopic;
    char * configTopic;
    
    void (* published)(void);
    msg_mqtt_t * mqtt;
} gcp_ctx_t;

messagingClient_t *msg_gcp_createGcpClient(gcpSettings_t);

int msg_gcp_start(messagingClient_t *client);
int msg_gcp_stop(messagingClient_t *client);
int msg_gcp_connect(messagingClient_t *client);
void msg_gcp_connected_cb(void *self, void *params);
void msg_gcp_data_cb(void *self, void *params);
#endif