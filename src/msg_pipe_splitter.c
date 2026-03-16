#include "msg_utils.h"
#include "msg_pipe.h"
#include "msg_pipe_splitter.h"
#include "logger.h"

const static char *APP_TAG = "MSG_PIPE_SPLITTER";

message_t * msg_pipe_concat(messageBundle_t * messages)
{
    LOG_V(APP_TAG,"START - concat");
    
    if(messages == NULL) 
    {
        LOG_D(APP_TAG,"Message bundle is empty");
        return NULL;
    }
    
    LOG_D(APP_TAG,"Number of messages %d", messages->numMessages);
    
    if(messages->numMessages == 0)
    {
        LOG_D(APP_TAG,"No messages");
        return NULL;
    }
    size_t total = 0;
    if(messages->messages[0] == NULL) 
    {
        LOG_D(APP_TAG,"First message is empty");
        return NULL;
    }
    
    LOG_D(APP_TAG,"Message length %d",messages->messages[0]->length);
        
    uint8_t * data = malloc(messages->messages[0]->length);

    for(int i = 0;i < messages->numMessages; i++)
    {
        if(messages->messages[i] == NULL) break;
        
        LOG_BUFFER_HEXDUMP(APP_TAG,messages->messages[i]->data,messages->messages[i]->length,LOG_DEBUG);
    
        data = realloc(data, total + messages->messages[i]->length);
        memcpy(data + total, messages->messages[i]->data, messages->messages[i]->length);
        total += messages->messages[i]->length;
    }

    if(total == 0) return NULL;

    message_t * message = msg_utils_createMsg(data,total);
    
    LOG_BUFFER_HEXDUMP(APP_TAG,message->data,message->length,LOG_DEBUG);
    
    free(data);
    
    LOG_V(APP_TAG,"END - concat");
    
    return message;
}
message_t * msg_pipe_splitter_aggregrator(messageBundle_t * messages)
{
    LOG_V(APP_TAG,"START - aggregrator");
    
    message_t * out = msg_pipe_concat(messages);

    LOG_V(APP_TAG,"END - aggregrator");
    
    return out;
}
messageBundle_t * msg_pipe_splitter(msg_pipe_ctx_t *ctx, msg_pipe_chain_t * chain, message_t *message)
{
    LOG_V(APP_TAG,"START - splitter");
    
    messageBundle_t * out = chain->splitter(ctx, message);
    
    LOG_V(APP_TAG,"END - splitter");
    return out;  
}
