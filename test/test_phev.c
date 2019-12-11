#include "unity.h"
#include "phev.h"
#include "cjson/cJSON.h"
/*
typedef struct phevCtx_t {
    phevServiceCtx_t * serviceCtx;
    phevEventHandler_t eventHandler;
    void * ctx;
} phevCtx_t;
*/
const static uint8_t adapter_request_one[]      = {0xf2,0x0a,0x00,0x01,0x24,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xc8,0xf6,0x04,0x00,0xaa,0x00,0xa4};
const static uint8_t adapter_request_two[]      = {0xf6,0x0a,0x00,0x05,0x13,0x07,0x1e,0x14,0x28,0x2e,0x02,0xa9};
const static uint8_t adapter_request_three[]    = {0xf9,0x04,0x00,0x00,0x00,0xfd};
const static uint8_t adapter_request_four[]     = {0xf9,0x04,0x00,0x01,0x00,0xfe};
const static uint8_t adapter_request_five[]     = {0xf9,0x04,0x00,0x02,0x00,0xff};
const static uint8_t adapter_request_six[]      = {0xf9,0x04,0x00,0x03,0x00,0x00};
const static uint8_t adapter_request_seven[]    = {0xf9,0x04,0x00,0x04,0x00,0x01};
const static uint8_t adapter_ack_eight[]        = {0xf6,0x04,0x01,0x03,0x00,0xfe};
const static uint8_t adapter_request_nine[]     = {0xf9,0x04,0x00,0x05,0x00,0x02};
const static uint8_t adapter_request_ten[]      = {0xf9,0x04,0x00,0x06,0x00,0x03};
const static uint8_t adapter_ack_eleven[]       = {0xf6,0x04,0x01,0x15,0x00,0x10};
const static uint8_t adapter_request_twelve[]   = {0xf9,0x04,0x00,0x07,0x00,0x04};
const static uint8_t adpater_request_thirteen[] = {0xf9,0x04,0x00,0x08,0x00,0x05};
const static uint8_t adapter_request_fourteen[] = {0xf9,0x04,0x00,0x09,0x00,0x06};
const static uint8_t adapter_ack_fifteen[]      = {0xf6,0x04,0x01,0x2a,0x00,0x25};
const static uint8_t adapter_request_sixteen[]  = {0xf6,0x04,0x00,0x10,0x01,0x0b};
const static uint8_t adapter_ack_seventeen[]    = {0xf6,0x04,0x01,0xc0,0x00,0xbb};
const static uint8_t adapter_request_eighteen[] = {0xf9,0x04,0x00,0x0a,0x00,0x07};


const static uint8_t car_response_one[]         = {0x2f,0x04,0x01,0x01,0x00,0x35,0x9f,0x04,0x01,0x00,0x06,0xaa,0x9f,0x04,0x01,0x01,0x06,0xab,0x6f,0x04,0x01,0xaa,0x00,0x1e,0x6f,0x04,0x01,0x05,0x00,0x79};
const static uint8_t car_response_two[]         = {0x9f,0x04,0x01,0x02,0x06,0xac};
const static uint8_t car_response_three[]       = {0x9f,0x04,0x01,0x03,0x06,0xad};
const static uint8_t car_response_four[]        = {0x9f,0x04,0x01,0x04,0x06,0xae,0x6f,0x06,0x00,0x03,0x01,0x11,0x63,0xed};
const static uint8_t car_response_five[]        = {0x9f,0x04,0x01,0x05,0x06,0xaf};
const static uint8_t car_response_six[]         = {0x9f,0x04,0x01,0x06,0x06,0xb0,0x6f,0x17,0x00,0x15,0x00,0x4a,0x4d,0x41,0x58,0x44,0x47,0x47,0x32,0x57,0x47,0x5a,0x30,0x30,0x32,0x30,0x33,0x35,0x01,0x00,0xf2};
const static uint8_t car_response_seven[]       = {0x9f,0x04,0x01,0x07,0x06,0xb1};
const static uint8_t car_response_eight[]       = {0x9f,0x04,0x01,0x08,0x06,0xb2};
const static uint8_t car_response_nine[]        = {0x6f,0x04,0x00,0x2a,0x00,0x9d};
const static uint8_t car_response_ten[]         = {0x9f,0x04,0x01,0x09,0x06,0xb3,0x6f,0x10,0x00,0xc0,0x30,0x30,0x31,0x46,0x30,0x32,0x30,0x30,0x30,0x30,0x01,0x00,0x00,0x39,0x6f,0x04,0x01,0x10,0x00,0x84};

