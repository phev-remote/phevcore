#include <string.h>
#include "unity.h"

#include "msg_core.h"
#include "msg_pipe.h"
#include "msg_utils.h"
#ifndef PHEV_CONNECT_WAIT_TIME
#define PHEV_CONNECT_WAIT_TIME (1)
#endif
#ifndef PHEV_CONNECT_MAX_RETRIES
#define PHEV_CONNECT_MAX_RETRIES 2
#endif

#include "phev_pipe.h"


static message_t * test_pipe_global_message[10];
static message_t * test_pipe_global_in_message = NULL;
static int test_pipe_global_message_idx = 0;
static uint8_t test_phev_pipe_startMsg[] = { 0x6f,0x17,0x00,0x15,0x00,0x4a,0x4d,0x41,0x58,0x44,0x47,0x47,0x32,0x57,0x47,0x5a,0x30,0x30,0x32,0x30,0x33,0x35,0x01,0x01,0xf3 };


void test_phev_pipe_outHandlerIn(messagingClient_t *client, message_t *message) 
{
    return;
}

message_t * test_phev_pipe_inHandlerIn(messagingClient_t *client) 
{
    return NULL;
}

void test_phev_pipe_outHandlerOut(messagingClient_t *client, message_t *message) 
{
    hexdump("OUTHANDLEROUT",message->data,message->length,0);
    test_pipe_global_message[test_pipe_global_message_idx++] = msg_utils_copyMsg(message);
    return;
}

message_t * test_phev_pipe_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_pipe_global_in_message) {
        message = msg_utils_copyMsg(test_pipe_global_in_message);
    }
    
    return message;
}

