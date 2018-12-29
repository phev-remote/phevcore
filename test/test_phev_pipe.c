#include <string.h>
#include "unity.h"

#include "msg_core.h"
#include "msg_pipe.h"
#include "msg_utils.h"
#define PHEV_CONNECT_MAX_RETRIES 2
#define PHEV_CONNECT_WAIT_TIME (10)
#include "phev_pipe.h"



static message_t * test_pipe_global_message[10];
static message_t * test_pipe_global_in_message = NULL;
static int test_pipe_global_message_idx = 0;

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

    message_t * message = msg_utils_createMsg(startMsg,sizeof(startMsg));

    msg_pipe_outboundPublish(ctx->pipe,  message);

    TEST_ASSERT_NOT_NULL(test_pipe_global_message[0]);
    TEST_ASSERT_EQUAL_MEMORY(startMsg,test_pipe_global_message[0]->data,sizeof(startMsg));
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

void test_phev_pipe_outputChainInputTransformer(void)
{
    // TODO
}
void test_phev_pipe_splitter_one_message(void)
{
    uint8_t msg_data[] = {0x6f,0x04,0x01,0x02,0x00,0xff};
    
    message_t * message = malloc(sizeof(message_t));

    message->data = msg_data;
    message->length = sizeof(msg_data);
    
    messageBundle_t * messages = phev_pipe_outputSplitter(NULL, message);

    TEST_ASSERT_EQUAL(1, messages->numMessages);
} 
void test_phev_pipe_splitter_two_messages(void)
{
    uint8_t msg_data[] = {0x6f,0x04,0x01,0x01,0x00,0xff,0x6f,0x04,0x01,0x02,0x00,0xff};
    const uint8_t msg1_data[] = {0x6f,0x04,0x01,0x01,0x00,0xff};
    const uint8_t msg2_data[] = {0x6f,0x04,0x01,0x02,0x00,0xff};
    
    message_t * message = malloc(sizeof(message_t));
    
    message->data = msg_data;
    message->length = sizeof(msg_data);
     
    messageBundle_t * messages = phev_pipe_outputSplitter(NULL, message);

    TEST_ASSERT_EQUAL(2, messages->numMessages);
    TEST_ASSERT_EQUAL(6, messages->messages[0]->length);
    TEST_ASSERT_EQUAL(6, messages->messages[1]->length);
    
    TEST_ASSERT_EQUAL_MEMORY(msg1_data, messages->messages[0]->data, 6);
    TEST_ASSERT_EQUAL_MEMORY(msg2_data, messages->messages[1]->data, 6);

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
void test_phev_pipe_createRegisterEvent(void)
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
    TEST_ASSERT_EQUAL(PHEV_PIPE_REG_UPDATE_ACK,event->event);
    TEST_ASSERT_EQUAL(0x12,((phevMessage_t *) event->data)->reg);

    TEST_ASSERT_EQUAL_MEMORY(message->data,((phevMessage_t *) event->data)->data,message->length);
}
