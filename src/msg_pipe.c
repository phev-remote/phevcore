#include <stdio.h>
#include <string.h>
#include "msg_pipe.h"
#include "msg_pipe_splitter.h"
#include "msg_utils.h"
#include "logger.h"

const static char *APP_TAG = "MSG_PIPE";

void msg_pipe_loop(msg_pipe_ctx_t * ctx)
{
    //LOG_V(APP_TAG,"START - loop");
    
    ctx->in->loop(ctx->in);
    ctx->out->loop(ctx->out);

    //LOG_V(APP_TAG,"END - loop");
    
}

message_t * msg_pipe_transformChain(msg_pipe_ctx_t * ctx, messagingClient_t * client, msg_pipe_chain_t * chain, message_t * message, bool respond) 
{
    LOG_V(APP_TAG,"START - transformChain");

    if(message == NULL)
    {
        LOG_D(APP_TAG,"Tranform incoming message is NULL");
        return NULL;
    }    
    message_t * msg = message;

    if(chain->inputTransformer != NULL) 
    {
        msg = chain->inputTransformer(ctx->user_context, message);
        if(msg == NULL)
        {
            LOG_D(APP_TAG,"Not a valid input message");
            return NULL;
        }
    }
    //msg_utils_destroyMsg(message);
    if(chain->filter != NULL)
    {
        if(!chain->filter(ctx->user_context, msg))
        {
            return NULL;
        }
    }
    if(chain->responder != NULL && respond)
    {
        message_t * response = chain->responder(ctx->user_context, msg);
        if(response != NULL)
        {
            client->publish(client,response);
            LOG_D(APP_TAG,"Destroy message after publish");
            msg_utils_destroyMsg(response);
        }
    }
    message_t * out = NULL;

    if(chain->outputTransformer != NULL)
    {
        out = chain->outputTransformer(ctx->user_context, msg);
        if(out == NULL) 
        {
            LOG_D(APP_TAG,"Output transformer returned NULL");
        } 
    } else {
        out = msg_utils_copyMsg(msg);
    }
    //msg_utils_destroyMsg(msg);

    LOG_V(APP_TAG,"END - transformChain");
    
    return out;
}
message_t * msg_pipe_callTransformers(msg_pipe_ctx_t *ctx, messagingClient_t * client, msg_pipe_chain_t * chain, message_t *message)
{
    LOG_V(APP_TAG,"START - callTransformers");
    
    message_t * ret  = NULL;
    messageBundle_t * messages = NULL;
    messageBundle_t * out = NULL;

    if(chain->splitter != NULL)
    {
        messages = msg_pipe_splitter(ctx->user_context, chain, message);
 
        if(messages == NULL) return NULL;
        
        
        if(messages->numMessages == 0) 
        {
            free(messages);
            return NULL;
        }

        out = malloc(sizeof(messageBundle_t));
        
        out->numMessages = 0;
        
        LOG_D(APP_TAG,"Transform Loop - number of messages :%d ", messages->numMessages);
        bool respond = true;    
        for(int i=0;i<messages->numMessages;i++) 
        {
            if(messages->messages[i] != NULL) 
            {
                message_t * transMsg = msg_pipe_transformChain(ctx, client, chain, messages->messages[i],respond);
                
                if(chain->respondOnce) 
                {
                    respond = false;
                }

                if(transMsg != NULL) 
                {
                    LOG_D(APP_TAG,"Message %d", out->numMessages);
                    LOG_BUFFER_HEXDUMP(APP_TAG,transMsg->data,transMsg->length,LOG_DEBUG);

                    out->messages[out->numMessages++] = transMsg;
                } else {
                    LOG_D(APP_TAG,"NULL returned from transform chain");
                }
                msg_utils_destroyMsg(messages->messages[i]);
                
            } else {
                LOG_E(APP_TAG, "Not expected message %d to be NULL, numMessages is %d",i,messages->numMessages);
                return NULL;
            }

        } 
        
         
        //LOG_D(APP_TAG,"Destroy messages (messages) after transform loop");
        
        //msg_utils_destroyMsgBundle(messages);
        free(messages);
                
        LOG_D(APP_TAG,"Transform Loop finished - processed %d messages", out->numMessages);
        
        if(out->numMessages == 0)
        {
            LOG_D(APP_TAG,"Destroy out message bundle no messages");
            
            msg_utils_destroyMsgBundle(out);
            return NULL;
        }
        
        if(chain->aggregator != NULL && out != NULL)
        {
            LOG_D(APP_TAG,"Custom aggregator");
            ret = chain->aggregator(ctx->user_context,out);     
            msg_utils_destroyMsgBundle(out);
            
        } else {
            LOG_D(APP_TAG,"Default aggregator");
        
            ret = msg_pipe_splitter_aggregrator(out);
            msg_utils_destroyMsgBundle(out);
        }
        LOG_BUFFER_HEXDUMP(APP_TAG,ret->data,ret->length,LOG_DEBUG);
        
        LOG_D(APP_TAG,"Destroy messages (out) after transformChain 1");
        
        LOG_V(APP_TAG,"END - callTransformers -1");


        return ret;
    }  else {
        
        ret = msg_pipe_transformChain(ctx, client, chain, message,true);
        
        //LOG_D(APP_TAG,"Destroy message after transformChain 2");
            
        //msg_utils_destroyMsg(message);
        
        LOG_V(APP_TAG,"END - callTransformers -2");
    
        return ret;
    
    }
}
message_t * msg_pipe_callInputTransformers(msg_pipe_ctx_t *ctx, message_t *message)
{
    LOG_V(APP_TAG,"START - callInputTransformers");
    message_t * out = msg_pipe_callTransformers(ctx, ctx->in, ctx->in_chain, message);
    
    //msg_utils_destroyMsg(message);
        
    LOG_V(APP_TAG,"END - callInputTransformers");
    
    return out;
}
message_t * msg_pipe_callOutputTransformers(msg_pipe_ctx_t *ctx, message_t *message)
{
    LOG_V(APP_TAG,"START - callOutputTransformers");
    
    message_t * ret = msg_pipe_callTransformers(ctx, ctx->out, ctx->out_chain, message);
    
    //LOG_D(APP_TAG,"Destroy message after callTransformers");
            
    //msg_utils_destroyMsg(message);
        
    LOG_V(APP_TAG,"END - callOutputTransformers");
    return ret;
}
void msg_pipe_outboundPublish(msg_pipe_ctx_t * ctx, message_t * message)
{
    LOG_V(APP_TAG,"START - outboundPublish");
    messagingClient_t *outboundClient = ctx->out;
    if(outboundClient->connected == 0) 
    {
        msg_pipe_out_connect(ctx);
    }
    if(message != NULL) 
    {
        LOG_D(APP_TAG,"Calling outbound client publish %p",outboundClient->publish);
        LOG_BUFFER_HEXDUMP(APP_TAG,message->data,message->length,LOG_DEBUG);
        outboundClient->publish(outboundClient, message);
        LOG_D(APP_TAG,"Destroy message after outboundClient publish");
            
        msg_utils_destroyMsg(message);
    }
    LOG_V(APP_TAG,"END - outboundPublish");
    
}
void msg_pipe_inboundPublish(msg_pipe_ctx_t * ctx, message_t * message)
{
    LOG_V(APP_TAG,"START - inboundPublish");
    
    messagingClient_t *inboundClient = ctx->in;
    if(inboundClient->connected == 0) 
    {
        LOG_D(APP_TAG,"inboundClient not connected - connecting");
        msg_pipe_in_connect(ctx);
    } else 
    {
        LOG_D(APP_TAG,"inboundClient connected");
    }
    if(message != NULL) 
    {
        inboundClient->publish(inboundClient, message);
        LOG_D(APP_TAG,"Destroy message after inboundClient publish");
            
        msg_utils_destroyMsg(message);
    }
    LOG_V(APP_TAG,"END - inboundPublish");
    
}