message_t * test_phev_pipe_inHandlerIn_notConnnected(messagingClient_t * client)
{
    client->connected = 0;
    return NULL;
}
void test_phev_pipe_createPipe(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    TEST_ASSERT_NOT_NULL(ctx);

}
void test_phev_pipe_create(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);
    
    TEST_ASSERT_NOT_NULL(ctx);

}
void test_phev_pipe_loop(void)
{

    test_pipe_global_message_idx = 0;
    test_pipe_global_message[1] = NULL;
    test_pipe_global_in_message = NULL;
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);
    
    phev_pipe_loop(ctx);

    TEST_ASSERT_NULL(test_pipe_global_message[1]);

}
void test_phev_pipe_sendMac(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t expected[] = {0xf2,0x0a,0x00,0x01,0x24,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xc8,0xf6,0x04,0x00,0xaa,0x00,0xa4};
    
    uint8_t mac[] = {0x24,0x0d,0xc2,0xc2,0x91,0x85};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    phev_pipe_sendMac(ctx, mac);

    phev_pipe_loop(ctx);

    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));

}
/* No longer required for MY18
void test_phev_pipe_start(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t expected[] = {0xf2,0x0a,0x00,0x01,0x24,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xc8,0xf6,0x04,0x00,0xaa,0x00,0xa4};
    
    uint8_t mac[] = {0x24,0x0d,0xc2,0xc2,0x91,0x85};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    phev_pipe_start(ctx,mac);

    phev_pipe_loop(ctx);

    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));

} */
void test_phev_pipe_start_my18(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t expected[] = {0xf2,0x0a,0x00,0x01,0x24,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xc8,0xf6,0x04,0x00,0xaa,0x00,0xa4};
    
    uint8_t mac[] = {0x24,0x0d,0xc2,0xc2,0x91,0x85};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    phev_pipe_start(ctx,mac);

    phev_pipe_loop(ctx);

    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));

}
void test_phev_pipe_publish(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    message_t * message = msg_utils_createMsg(test_phev_pipe_startMsg,sizeof(test_phev_pipe_startMsg));

    phev_pipe_outboundPublish(ctx,  message);

    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(test_phev_pipe_startMsg,test_pipe_global_message[0]->data,sizeof(test_phev_pipe_startMsg));
}
void test_phev_pipe_commandResponder(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t reg[] = {0x6f,0x17,0x00,0x15,0x00,0x4a,0x4d,0x41,0x58,0x44,0x47,0x47,0x32,0x57,0x47,0x5a,0x30,0x30,0x32,0x30,0x33,0x35,0x01,0x01,0xf3};
    const uint8_t expected[] = {0xf6,0x04,0x01,0x15,0x00,0x10};
    test_pipe_global_in_message = msg_utils_createMsg(reg,sizeof(reg));
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    msg_pipe_loop(ctx->pipe);
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));
}
void test_phev_pipe_commandResponder_reg_update_odd_xor(void) //E9 1B 1E 09 1F 0E
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t reg[] = {0x0B,0x60,0x64,0x77,0x64,0xE2};
    const uint8_t expected[] = {0x92,0x60,0x65,0x77,0x64,0x6A};
    test_pipe_global_in_message = msg_utils_createMsg(reg,sizeof(reg));
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    phev_pipe_loop(ctx);
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));
}
void test_phev_pipe_commandResponder_reg_update_even_xor(void) //E9 1B 1E 09 1F 0E
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t reg[] = {0x70,0x14,0x1F,0x09,0x1D,0x98,0x21,0xD7,0x01,0x96,0x21,0x1F,0x1B};
    const uint8_t expected[] = {0xE9,0x1B,0x1E,0x09,0x1F,0x0E};
    test_pipe_global_in_message = msg_utils_createMsg(reg,sizeof(reg));
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    phev_pipe_loop(ctx);
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));
}
void test_phev_pipe_ping_even_xor(void) 
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t expected[] = {0xC3,0x34,0x30,0x27,0x30,0x3E};
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    ctx->pingXOR = 0x30;

    ctx->currentPing = 0x17;
    phev_pipe_ping(ctx);
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));
} 
void test_phev_pipe_ping_odd_xor(void) 
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    const uint8_t expected[] = {0x94,0x63,0x67,0x45,0x67,0x7E};
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    ctx->pingXOR = 0x67;
    ctx->currentPing = 0x22;
    phev_pipe_ping(ctx);
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));
} 
void test_phev_pipe_commandResponder_should_only_respond_to_commands(void)
{
    const uint8_t reg[] = {0x9f,0x04,0x01,0x10,0x06,0xba};
    
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = msg_utils_createMsg(reg,sizeof(reg));
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    msg_pipe_loop(ctx->pipe);
    TEST_ASSERT_NULL(test_pipe_global_message[0]);
}
void test_phev_pipe_commandResponder_should_encrypt_with_correct_xor(void)
{
    uint8_t input[] = { 0x62,0x09,0x0d,0x2c,0x0d,0x99 }; 
    uint8_t expected[] = { 0xfb,0x09,0x0c,0x2c,0x0d,0x11 };
    
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = msg_utils_createMsg(input,sizeof(input));
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    phev_pipe_loop(ctx);

    TEST_ASSERT_EQUAL(0x0d,ctx->currentXOR);
    
    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected,test_pipe_global_message[0]->data,sizeof(expected));
}
void test_phev_pipe_outputChainInputTransformer(void)
{

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    const uint8_t input[] = {0x6f,0x17,0x00,0x15,0x00,0x4a,0x4d,0x41,0x58,0x44,0x47,0x47,0x32,0x57,0x47,0x5a,0x30,0x30,0x32,0x30,0x33,0x35,0x01,0x01,0xf3};
    message_t * inputMessage = msg_utils_createMsg(input, sizeof(input));

    message_t * message = phev_pipe_outputChainInputTransformer(ctx,inputMessage);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_NOT_NULL(message->data);
    TEST_ASSERT_EQUAL(0x6f,message->data[0]);
    TEST_ASSERT_EQUAL(0x17,message->data[1]);
    TEST_ASSERT_EQUAL(REQUEST_TYPE,message->data[2]);
    TEST_ASSERT_EQUAL(0x15,message->data[3]);
}
void test_phev_pipe_outputChainInputTransformer_encoded(void)
{

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    uint8_t input[] = { 0x62,0x09,0x0d,0x2c,0x0d,0x99 }; 
    
    message_t * inputMessage = msg_utils_createMsg(input, sizeof(input));

    message_t * message = phev_pipe_outputChainInputTransformer(ctx,inputMessage);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_NOT_NULL(message->data);
    TEST_ASSERT_EQUAL(0x6f,message->data[0]);
    TEST_ASSERT_EQUAL(0x04,message->data[1]);
    TEST_ASSERT_EQUAL(REQUEST_TYPE,message->data[2]);
    TEST_ASSERT_EQUAL(0x21,message->data[3]);   
}
void test_phev_pipe_outputChainInputTransformer_changedXOR_command_response(void)
{

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    TEST_ASSERT_EQUAL(0,ctx->currentXOR);

    uint8_t input[] = { 0xAF,0xC4,0xC1,0xC5,0xC0,0xB9 }; 
    
    message_t * inputMessage = msg_utils_createMsg(input, sizeof(input));

    message_t * message = phev_pipe_outputChainInputTransformer(ctx,inputMessage);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(0xc0,ctx->currentXOR);
    
}
void test_phev_pipe_outputChainInputTransformer_changedXOR_command_request(void)
{

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    TEST_ASSERT_EQUAL(0,ctx->currentXOR);

    uint8_t input[] = { 0x00,0x6B,0x6F,0x7F,0x6D,0xEA }; 
    
    message_t * inputMessage = msg_utils_createMsg(input, sizeof(input));

    message_t * message = phev_pipe_outputChainInputTransformer(ctx,inputMessage);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(0x6f,ctx->currentXOR);
    
}
void test_phev_pipe_outputChainInputTransformer_changedXOR_ping_response(void)
{

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    TEST_ASSERT_EQUAL(0,ctx->currentXOR);

    uint8_t input[] = { 0x05,0x3E,0x3B,0x6B,0x3A,0xAF }; 
    uint8_t expoected[] = { };
    
    message_t * inputMessage = msg_utils_createMsg(input, sizeof(input));

    message_t * message = phev_pipe_outputChainInputTransformer(ctx,inputMessage);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(0x3a,ctx->currentXOR);
    
}
void test_phev_pipe_splitter_one_message(void)
{
    uint8_t msg_data[] = {0x3F,0x04,0x01,0x02,0x00,0x46};

        messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    message_t * message = malloc(sizeof(message_t));

    message->data = msg_data;
    message->length = sizeof(msg_data);
    
    messageBundle_t * messages = phev_pipe_outputSplitter(ctx, message);

    TEST_ASSERT_EQUAL(1, messages->numMessages);
} 
void test_phev_pipe_splitter_two_messages(void)
{

    uint8_t msg_data[] = {0x4E,0x0C,0x00,0x01,0x04,0x69,0x1D,0x04,0x61,0x94,0xF2,0x3F,0x02,0x11,0x3F,0x04,0x01,0x02,0x00,0x46};
    const uint8_t msg1_data[] = {0x4E,0x0C,0x00,0x01,0x04,0x69,0x1D,0x04,0x61,0x94,0xF2,0x3F,0x02,0x11};
    const uint8_t msg2_data[] = {0x3F,0x04,0x01,0x02,0x00,0x46};
        messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    message_t * message = malloc(sizeof(message_t));
    
    message->data = msg_data;
    message->length = sizeof(msg_data);
     
    messageBundle_t * messages = phev_pipe_outputSplitter(ctx, message);

    TEST_ASSERT_NOT_NULL(messages);
    TEST_ASSERT_EQUAL(2, messages->numMessages);
    TEST_ASSERT_EQUAL(14, messages->messages[0]->length);
    TEST_ASSERT_EQUAL(6, messages->messages[1]->length);
    
    TEST_ASSERT_EQUAL_MEMORY(msg1_data, messages->messages[0]->data, 14);
    TEST_ASSERT_EQUAL_MEMORY(msg2_data, messages->messages[1]->data, 6);

}
void test_phev_pipe_splitter_one_encoded_message(void)
{
    uint8_t msg_data[] = {0xFD,0xC6,0xC3,0xDA,0xC2,0x9E};

        messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);
    
    message_t * message = malloc(sizeof(message_t));

    message->data = msg_data;
    message->length = sizeof(msg_data);
    
    messageBundle_t * messages = phev_pipe_outputSplitter(ctx, message);

    TEST_ASSERT_EQUAL(1, messages->numMessages);
    TEST_ASSERT_EQUAL_MEMORY(msg_data,messages->messages[0]->data,sizeof(msg_data));
    TEST_ASSERT_EQUAL(0xc2, phev_core_getMessageXOR(messages->messages[0]));
} 
void test_phev_pipe_splitter_two_encoded_messages(void)
{

    uint8_t msg_data[] = {0xFD,0xC6,0xC3,0xD9,0xC2,0x9D,0xAD,0xCB,0xC2,0xE0,0xC2,0xC2,0x3D,0xBD,0x3D,0xC3,0xDA};
    const uint8_t msg1_data[] = {0xFD,0xC6,0xC3,0xD9,0xC2,0x9D};
    const uint8_t msg2_data[] = {0xAD,0xCB,0xC2,0xE0,0xC2,0xC2,0x3D,0xBD,0x3D,0xC3,0xDA};
        messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    message_t * message = malloc(sizeof(message_t));
    
    message->data = msg_data;
    message->length = sizeof(msg_data);
     
    messageBundle_t * messages = phev_pipe_outputSplitter(ctx, message);

    TEST_ASSERT_NOT_NULL(messages);
    TEST_ASSERT_EQUAL(2, messages->numMessages);
    TEST_ASSERT_EQUAL(sizeof(msg1_data), messages->messages[0]->length);
    TEST_ASSERT_EQUAL(sizeof(msg2_data), messages->messages[1]->length);
    
    TEST_ASSERT_EQUAL_MEMORY(msg1_data, messages->messages[0]->data, sizeof(msg1_data));
    TEST_ASSERT_EQUAL_MEMORY(msg2_data, messages->messages[1]->data, sizeof(msg2_data));
    TEST_ASSERT_EQUAL(0xc2, phev_core_getMessageXOR(messages->messages[0]));
    TEST_ASSERT_EQUAL(0xc2, phev_core_getMessageXOR(messages->messages[1]));

}

