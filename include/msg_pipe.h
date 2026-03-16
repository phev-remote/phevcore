#ifndef _MSG_PIPE_H_
#define _MSG_PIPE_H_

#include <stdlib.h>
#include <stdbool.h>
#include "msg_core.h"

#define MAX_TRANSFORMERS 16
#define MAX_MESSAGES 100

typedef struct msg_pipe_ctx_t msg_pipe_ctx_t;

typedef messageBundle_t *(*msg_pipe_splitter_t)(void *ctx, message_t *);
typedef bool (*msg_pipe_filter_t)(void *ctx, message_t *);
typedef message_t *(*msg_pipe_responder_t)(void *ctx, message_t *);
typedef message_t *(*msg_pipe_aggregator_t)(void *ctx, messageBundle_t *);
typedef message_t *(*msg_pipe_transformer_t)(void *ctx, message_t *);
typedef void (* msg_pipe_connectHook_t)(msg_pipe_ctx_t * ctx);

typedef struct msg_pipe_chain_t
{
    msg_pipe_splitter_t splitter;
    msg_pipe_transformer_t inputTransformer;
    msg_pipe_filter_t filter;
    msg_pipe_responder_t responder;
    msg_pipe_transformer_t outputTransformer;
    msg_pipe_aggregator_t aggregator;
    bool respondOnce;

} msg_pipe_chain_t;
typedef struct msg_pipe_settings_t
{
    messagingClient_t *in;
    messagingClient_t *out;

    msg_pipe_chain_t *in_chain;
    msg_pipe_chain_t *out_chain;

    msg_pipe_connectHook_t preInConnectHook;
    msg_pipe_connectHook_t preOutConnectHook;

    void *user_context;

    int lazyConnect;

} msg_pipe_settings_t;

typedef struct msg_pipe_ctx_t
{
    messagingClient_t *in;
    messagingClient_t *out;
    void (*loop)(msg_pipe_ctx_t *ctx);

    msg_pipe_chain_t *in_chain;
    msg_pipe_chain_t *out_chain;

    msg_pipe_connectHook_t preInConnectHook;
    msg_pipe_connectHook_t preOutConnectHook;

    void *user_context;
} msg_pipe_ctx_t;

msg_pipe_ctx_t *msg_pipe(msg_pipe_settings_t);

void msg_pipe_loop(msg_pipe_ctx_t *ctx);
int msg_pipe_in_connect(msg_pipe_ctx_t * ctx);
int msg_pipe_out_connect(msg_pipe_ctx_t * ctx);
int msg_pipe_in_disconnect(msg_pipe_ctx_t * ctx);
int msg_pipe_out_disconnect(msg_pipe_ctx_t * ctx);
message_t * msg_pipe_transformChain(msg_pipe_ctx_t * ctx, messagingClient_t * client, msg_pipe_chain_t * chain, message_t * message,bool respond);
message_t * msg_pipe_callOutputTransformers(msg_pipe_ctx_t *ctx, message_t *message);
void msg_pipe_outboundPublish(msg_pipe_ctx_t * ctx, message_t * message);
void msg_pipe_inboundPublish(msg_pipe_ctx_t * ctx, message_t * message);
void msg_pipe_outboundSubscription(messagingClient_t *client, void * params, message_t * message);
void msg_pipe_inboundSubscription(messagingClient_t *client, void * params, message_t * message);

#endif
