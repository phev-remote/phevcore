#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "msg_core.h"
#include "msg_utils.h"
#include "msg_mqtt.h"
#include "logger.h"

const static char *APP_TAG = "MSG_MQTT";

void dataEvent(mqtt_event_handle_t event)
{
    char *topic = malloc(event->topic_len + 1);

    message_t * message = msg_utils_createMsgTopic(topic,(uint8_t *) event->data,event->data_len);

    ((msg_mqtt_t *) event->user_context)->incoming_cb(((msg_mqtt_t *) event->user_context)->client, message);
    free(topic);
    
}

msg_mqtt_err_t mqtt_event_handler(mqtt_event_handle_t event)
{
    msg_mqtt_t * mqtt = (msg_mqtt_t *) event->user_context;
    switch (event->event_id) {
        case MSG_MQTT_EVENT_CONNECTED:
            if(mqtt->connected_cb) mqtt->connected_cb(event);
            break;
        case MSG_MQTT_EVENT_DISCONNECTED:
            if(mqtt->disconnected_cb) mqtt->disconnected_cb(event);
            break;
        case MSG_MQTT_EVENT_SUBSCRIBED:
            if(mqtt->subscribed_cb) mqtt->subscribed_cb(event);
            break;
        case MSG_MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MSG_MQTT_EVENT_PUBLISHED:
            if(mqtt->published_cb) mqtt->published_cb(event);
            break;
        case MSG_MQTT_EVENT_DATA:
            if(mqtt->incoming_cb) dataEvent(event);
            break;
        case MSG_MQTT_EVENT_ERROR:
            if(mqtt->error_cb) mqtt->error_cb(event);
            break;
    }
    
    return MQTT_OK;
}

int msg_mqtt_publish(msg_mqtt_t * mqtt, topic_t topic, message_t *message)
{
    LOG_V(APP_TAG,"START - publish");
        
    int ret = -1;
    if(message->topic == NULL)
    {
        LOG_D(APP_TAG,"Publish message to topic %s",topic);
        ret =  mqtt->publish((handle_t *) mqtt->handle, topic, (const char *) message->data, message->length, 0, 0);
    } else 
    {
        LOG_D(APP_TAG,"Publish message to topic %s", message->topic);
        //ret =  mqtt->publish((handle_t *) mqtt->handle, "/devices/my-device2/state", (const char *) message->data, message->length, 0, 0);
        ret =  mqtt->publish((handle_t *) mqtt->handle, message->topic, (const char *) message->data, message->length, 0, 0);
    
    }

    LOG_D(APP_TAG,"Publish return code %d", ret);

    LOG_V(APP_TAG,"END - publish");
         
    return ret;
}

void msg_mqtt_subscribe(msg_mqtt_t * mqtt, topic_t topic)
{
    mqtt->subscribe((handle_t *) mqtt->handle, topic, 0);
}
handle_t msg_mqtt_start(msg_mqtt_settings_t * settings)
{

    msg_mqtt_t * mqtt = settings->mqtt;

    const msg_mqtt_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
        .user_context = (void *) mqtt,
        .uri = settings->uri,
        .client_id = settings->clientId,
        .username = settings->username,
        .password = settings->password,
    };
    handle_t client = mqtt->init(&mqtt_cfg);
    mqtt->start(client);
    mqtt->handle = client;
    mqtt->incoming_cb = settings->incoming_cb;
    mqtt->connected_cb = settings->connected_cb;
    mqtt->disconnected_cb = settings->disconnected_cb;
    mqtt->subscribed_cb = settings->subscribed_cb;
    mqtt->error_cb = settings->error_cb;
    mqtt->published_cb = settings->published_cb;
    mqtt->client = settings->client;
    
    return client;
}