void test_phev_pipe_no_input_connection(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn_notConnnected,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_settings_t settings = {
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = NULL,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
    
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    
    };

    phev_pipe_ctx_t * ctx =  phev_pipe_createPipe(settings);

    msg_pipe_loop(ctx->pipe);
    TEST_ASSERT_NULL(test_pipe_global_message[0]);

}
void test_phev_pipe_waitForConnection_should_timeout(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    in->connected = 0;
    out->connected = 0;
    
    phev_pipe_waitForConnection(ctx);

    TEST_ASSERT_FALSE(ctx->connected);
}
void test_phev_pipe_waitForConnection(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;
    
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    in->connected = 1;
    out->connected = 1;
    
    phev_pipe_waitForConnection(ctx);

    TEST_ASSERT_TRUE(ctx->connected);
}
void test_phev_pipe_updateRegister(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;

    const uint8_t expected[] = {0xf6,0x04,0x00,0x10,0x01,0x0b};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    phev_pipe_updateRegister(ctx, 0x10, 1);

    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_pipe_global_message[0]->data,sizeof(expected));
    
}

static int test_phev_pipe_update_register_callback_called = 0;

static uint8_t test_phev_pipe_update_register_callback_expected_reg = 0;

void test_phev_pipe_update_register_callback(phev_pipe_ctx_t * ctx, uint8_t reg)
{
    test_phev_pipe_update_register_callback_called++;
    test_phev_pipe_update_register_callback_expected_reg = reg;
}
void test_phev_pipe_updateRegisterWithCallback(void)
{
    const static uint8_t msg[] = {0x6f,0x04,0x01,0x10,0x00,0x84}; 
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = msg_utils_createMsg(msg,sizeof(msg));

    const uint8_t expected[] = {0xf6,0x04,0x00,0x10,0x01,0x0b};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    phev_pipe_updateRegisterWithCallback(ctx, 0x10, 1,(phev_pipe_updateRegisterCallback_t) test_phev_pipe_update_register_callback,NULL);

    phev_pipe_loop(ctx);

    TEST_ASSERT_EQUAL(1,test_phev_pipe_update_register_callback_called);
    TEST_ASSERT_EQUAL(0x10,test_phev_pipe_update_register_callback_expected_reg);
    
    
}
void test_phev_pipe_updateRegisterWithCallback_encoded(void)
{
    const static uint8_t msg[] = {0x62,0x09,0x0c,0x07,0x0d,0x73}; 
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = msg_utils_createMsg(msg,sizeof(msg));
    test_phev_pipe_update_register_callback_called = 0;
    test_phev_pipe_update_register_callback_expected_reg = 0;
    
    const uint8_t expected[] = {0xfb,0x09,0x0d,0x07,0x0c,0x08};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);
    ctx->currentXOR = 0x0d;

    phev_pipe_updateRegisterWithCallback(ctx, KO_WF_H_LAMP_CONT_SP, 1,(phev_pipe_updateRegisterCallback_t) test_phev_pipe_update_register_callback,NULL);

    TEST_ASSERT_EQUAL(1,test_pipe_global_message_idx);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected,test_pipe_global_message[0]->data,sizeof(expected));
    
    phev_pipe_loop(ctx);

    TEST_ASSERT_EQUAL(1,test_phev_pipe_update_register_callback_called);
    TEST_ASSERT_EQUAL(0x0a,test_phev_pipe_update_register_callback_expected_reg);
    
    
}
int test_phev_pipe_event_handler(phev_pipe_ctx_t * ctx, phevPipeEvent_t * event)
{

}
void test_phev_pipe_registerEventHandler(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;

    const uint8_t expected[] = {0xf6,0x04,0x00,0x10,0x01,0x0b};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    TEST_ASSERT_EQUAL(0,ctx->eventHandlers);
    phev_pipe_registerEventHandler(ctx,test_phev_pipe_event_handler);
    TEST_ASSERT_EQUAL(1,ctx->eventHandlers);
    TEST_ASSERT_EQUAL(test_phev_pipe_event_handler,ctx->eventHandler[0]);
}
void test_phev_pipe_register_multiple_registerEventHandlers(void)
{
    test_pipe_global_message_idx = 0;
    test_pipe_global_message[0] = NULL;
    test_pipe_global_in_message = NULL;

    const uint8_t expected[] = {0xf6,0x04,0x00,0x10,0x01,0x0b};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);

    TEST_ASSERT_EQUAL(0,ctx->eventHandlers);
    phev_pipe_registerEventHandler(ctx,test_phev_pipe_event_handler);
    phev_pipe_registerEventHandler(ctx,test_phev_pipe_event_handler);
    TEST_ASSERT_EQUAL(2,ctx->eventHandlers);
    TEST_ASSERT_EQUAL(test_phev_pipe_event_handler,ctx->eventHandler[0]);
    TEST_ASSERT_EQUAL(test_phev_pipe_event_handler,ctx->eventHandler[1]);

}

