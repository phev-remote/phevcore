#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "msg_gcp_mqtt.h"
#include "msg_mqtt.h"
#include "logger.h"

const static char *APP_TAG = "MSG_GCP_MQTT";
 
int msg_gcp_start(messagingClient_t *client) 
{
    return MSG_GCP_OK;
}
int msg_gcp_stop(messagingClient_t *client)
{
    return MSG_GCP_OK;
}

void msg_gcp_asyncIncomingHandler(messagingClient_t *client, message_t *message)
{
    msg_core_call_subs(client, message);
}

void msg_gcp_connected(mqtt_event_handle_t *event)
{    
    ((msg_mqtt_t *)((mqtt_event_t *)event)->user_context)->client->connected = 1;
//    msg_mqtt_subscribe((msg_mqtt_t *)((mqtt_event_t *)event)->user_context, "/devices/my-device2/config");
    msg_mqtt_subscribe((msg_mqtt_t *)((mqtt_event_t *)event)->user_context, ((gcp_ctx_t *) ((msg_mqtt_t *)((mqtt_event_t *)event)->user_context)->client->ctx)->configTopic);
    LOG_D(APP_TAG,"Subdcribed to %s",((gcp_ctx_t *) ((msg_mqtt_t *)((mqtt_event_t *)event)->user_context)->client->ctx)->configTopic);
    msg_mqtt_subscribe((msg_mqtt_t *)((mqtt_event_t *)event)->user_context, ((gcp_ctx_t *) ((msg_mqtt_t *)((mqtt_event_t *)event)->user_context)->client->ctx)->commandsTopic);
    LOG_D(APP_TAG,"Subdcribed to %s",((gcp_ctx_t *) ((msg_mqtt_t *)((mqtt_event_t *)event)->user_context)->client->ctx)->commandsTopic);

}
void msg_gcp_disconnected(mqtt_event_handle_t *event)
{    
    ((msg_mqtt_t *)((mqtt_event_t *)event)->user_context)->client->connected = 0;
}

int msg_gcp_connect(messagingClient_t *client)
{
    LOG_V(APP_TAG,"START - connect");
    
    gcp_ctx_t * ctx = (gcp_ctx_t *) client->ctx;

    msg_mqtt_settings_t settings = {
        .host = ctx->host, 
        .port = ctx->port, 
        .uri = ctx->uri,
        .clientId = ctx->clientId, 
        .username = ctx->device, 
        .password = ctx->createJwt(ctx->projectId,NULL), 
        .mqtt = ctx->mqtt,
        .subscribed_cb = NULL,
        .connected_cb = msg_gcp_connected,
        .published_cb = ctx->published,
        .disconnected_cb = msg_gcp_disconnected,
        .incoming_cb = msg_gcp_asyncIncomingHandler,
        .client = client,
        .transport = MSG_MQTT_TRANSPORT_OVER_SSL,
    };
    
    LOG_D(APP_TAG,"Calling MQTT start");
    
    ctx->mqtt->handle = msg_mqtt_start(&settings);

    LOG_V(APP_TAG,"END - connect");
    
    return MSG_GCP_OK;
   
}
message_t * msg_gcp_incomingHandler(messagingClient_t *client)
{
    return NULL;
}
void msg_gcp_outgoingHandler(messagingClient_t *client, message_t *message)
{
    gcp_ctx_t * ctx = (gcp_ctx_t *) client->ctx;
    msg_mqtt_publish(ctx->mqtt, ctx->topic, message);
}
messagingClient_t * msg_gcp_createGcpClient(gcpSettings_t settings)
{
    LOG_V(APP_TAG,"START - createGcpClient");
    
    messagingSettings_t clientSettings;
    
    gcp_ctx_t * ctx = malloc(sizeof(gcp_ctx_t));
    //msg_mqtt_t * mqtt_ctx = malloc(sizeof(msg_mqtt_t));

    ctx->uri = strdup(settings.uri);
    ctx->device = strdup(settings.device);
    ctx->clientId = strdup(settings.clientId);
    ctx->eventTopic = strdup(settings.eventTopic);
    ctx->stateTopic = strdup(settings.stateTopic);
    ctx->commandsTopic = strdup(settings.commandsTopic);
    ctx->configTopic = strdup(settings.configTopic);
    ctx->topic = ctx->eventTopic; // default topic
    ctx->createJwt = settings.createJwt;
    ctx->projectId = strdup(settings.projectId);
    ctx->published = settings.published;
    ctx->readBuffer = malloc(GCP_CLIENT_READ_BUF_SIZE);
    
    ctx->mqtt = settings.mqtt;

    clientSettings.incomingHandler = msg_gcp_incomingHandler;
    clientSettings.outgoingHandler = msg_gcp_outgoingHandler;
    
    clientSettings.start = msg_gcp_start;
    clientSettings.stop = msg_gcp_stop;
    clientSettings.connect = msg_gcp_connect;

    clientSettings.ctx = (void *) ctx;

    LOG_V(APP_TAG,"END - createGcpClient");
    
    return msg_core_createMessagingClient(clientSettings);

} 