#include "unity.h"
#include "phev.h"
#include "cjson/cJSON.h"
const static uint8_t car_5e[]                  = {0x5e,0x0c,0x00,0x01,0x9c,0x8b,0x69,0x1d,0x0c,0xe9,0xad,0xa5,0x01,0x60};
const static uint8_t car_5e_response[]         = {0x5e,0x04,0x01,0x01,0x00,0xeb};


static message_t * test_phev_my18_global_in_out_message = NULL;
// 5e0c00019c8b691d0ce9ada50160
// e504010100eb

// car 2740485248484a4848db
// app bb4c4808487f

message_t * test_phev_my18_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_phev_my18_global_in_out_message) {
        message = msg_utils_copyMsg(test_phev_my18_global_in_out_message);
        test_phev_my18_global_in_out_message = NULL;
    }
    
    return message;
}
void test_phev_my18_outHandlerOut(messagingClient_t *client, message_t *message) 
{

}
void test_phev_my18_5e(void)
{
    uint8_t mac[] = {0x24,0x0d,0xc2,0xc2,0x91,0x85};

    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_inHandlerOut,
        .outgoingHandler = test_phev_outHandlerOut,
    };
    
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevSettings_t settings = {
        .host = "localhost",
        .mac = mac,
        .out = out,
    };


    phevCtx_t * handle = phev_init(settings);
    
    TEST_ASSERT_NULL(handle);

    test_phev_my18_global_in_out_message = msg_utils_createMsg(car_5e, sizeof(car_5e));
    
}