void test_phev_pipe_createRegisterEvent_ack(void)
{
    uint8_t data[] = {0,1,2,3,4,5};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);
    phevMessage_t * message = phev_core_createMessage(0x6f,RESPONSE_TYPE,0x12,data, sizeof(data));

    phevPipeEvent_t * event = phev_pipe_createRegisterEvent(ctx,message);

    TEST_ASSERT_NOT_NULL(event);
    TEST_ASSERT_EQUAL(ctx,event->ctx);
    TEST_ASSERT_EQUAL(PHEV_PIPE_REG_UPDATE_ACK,event->event);
    TEST_ASSERT_EQUAL(0x12,((phevMessage_t *) event->data)->reg);

    TEST_ASSERT_EQUAL_MEMORY(message->data,((phevMessage_t *) event->data)->data,message->length);
}
void test_phev_pipe_createRegisterEvent_update(void)
{
    uint8_t data[] = {0,1,2,3,4,5};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);
    phevMessage_t * message = phev_core_createMessage(0x6f,REQUEST_TYPE,0x12,data, sizeof(data));

    phevPipeEvent_t * event = phev_pipe_createRegisterEvent(ctx,message);

    TEST_ASSERT_NOT_NULL(event);
    TEST_ASSERT_EQUAL(ctx,event->ctx);
    TEST_ASSERT_EQUAL(PHEV_PIPE_REG_UPDATE,event->event);
    TEST_ASSERT_EQUAL(0x12,((phevMessage_t *) event->data)->reg);

    TEST_ASSERT_EQUAL_MEMORY(message->data,((phevMessage_t *) event->data)->data,message->length);
    TEST_ASSERT_EQUAL_MEMORY(data,((phevMessage_t *) event->data)->data,sizeof(data));
} 
/*
void test_phev_pipe_default_event_handler(void)
{
    uint8_t data[] = {0,1,2,3,4,5};

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_pipe_inHandlerIn,
        .outgoingHandler = test_phev_pipe_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_pipe_inHandlerOut,
        .outgoingHandler = test_phev_pipe_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx =  phev_pipe_create(in,out);
    
    phevMessage_t * message = phev_core_createMessage(0x6f,REQUEST_TYPE,0x12,data, sizeof(data));

    phevPipeEvent_t * event = phev_pipe_createRegisterEvent(ctx,message);

    TEST_ASSERT_NOT_NULL(event);
    TEST_ASSERT_EQUAL(PHEV_PIPE_REG_UPDATE,event->event);
    TEST_ASSERT_EQUAL(0x12,((phevMessage_t *) event->data)->reg);

    TEST_ASSERT_EQUAL_MEMORY(message->data,((phevMessage_t *) event->data)->data,message->length);

} */
