#ifndef _MSG_MQTT_PAHO_H_
#define _MSG_MQTT_PAHO_H_
#include "MQTTClient.h"
#include "msg_core.h"


#define DEFAULT_INCOMING_TOPIC "incoming_topic"
#define DEFAULT_OUTGOING_TOPIC "outgoing_topic"

typedef struct mqttPahoSettings_t {
    char * uri;
    char *clientId;
    char *username;
    char *password;
    char *incomingTopic;
    char *outgoingTopic;
} mqttPahoSettings_t;
typedef struct mqtt_paho_ctx_t {
    MQTTClient client;
    messagingClient_t * messagingClient;
    char * uri;
    char *clientId;
    char *username;
    char *password;
    char *incomingTopic;
    char *outgoingTopic;

    
} mqtt_paho_ctx_t;

messagingClient_t * msg_mqtt_paho_createMqttPahoClient(mqttPahoSettings_t);

int msg_mqtt_paho_start(messagingClient_t *client);
int msg_mqtt_paho_stop(messagingClient_t *client);
int msg_mqtt_paho_connect(messagingClient_t *client);
#endif
