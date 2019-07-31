#include "unity.h"
#include "phev.h"

typedef struct phevCtx_t {
    phevServiceCtx_t * serviceCtx;
    phevEventHandler_t eventHandler;
    void * ctx;
} phevCtx_t;

int test_phev_handler_connectCalled = 0;
message_t * test_phev_global_out_out_message;
message_t * test_phev_global_in_out_message;

void test_phev_outHandlerOut(messagingClient_t *client, message_t *message) 
{
    test_phev_global_out_out_message = msg_utils_copyMsg(message);
    return;
}

message_t * test_phev_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_phev_global_in_out_message) {
        message = msg_utils_copyMsg(test_phev_global_in_out_message);
    }
    
    return message;
}

int test_phev_handler(phevEvent_t * event)
{
    test_phev_handler_connectCalled ++;
}
void test_phev_init_returns_context(void)
{
    phevSettings_t settings = {
        .host = "localhost",
    };
    phevCtx_t * handle = phev_init(settings);

    TEST_ASSERT_NOT_NULL(handle);
}
void test_phev_calls_connect_event(void)
{
    phevSettings_t settings = {
        .host = "localhost",
        .handler = test_phev_handler,
    };
    phevCtx_t * handle = phev_init(settings);

    TEST_ASSERT_EQUAL(1,test_phev_handler_connectCalled);
}
const static uint8_t response_one[] = {0x2f,0x04,0x01,0x01,0x00,0x35};
const static uint8_t response_two[] = {0x6f,0x04,0x01,0xaa,0x00,0x1e};  
const static uint8_t response_three[] = {0x6f,0x04,0x01,0x05,0x00,0x79};
const static uint8_t request_four[] = {0x6f,0x06,0x00,0x03,0x01,0x11,0x63,0xed};
const static uint8_t request_five[] = {0x6f,0x17,0x00,0x15,0x00,0x4a,0x4d,0x41,0x58,0x44,0x47,0x47,0x32,0x57,0x47,0x5a,0x30,0x30,0x32,0x30,0x33,0x35,0x01,0x00,0xf2};
// 
void test_phev_registrationEndToEnd(void)
{
    uint8_t mac[] = {0x11,0x22,0x33,0x44,0x55,0x66};

    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_inHandlerOut,
        .outgoingHandler = test_phev_outHandlerOut,
    };
    
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevSettings_t settings = {
        .host = "localhost",
        .handler = test_phev_handler,
        .registerDevice = true,
    };

    phevCtx_t * handle = phev_init(settings);

    ((phevServiceCtx_t *) handle->serviceCtx)->pipe->pipe->out = out;

    TEST_ASSERT_NULL(handle);

}