void msg_pipe_inboundSubscription(messagingClient_t *client, void * params, message_t * message)
{
    LOG_V(APP_TAG,"START - inboundSubscription");
    
    //messagingClient_t *outboundClient = ((msg_pipe_ctx_t *) params)->out;
    if(message == NULL)
    {
        LOG_D(APP_TAG,"Inbound message is NULL");
        return;
    }
    if(message->length == 0) return;

    message_t * out = message;
    if(!params)
    {
        LOG_E(APP_TAG,"No pipe passed in params");
        return;
    }
    
    msg_pipe_ctx_t * pipe = (msg_pipe_ctx_t *) params;
    
    if(pipe->in_chain != NULL)
    {
          out = msg_pipe_callInputTransformers(pipe, message);
    }
    if(out != NULL)
    {
        msg_pipe_outboundPublish(pipe,out);
    }
    LOG_V(APP_TAG,"END - inboundSubscription");
    
}
void msg_pipe_outboundSubscription(messagingClient_t *client, void * params, message_t * message)
{
    LOG_V(APP_TAG,"START - outboundSubscription");
    
    if(message == NULL)
    {
        LOG_D(APP_TAG,"Outbound message is NULL");
        return;
    }
    if(message->length == 0) return;

    if(!params)
    {
        LOG_E(APP_TAG,"No pipe passed in params");
        return;
    }

    message_t * out = message;
    msg_pipe_ctx_t * pipe = (msg_pipe_ctx_t *) params;
    
    if(pipe->out_chain != NULL)
    {
        out = msg_pipe_callOutputTransformers(pipe, message);
    }
    if(out != NULL)
    {
        msg_pipe_inboundPublish(pipe, out);
    }

    //msg_utils_destroyMsg(message);
    
    LOG_V(APP_TAG,"END - outboundSubscription");
    
}

