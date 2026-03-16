#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "msg_core.h"
#include "msg_utils.h"
#include "logger.h"

const static char * TAG = "MSG_CORE";

int msg_core_start(messagingClient_t *client)
{
    return 0;
}
int msg_core_stop(messagingClient_t *client)
{
    return 0;
}
int msg_core_connect(messagingClient_t *client)
{
    client->connected = 1;
    return 0;
}
int msg_core_disconnect(messagingClient_t *client)
{
    client->connected = 0;
    return 0;
}
int msg_core_publish(messagingClient_t *client, message_t *message)
{
    LOG_V(TAG,"START - Publish");
    client->outgoingHandler(client, message);
    LOG_V(TAG,"END - Publish");
    return 0;
}

message_t * msg_core_incomingNop(messagingClient_t *client)
{
    LOG_V(TAG,"START - incomingNop");
    LOG_V(TAG,"END - incomingNop");
    
    return NULL;
}
void msg_core_outgoingNop(messagingClient_t *client, message_t *message)
{
    LOG_V(TAG,"START - outgoingNop");
    LOG_V(TAG,"END - outgoingNop");
    
    return;
}
int msg_core_registerHandlers(messagingClient_t *client, messagingClientHandler_t incoming, messagingClientHandler_t outgoing) 
{
    if(incoming != NULL) 
    {
        LOG_D(TAG,"Setting incoming handler to %p",incoming);
        
        client->incomingHandler = incoming;
    } else {
        LOG_D(TAG,"Setting incoming handler to NOP %p",msg_core_incomingNop);
        
        client->incomingHandler = msg_core_incomingNop;
    }
    if(outgoing != NULL) 
    {
        LOG_D(TAG,"Setting ougoing handler to %p",outgoing);
        
        client->outgoingHandler = outgoing;
    } else {
        LOG_D(TAG,"Setting ougoing handler to NOP %p",msg_core_outgoingNop);
        
        client->outgoingHandler = msg_core_outgoingNop;
    }
    return 0;
}

void msg_core_call_subs(messagingClient_t *client, message_t *message)
{
    if(message && client->numSubs)
    {
        for(int i = 0;i < client->numSubs;i++)
        {
            if(client->subs[i][0]) 
            {
                client->subs[i][0](client, client->subs[i][1],message);
            }
        }
    }
}
void msg_core_loop(messagingClient_t *client) 
{
    message_t *message = client->incomingHandler(client);

    msg_core_call_subs(client, message);
    
    msg_utils_destroyMsg(message);
}
void msg_core_subscribe(messagingClient_t *client, void * params, messagingSubscriptionCallback_t callback)
{
    if(client->numSubs < MAX_SUBSCRIPTIONS) 
    {
        client->subs[client->numSubs][0] = callback;
        client->subs[client->numSubs++][1] = params;
    }
}
int msg_core_messagingClientInit(messagingClient_t **client)
{
    (*client) = (messagingClient_t *) malloc(sizeof(messagingClient_t));
    (*client)->start = msg_core_start;
    (*client)->stop = msg_core_stop;
    (*client)->connect = msg_core_connect;
    (*client)->disconnect = msg_core_disconnect;
    (*client)->loop = msg_core_loop;
    (*client)->subscribe = msg_core_subscribe;
    (*client)->publish = msg_core_publish;
    (*client)->numSubs = 0;
    return 0;
}

messagingClient_t * msg_core_createMessagingClient(messagingSettings_t settings)
{
    messagingClient_t * client;

    msg_core_messagingClientInit(&client);
    msg_core_registerHandlers(client, settings.incomingHandler,settings.outgoingHandler);

    if(settings.start) client->start = settings.start;
    if(settings.stop) client->stop = settings.stop;
    if(settings.connect) client->connect = settings.connect;
    if(settings.disconnect) client->disconnect = settings.disconnect;
    
    client->connected = 0;
    
    client->ctx = settings.ctx;

    return client;
}