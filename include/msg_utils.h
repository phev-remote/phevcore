#ifndef _MSG_UTILS_H_
#define _MSG_UTILS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "msg_core.h"
#include "logger.h"
#ifdef __XTENSA__
#include "esp_system.h"
#endif

const static char *MSG_UTILS_APP_TAG = "MSG_UTILS";

inline static message_t * msg_utils_createMsg(const uint8_t * data, const size_t length)
{
    LOG_V(MSG_UTILS_APP_TAG,"START - createMsg");
    message_t * message = malloc(sizeof(message_t));

#ifdef __XTENSA__
    message->id = esp_random();
#else
    message->id = rand();
#endif
    message->data = malloc(length);
    if(!data)
    {
        LOG_E(MSG_UTILS_APP_TAG,"Cannot allocate memory Out of memory! Size requested %zu", length);
        return NULL;
    }
    memcpy(message->data, data, length);
    message->length = length;
    message->topic = NULL;
    message->ctx = NULL;
    LOG_D(MSG_UTILS_APP_TAG,"Created Message ID %u length %zu", message->id, message->length);
    LOG_BUFFER_HEXDUMP(MSG_UTILS_APP_TAG,message->data,message->length,LOG_DEBUG);
    LOG_V(MSG_UTILS_APP_TAG,"END - createMsg");
    return message;
}
inline static message_t * msg_utils_createMsgTopic(const char * topic, const uint8_t * data, const size_t length)
{
    LOG_V(MSG_UTILS_APP_TAG,"START - createMsgTopic");
    message_t * message = msg_utils_createMsg(data,length);
    message->topic = strdup(topic);
    LOG_V(MSG_UTILS_APP_TAG,"END - createMsgTopic");
    return message;
}
inline static message_t * msg_utils_createMsgTopicCtx(const char * topic, const uint8_t * data, const size_t length, void * ctx)
{
    LOG_V(MSG_UTILS_APP_TAG,"START - createMsgTopicCtx");
    message_t * message = msg_utils_createMsg(data,length);
    message->topic = strdup(topic);
    message->ctx = ctx;
    LOG_V(MSG_UTILS_APP_TAG,"END - createMsgTopicCtx");
    return message;
}
inline static message_t * msg_utils_createMsgCtx(const uint8_t * data, const size_t length, void * ctx)
{
    LOG_V(MSG_UTILS_APP_TAG,"START - createMsgCtx");
    message_t * message = msg_utils_createMsg(data,length);
    message->ctx = ctx;
    LOG_V(MSG_UTILS_APP_TAG,"END - createMsgCtx");
    return message;
}
inline static void msg_utils_destroyMsg(message_t * message)
{
    if(message != NULL)
    {
        LOG_D(MSG_UTILS_APP_TAG,"Destroyed Message ID %ul length %zu", message->id, message->length);
        if(message->data != NULL)
        {
            free(message->data);
            message->data = NULL;
        }
        free(message);
        message = NULL;
    }
}
inline static void msg_utils_destroyMsgBundle(messageBundle_t * messages) 
{
    LOG_V(MSG_UTILS_APP_TAG,"START - destroyMsgBundle");
    if(messages == NULL) return;
    for(int i=0;i<messages->numMessages;i++)
    {
        if(messages->messages[i] != NULL)
        {
            msg_utils_destroyMsg(messages->messages[i]);
        }
    }
    free(messages);
    messages = NULL;
    LOG_V(MSG_UTILS_APP_TAG,"END - destroyMsgBundle");
}
inline static message_t * msg_utils_copyMsg(message_t * message)
{
    LOG_V(MSG_UTILS_APP_TAG,"START - copyMsg");
    message_t * out = msg_utils_createMsg(message->data,message->length);
    out->topic = message->topic != NULL ? strdup(message->topic) : NULL;
    out->ctx = message->ctx;
    LOG_D(MSG_UTILS_APP_TAG,"Copied message %ul to new message %ul",message->id,out->id);
    LOG_V(MSG_UTILS_APP_TAG,"END - copyMsg");
    return out;
}

message_t * msg_utils_concatMessages(message_t * message1, message_t * message2);

#endif