int msg_pipe_in_connect(msg_pipe_ctx_t * ctx)
{
    LOG_V(APP_TAG,"START - in connect");

    ctx->in->connected = 1;

    if(ctx->preInConnectHook != NULL)
    {
        LOG_V(APP_TAG,"Calling in pre connection hook");
        
        ctx->preInConnectHook(ctx);
    }
     

    if(ctx->in->connect == NULL) return 0;
    
    LOG_V(APP_TAG,"Calling in connect %p", ctx->in->connect);

    int ret = ctx->in->connect(ctx->in);
    if(ret < 0)
    {
        ctx->in->connected = 0;
    }

    LOG_V(APP_TAG,"END - in connect");
    return ret;
}
int msg_pipe_in_disconnect(msg_pipe_ctx_t * ctx)
{
    LOG_V(APP_TAG,"START - in disconnect");
    
    int ret = ctx->in->disconnect(ctx->in);

    ctx->in->connected = 0;

    LOG_V(APP_TAG,"END - in disconnect");

    return ret;
}
int msg_pipe_out_disconnect(msg_pipe_ctx_t * ctx)
{
    LOG_V(APP_TAG,"START - out disconnect");
    
    int ret = ctx->out->disconnect(ctx->out);

    ctx->out->connected = 0;

    LOG_V(APP_TAG,"END - out disconnect");

    return ret;
}
int msg_pipe_out_connect(msg_pipe_ctx_t * ctx)
{
    LOG_V(APP_TAG,"START - out connect");
    if(ctx->preOutConnectHook != NULL) 
    {
        LOG_V(APP_TAG,"Calling out pre connection hook");
        ctx->preOutConnectHook(ctx);
    }
    
    ctx->out->connected = 1;

    if(ctx->out->connect == NULL) return 0;
    
    LOG_V(APP_TAG,"Calling out connect %p", ctx->out->connect);

    int ret = ctx->out->connect(ctx->out);
    
    if(ret < 0)
    {
        ctx->out->connected = 0;
    } 
    LOG_V(APP_TAG,"END - out connect");
    return ret;    
}
msg_pipe_ctx_t * msg_pipe(msg_pipe_settings_t settings) 
{
    LOG_V(APP_TAG,"START pipe");
    msg_pipe_ctx_t * ctx = malloc(sizeof(msg_pipe_ctx_t));

    ctx->in = settings.in;
    ctx->out = settings.out;

    ctx->in_chain = settings.in_chain;
    ctx->out_chain = settings.out_chain;

    ctx->user_context = settings.user_context;
    
    ctx->loop = msg_pipe_loop;

    ctx->preOutConnectHook = settings.preOutConnectHook;
    ctx->preInConnectHook = settings.preInConnectHook;

    LOG_D(APP_TAG,"Calling inbound subscribe");

    LOG_D(APP_TAG,"Addr of subscribe %p", ctx->in->subscribe);
    ctx->in->subscribe(ctx->in, ctx, msg_pipe_inboundSubscription);

    LOG_D(APP_TAG,"Calling outbound subscribe");
    
    ctx->out->subscribe(ctx->out, ctx, msg_pipe_outboundSubscription);

    
    if(ctx->in->start)
    {
        LOG_D(APP_TAG,"Calling input start");
        ctx->in->start(ctx->in);
    }
    
    LOG_D(APP_TAG,"Calling input connect");
        
    msg_pipe_in_connect(ctx);

    
    if(ctx->out->start)
    {
        LOG_D(APP_TAG,"Calling output start");
        ctx->out->start(ctx->out);
    }
    
    if(!settings.lazyConnect) 
    {
        LOG_D(APP_TAG,"Calling output connect");
        msg_pipe_out_connect(ctx);
    } else {
        LOG_D(APP_TAG,"Not connecting to output as lazy connected set");
    }
    
    
    LOG_V(APP_TAG,"END - pipe");
    
    return ctx;    
}