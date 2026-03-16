#ifndef _MSG_PIPE_SPLITTER_H_
#define _MSG_PIPE_SPLITTER_H_
#include "msg_core.h"
#include "msg_pipe.h"

messageBundle_t * msg_pipe_splitter(msg_pipe_ctx_t *ctx, msg_pipe_chain_t * chain, message_t *message);
message_t * msg_pipe_splitter_aggregrator(messageBundle_t * messages);

#endif