int test_phev_handler_connectCalled = 0;
message_t * test_phev_global_out_out_message;
message_t * test_phev_global_in_out_message;

static int message_num = 0;
static bool vin_event_fired = false;
static bool registration_complete_event_fired = false;

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
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_ack_eight,message->data,message->length,"Register 3 ack message"); // {0xf6,0x04,0x01,0x03,0x00,0xfe};
            break;
        }
        case 8: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_nine,message->data,message->length,"Ping 7 message"); // {0xf9,0x04,0x00,0x05,0x00,0x02};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_five, sizeof(car_response_five)); // {0x9f,0x04,0x01,0x05,0x06,0xaf};
            break;
        }
        case 9: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_ten,message->data,message->length,"Ping 8 message"); // {0xf9 04 00 06 00 03
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_six, sizeof(car_response_six)); // VIN
            break;
        }
        case 10: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_ack_eleven,message->data,message->length,"Register 15 ack message"); // {0xf6,0x04,0x01,0x15,0x00,0xfe};
            break;
        }
        case 11: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_twelve,message->data,message->length,"Ping 9 message"); // {0xf9,0x04,0x01,0x06,0x00,0xfe};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_seven, sizeof(car_response_seven));
            break;
        }
        case 12: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adpater_request_thirteen,message->data,message->length,"Ping 10 message"); // {0xf9,0x04,0x01,0x07,0x00,0xfe};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_eight, sizeof(car_response_eight));
            break;
        }
        case 13: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_fourteen,message->data,message->length,"Ping 11 message"); // {0xf9,0x04,0x01,0x07,0x00,0xfe};
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_nine, sizeof(car_response_nine));
            break;
        }
        case 14: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_ack_fifteen,message->data,message->length,"Register x2a ack message"); 
            break;
        }
        case 15: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_sixteen,message->data,message->length,"Register x10 request"); 
            test_phev_global_in_out_message = msg_utils_createMsg(car_response_ten, sizeof(car_response_ten));
            break;
        }
        case 16: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_ack_seventeen,message->data,message->length,"Register xc0 ack message"); 
            break;
        }
        case 17: {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(adapter_request_eighteen,message->data,message->length,"Ping 12 message"); 
            break;
        }
        
        default: {
            TEST_ASSERT_TRUE_MESSAGE(vin_event_fired,"VIN event fired");
            TEST_ASSERT_TRUE_MESSAGE(registration_complete_event_fired,"Registration complete event fired");   
        }
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
    switch (event->type)
    {
        case PHEV_VIN: vin_event_fired = true; break;
        case PHEV_CONNECTED: test_phev_handler_connectCalled ++; break;
        case PHEV_REGISTRATION_COMPLETE: registration_complete_event_fired = true; break;
    }
    return 0;
    
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

    phevCtx_t * handle = phev_registerDevice(settings);
    
    //phevCtx_t * handle = phev_init(settings);


    TEST_ASSERT_NOT_NULL(handle);

   // ((phevServiceCtx_t *) handle->serviceCtx)->pipe->pipe->out = out;

    phev_start(handle);

}

void test_phev_statusAsJson(void)
{
    const uint8_t data[] = {50};

    phevSettings_t settings = {
        .host = "localhost",
        .handler = test_phev_handler,
    };
    phevCtx_t * handle = phev_init(settings);
    
    phev_model_setRegister(handle->serviceCtx->model,29,data,1);

    char * str = phev_statusAsJson(handle);
    
    cJSON * json = cJSON_Parse(str);

    cJSON * status = cJSON_GetObjectItemCaseSensitive(json, "status");

    cJSON * battery = cJSON_GetObjectItemCaseSensitive(status, "battery");

    cJSON * level = cJSON_GetObjectItemCaseSensitive(battery, "soc");

    TEST_ASSERT_EQUAL(50,level->valueint);
}