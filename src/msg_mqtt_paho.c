#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef MQTT_PAHO
#include "MQTTClient.h"
#include "msg_mqtt_paho.h"
#include "msg_core.h"
#include "msg_utils.h"
#include "msg_mqtt.h"
#include "logger.h"

const static char *APP_TAG = "MSG_MQTT_PAHO";

message_t * msg_mqtt_paho_incomingHandler(messagingClient_t *client)
{
    return NULL;
}

void msg_mqtt_paho_asyncIncomingHandler(messagingClient_t *client, message_t *message)
{
    msg_core_call_subs(client, message);
}
void msg_mqtt_paho_outgoingHandler(messagingClient_t *client, message_t *message)
{
    LOG_V(APP_TAG,"START - outgoingHandler");
    mqtt_paho_ctx_t * ctx = (mqtt_paho_ctx_t *) client->ctx;
    
    int ret = client->publish(client,message);
    LOG_V(APP_TAG,"END - outgoingHandler");
    //return ret;
}
int msg_mqtt_paho_publish(messagingClient_t *client, message_t *message)
{
    
    LOG_V(APP_TAG,"START - publish");
    mqtt_paho_ctx_t * ctx = (mqtt_paho_ctx_t *) client->ctx;
        
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
    
    pubmsg.payload = message->data;
    pubmsg.payloadlen = message->length;
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    char * topic;

    if(message->topic)
    {
        topic = message->topic;
    } else {
        topic = ctx->outgoingTopic;
    }
    MQTTClient_publishMessage(ctx->client, topic, &pubmsg, &token);
    LOG_D(APP_TAG,"Waiting for up to %d seconds \n"
            "on topic %s\n",
            (int)(10000L/1000), topic);
    rc = MQTTClient_waitForCompletion(ctx->client, token, 10000L);
    LOG_D(APP_TAG,"Message with delivery token %d delivered\n", token);
    //MQTTClient_disconnect(client, 10000);
    //MQTTClient_destroy(&client);
    LOG_D(APP_TAG,"Publish return code %d", rc);

    LOG_V(APP_TAG,"END - publish");
     
    return rc;
}
/*
void msg_mqtt_paho_subscribe(msg_mqtt_paho_t * mqtt, topic_t topic)
{
    mqtt->subscribe((handle_t *) mqtt->handle, topic, 0);
} */
int msg_mqtt_paho_start(messagingClient_t * client)
{

    return 0;
}
int msg_mqtt_paho_stop(messagingClient_t * client)
{

    return 0;
}
int msg_mqtt_paho_connect(messagingClient_t *client)
{
    int rc = 0;
    mqtt_paho_ctx_t * ctx = (mqtt_paho_ctx_t *) client->ctx;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    if ((rc = MQTTClient_connect(ctx->client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        LOG_E(APP_TAG,"Failed to connect, return code %d\n", rc);
        return -1;
    }

    MQTTClient_subscribe(ctx->client, ctx->incomingTopic, 1);

    return 0;
}

void msg_mqtt_paho_connlost(void *ctx, char *cause)
{
    LOG_W(APP_TAG,"Disconnected from MQTT %s",cause);
}

int msg_mqtt_paho_msgarrvd(void *ctx, char *topic, int topicLen, MQTTClient_message *message)
{
    message_t * msg = msg_utils_createMsgTopic(topic,(uint8_t *) message->payload,message->payloadlen);
    mqtt_paho_ctx_t * mqttCtx = (mqtt_paho_ctx_t *) ctx;

    msg_mqtt_paho_asyncIncomingHandler(mqttCtx->messagingClient, msg);
    return 1;
}
void msg_mqtt_paho_delivered(void *ctx, MQTTClient_deliveryToken dt)
{
    LOG_D(APP_TAG,"Message delivered to MQTT");
}
messagingClient_t * msg_mqtt_paho_createMqttPahoClient(mqttPahoSettings_t settings)
{
    messagingSettings_t clientSettings;

    mqtt_paho_ctx_t * ctx = malloc(sizeof(mqtt_paho_ctx_t)); 
 
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    int rc;
    
    MQTTClient_create(&ctx->client, settings.uri, settings.clientId,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTClient_setCallbacks(ctx->client, ctx, msg_mqtt_paho_connlost, msg_mqtt_paho_msgarrvd, msg_mqtt_paho_delivered);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    clientSettings.incomingHandler = msg_mqtt_paho_incomingHandler;
    clientSettings.outgoingHandler = msg_mqtt_paho_outgoingHandler;

    clientSettings.start = msg_mqtt_paho_start;
    clientSettings.stop = msg_mqtt_paho_stop;
    clientSettings.connect = msg_mqtt_paho_connect;

    clientSettings.ctx = (void *) ctx;
    messagingClient_t * client = msg_core_createMessagingClient(clientSettings);

    client->publish = msg_mqtt_paho_publish;
    ctx->messagingClient = client;
    if(settings.incomingTopic) 
    {
        ctx->incomingTopic = strdup(settings.incomingTopic);
    } else {
        ctx->incomingTopic = strdup(DEFAULT_INCOMING_TOPIC);
    }
    if(settings.outgoingTopic) 
    {
        ctx->outgoingTopic = strdup(settings.outgoingTopic);
    } else {
        ctx->outgoingTopic = strdup(DEFAULT_OUTGOING_TOPIC);
    }
    
    return client;
    
}
#endif