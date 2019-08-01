#include "unity.h"
#include "phev.h"

typedef struct phevCtx_t {
    phevServiceCtx_t * serviceCtx;
    phevEventHandler_t eventHandler;
    void * ctx;
} phevCtx_t;

const static uint8_t adapter_request_one[]      = {0xf2,0x0a,0x00,0x01,0x24,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xc8,0xf6,0x04,0x00,0xaa,0x00,0xa4};
const static uint8_t adapter_request_two[]      = {0xf6,0x0a,0x00,0x05,0x13,0x07,0x1e,0x14,0x28,0x2e,0x02,0xa9};
const static uint8_t adapter_request_three[]    = {0xf9,0x04,0x00,0x00,0x00,0xfd};
const static uint8_t adapter_request_four[]     = {0xf9,0x04,0x00,0x01,0x00,0xfe};
const static uint8_t adapter_request_five[]     = {0xf9,0x04,0x00,0x02,0x00,0xff};
const static uint8_t adapter_request_six[]      = {0xf9,0x04,0x00,0x03,0x00,0x00};
const static uint8_t adapter_request_seven[]    = {0xf9,0x04,0x00,0x04,0x00,0x01};
const static uint8_t adapter_request_eight[]    = {0xf6,0x04,0x01,0x03,0x00,0xfe};
const static uint8_t adapter_request_nine[]     = {0xf9,0x04,0x00,0x05,0x00,0x02};
const static uint8_t car_response_one[]         = {0x2f,0x04,0x01,0x01,0x00,0x35,0x9f,0x04,0x01,0x00,0x06,0xaa,0x9f,0x04,0x01,0x01,0x06,0xab,0x6f,0x04,0x01,0xaa,0x00,0x1e,0x6f,0x04,0x01,0x05,0x00,0x79};
const static uint8_t car_response_two[]         = {0x9f,0x04,0x01,0x02,0x06,0xac};
const static uint8_t car_response_three[]       = {0x9f,0x04,0x01,0x03,0x06,0xad};
const static uint8_t car_response_four[]        = {0x9f,0x04,0x01,0x04,0x06,0xae,0x6f,0x06,0x00,0x03,0x01,0x11,0x63,0xed};
const static uint8_t car_response_five[]        = {0x9f,0x04,0x01,0x05,0x06,0xaf};

int test_phev_handler_connectCalled = 0;
message_t * test_phev_global_out_out_message;
message_t * test_phev_global_in_out_message;

static int message_num = 0;

void test_phev_outHandlerOut(messagingClient_t *client, message_t *message) 
{
    
    switch(message_num) {
        case 0: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_one, message->data,message->length,"First message");
            break;
        }
        case 1: {
            // date so only test first 3 bytes
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_two, message->data,3,"Date message");
            break;
        }
        case 2: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_three, message->data,message->length,"Ping 1 message");
            break;
        }
        case 3: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_four, message->data,message->length,"Ping 2 message"); // {0xf9,0x04,0x00,0x01,0x00,0xfe};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_one, sizeof(car_response_one)); //0x2f,0x04,0x01,0x01,0x00,0x35,0x9f,0x04,0x01,0x00,0x06,0xaa,0x9f,0x04,0x01,0x01,0x06,0xab,0x6f,0x04,0x01,0xaa,0x00,0x1e,0x6f,0x04,0x01,0x05,0x00,0x79
            break;
        }
        case 4: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_five, message->data,message->length,"Ping 3 message"); //{0xf9,0x04,0x00,0x02,0x00,0xff};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_two, sizeof(car_response_two)); // {0x9f,0x04,0x01,0x02,0x06,0xac};
            break;
        }
        case 5: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_six, message->data,message->length,"Ping 4 message"); // {0xf9,0x04,0x00,0x03,0x00,0x00};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_three, sizeof(car_response_three)); // {0x9f,0x04,0x01,0x03,0x06,0xad};
            break;
        }
        case 6: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_seven, message->data,message->length,"Ping 5 message"); //{0xf9,0x04,0x00,0x04,0x00,0x01};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_four, sizeof(car_response_four)); // {0x9f,0x04,0x01,0x04,0x06,0xae,0x6f,0x06,0x00,0x03,0x01,0x11,0x63,0xed};
            break;
        }
        case 7: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_eight,message->data,message->length,"Register 3 ack message"); // {0xf6,0x04,0x01,0x03,0x00,0xfe};
            break;
        }
        case 8: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_nine,message->data,message->length,"Ping 7 message"); // {0xf9,0x04,0x00,0x05,0x00,0x02};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_five, sizeof(car_response_five)); // {0x9f,0x04,0x01,0x05,0x06,0xaf};
            break;
        }
        default: TEST_FAIL_MESSAGE("case not covered");
    }
    message_num++;
    //test_phev_global_out_out_message = msg_utils_copyMsg(message);
    return;
}

message_t * test_phev_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_phev_global_in_out_message) {
        message = msg_utils_copyMsg(test_phev_global_in_out_message);
        test_phev_global_in_out_message = NULL;
    }
    
    return message;
}

int test_phev_handler(phevEvent_t * event)
{
    printf("Hello\n");
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

void test_phev_registrationEndToEnd(void)
{
    uint8_t mac[] = {0x24,0x0d,0xc2,0xc2,0x91,0x85};

    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_inHandlerOut,
        .outgoingHandler = test_phev_outHandlerOut,
    };
    
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevSettings_t settings = {
        .host = "localhost",
        .handler = test_phev_handler,
        .registerDevice = true,
        .mac = mac,
        .out = out,
    };

    test_phev_handler_connectCalled = 0;

    phevCtx_t * handle = phev_init(settings);

    TEST_ASSERT_NOT_NULL(handle);

   // ((phevServiceCtx_t *) handle->serviceCtx)->pipe->pipe->out = out;

    phev_start(handle);

}