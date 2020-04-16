#include <string.h>
#include "unity.h"
#ifndef PHEV_CONNECT_WAIT_TIME
#define PHEV_CONNECT_WAIT_TIME (1)
#endif
#ifndef PHEV_CONNECT_MAX_RETRIES
#define PHEV_CONNECT_MAX_RETRIES 2
#endif

#include "phev_register.h"
#include "phev_service.h"
#include "msg_core.h"
#include "msg_pipe.h"
#include "msg_utils.h"
#include "phev_pipe.h"

static message_t * test_phev_register_messages[10];
static message_t * test_phev_register_inHandlerSend = NULL;
static int test_phev_register_index = 0;
static int test_register_start_ack = 0;
static int test_register_aa_ack = 0;
static int test_register_reg_evt = 0;
static int test_register_ecu_version2_evt = 0;
static int test_register_remote_security_prsnt_info_evt = 0;
static int test_phev_register_complete_called = 0;
static int test_phev_register_e2e_out_handler_stage = 0;
static int test_register_reg_disp_evt = 0;
static bool test_phev_register_e2e_completed = false;
static char * vin_event_vin = NULL;
static uint8_t vin_event_registrations = 0;
static int test_register_max_reg = 0;
            

static uint8_t test_phev_register_startMsg[] = { 0x6f,0x17,0x00,0x15,0x00,0x4a,0x4d,0x41,0x58,0x44,0x47,0x47,0x32,0x57,0x47,0x5a,0x30,0x30,0x32,0x30,0x33,0x35,0x01,0x01,0xf3 };
static uint8_t test_phev_register_startMsgMaxReg[] = { 0x6f,0x17,0x00,0x15,0x00,0x4a,0x4d,0x41,0x58,0x44,0x47,0x47,0x32,0x57,0x47,0x5a,0x30,0x30,0x32,0x30,0x33,0x35,0x01,0x03,0xf3 };
static uint8_t test_phev_register_startMsgResponse[] = {0x2f,0x04,0x01,0x01,0x00,0x35};
static uint8_t test_phev_register_AAMsgResponse[] = {0x6f,0x04,0x01,0xaa,0x00,0x1e};
static uint8_t test_phev_register_reg[] = {0x6f,0x04,0x00,0x2a,0x00,0x9d};
static uint8_t test_phev_register_firmware[] = {0x6f,0x10,0x00,0xc0,0x30,0x30,0x31,0x46,0x30,0x32,0x30,0x30,0x30,0x30,0x01,0x00,0x00,0x39};
static uint8_t test_phev_register_remoteSecurityInfo[] = {0x6f,0x06,0x00,0x03,0x01,0x11,0x63,0xed};
static uint8_t test_phev_register_regDisplayResponse[] = {0x6f,0x04,0x01,0x10,0x00,0x84};


void test_phev_register_complete(void)
{
    test_phev_register_complete_called ++;
}

void test_phev_register_outHandlerIn(messagingClient_t *client, message_t *message) 
{
    return;
}

message_t * test_phev_register_inHandlerIn(messagingClient_t *client) 
{
    return NULL;
}
void test_phev_register_outHandlerOut(messagingClient_t *client, message_t *message) 
{

    test_phev_register_messages[test_phev_register_index++] = msg_utils_copyMsg(message);
    return;
}

message_t * test_phev_register_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_phev_register_inHandlerSend != NULL)
    {
        message = test_phev_register_inHandlerSend;    
    }
    
    return message;
}
message_t * mock_outputInputTransformer(void *ctx, message_t *message)
{
    printf("Got message %s",message->data);
    TEST_FAIL_MESSAGE("FAILED");
    return NULL;
}

phev_pipe_ctx_t * test_phev_register_create_pipe_helper(void)
{
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_register_inHandlerIn,
        .outgoingHandler = test_phev_register_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_register_inHandlerOut,
        .outgoingHandler = test_phev_register_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevServiceCtx_t * srvCtx = malloc(sizeof(phevServiceCtx_t));

    phev_pipe_settings_t settings = {
        .ctx = srvCtx,
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = (msg_pipe_splitter_t) phev_pipe_outputSplitter,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    };

    phev_pipe_ctx_t * pipe = phev_pipe_createPipe(settings);

    return pipe;
}

void test_phev_register_bootstrap(void)
{
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .complete = NULL,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    TEST_ASSERT_NOT_NULL(ctx);
}

int test_phev_register_event_handler(phev_pipe_ctx_t * ctx, phevPipeEvent_t * event)
{
    switch(event->event) 
    {
        case PHEV_PIPE_GOT_VIN: {
            phevVinEvent_t * vinEvent = (phevVinEvent_t *) event->data;
            
            vin_event_vin = strdup(vinEvent->vin);
            vin_event_registrations = vinEvent->registrations;
            //printf("Got vin %s\n",vin);
            break;
        }
        case PHEV_PIPE_START_ACK: {
            test_register_start_ack ++;
            break;
        }
        case PHEV_PIPE_CONNECTED: {
            test_register_aa_ack ++;
            break;
        }
        case PHEV_PIPE_REGISTRATION: {
            test_register_reg_evt ++;
            break;
        }
        case PHEV_PIPE_ECU_VERSION2: {
            test_register_ecu_version2_evt ++;
            break;
        }
        case PHEV_PIPE_REMOTE_SECURTY_PRSNT_INFO: {
            test_register_remote_security_prsnt_info_evt ++;
            break;
        }
        case PHEV_PIPE_REG_DISP: {
            test_register_reg_disp_evt ++;
            break;
        }
        case PHEV_PIPE_MAX_REGISTRATIONS: {
            test_register_max_reg ++;
            break;
        }
        default : {
            printf("Unknown event %d\n",event->event);
        }
    }
}
// Outgoing tests
void test_phev_register_should_send_mac_and_aa(void) // TODO: Fixme
{
    test_phev_register_index = 0;
    test_phev_register_inHandlerSend = NULL;
    const uint8_t expected[] = {0xf2,0x0a,0x00,0x01,0x2f,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xd3,0xf6,0x04,0x00,0xaa,0x00,0xa4};
    const uint8_t mac[] = {0x2f,0x0d,0xc2,0xc2,0x91,0x85};
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) phev_register_eventHandler,
        .ctx = pipe->ctx,
    };
    
    memcpy(settings.mac,mac,MAC_ADDR_SIZE);

    phevRegisterCtx_t * ctx = phev_register_init(settings);

    phevPipeEvent_t event = {
        .event = PHEV_PIPE_GOT_VIN,
        .data = "JMAXDGG2WGZ002035",
        .length = 17,
    };

    ((phevServiceCtx_t *) pipe->ctx)->registrationCtx = ctx;

    phev_register_eventHandler(pipe ,&event);

    TEST_ASSERT_NOT_NULL(test_phev_register_messages[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_phev_register_messages[0]->data,sizeof(expected));
    test_phev_register_messages[0] = NULL;
}
void test_phev_register_should_trigger_aa_ack_event(void)
{
    test_register_aa_ack = 0;
    uint8_t aaAck[] = {0x6f,0x04,0x01,0xaa,0x00,0x1e};

    test_phev_register_inHandlerSend = msg_utils_createMsg(aaAck,sizeof(aaAck));

    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
        .ctx = pipe->ctx,
    };

    phevRegisterCtx_t * ctx = phev_register_init(settings);

    ((phevServiceCtx_t *) pipe->ctx)->registrationCtx = ctx;


    msg_pipe_loop(pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_aa_ack);
    
}

void test_phev_register_should_send_init(void)
{  
    const uint8_t expected[] = {0xf6,0x0a,0x00,0x01,0x2f,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xd3,0xf6,0x04,0x00,0xaa,0x00,0xa4};
    
    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_startMsg,sizeof(test_phev_register_startMsg));

    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) phev_register_eventHandler,
    };

    phevRegisterCtx_t * ctx = phev_register_init(settings);

    ((phevServiceCtx_t *) pipe->ctx)->registrationCtx = ctx;

    msg_pipe_loop(pipe->pipe);

    TEST_ASSERT_NOT_NULL(test_phev_register_messages[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_phev_register_messages[0]->data,sizeof(expected));
    
}
void test_phev_register_should_send_init_request(void)
{  
    const uint8_t expected[] = {0xf6,0x0a,0x00,0x01,0x2f,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xd3,0xf6,0x04,0x00,0xaa,0x00,0xa4};
    
    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_startMsg,sizeof(test_phev_register_startMsg));

    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) phev_register_eventHandler,
    };

    phevRegisterCtx_t * ctx = phev_register_init(settings);

    msg_pipe_loop(pipe->pipe);

    TEST_ASSERT_NOT_NULL(test_phev_register_messages[0]);
    TEST_ASSERT_EQUAL_MEMORY(expected,test_phev_register_messages[0]->data,sizeof(expected));
    
}  
void test_phev_register_should_call_complete_when_registered(void)
{
    uint8_t init_ack[] = {0x6f,0x04,0x01,0x10,0x00,0x84};
    test_phev_register_complete_called = 0;
    
    test_phev_register_inHandlerSend = msg_utils_createMsg(init_ack,sizeof(init_ack));

    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) phev_register_eventHandler,
        .complete = (phevRegistrationComplete_t) test_phev_register_complete,
        .ctx = pipe->ctx,
    };

    phevRegisterCtx_t * ctx = phev_register_init(settings);

    ((phevServiceCtx_t *) pipe->ctx)->registrationCtx = ctx;
    ctx->startAck = true;
    ctx->aaAck = true;
    ctx->registrationRequest = true;
    ctx->ecu = true;
    ctx->remoteSecurity = true;
    ctx->vin = strdup("1234");
    msg_pipe_loop(pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_phev_register_complete_called);
    
}

// Incoming Tests

void test_phev_register_getVin(void)
{
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_startMsg,sizeof(test_phev_register_startMsg));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL_STRING("JMAXDGG2WGZ002035",vin_event_vin);
    TEST_ASSERT_EQUAL(1,vin_event_registrations);
    test_phev_register_inHandlerSend = NULL;
}
void test_phev_register_should_error_when_too_many_registrations(void)
{
    test_register_max_reg = 0;
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_startMsgMaxReg,sizeof(test_phev_register_startMsgMaxReg));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_max_reg);
    test_phev_register_inHandlerSend = NULL;
}

void test_phev_register_should_get_start_ack(void)
{
    test_register_start_ack = 0;
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_startMsgResponse,sizeof(test_phev_register_startMsgResponse));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_start_ack);
    test_phev_register_inHandlerSend = NULL;
}
void test_phev_register_should_get_aa_ack(void)
{
    test_register_aa_ack = 0;
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_AAMsgResponse,sizeof(test_phev_register_AAMsgResponse));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_aa_ack);
    test_phev_register_inHandlerSend = NULL;
}
void test_phev_register_should_get_registration(void)
{
    test_register_reg_evt = 0;
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_reg,sizeof(test_phev_register_reg));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_reg_evt);
    test_phev_register_inHandlerSend = NULL;
}
void test_phev_register_should_get_ecu_version(void)
{
    test_register_ecu_version2_evt = 0;
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_firmware,sizeof(test_phev_register_firmware));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_ecu_version2_evt);
    test_phev_register_inHandlerSend = NULL;
}
void test_phev_register_should_get_remote_security_present(void)
{
    test_register_remote_security_prsnt_info_evt = 0;
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_remoteSecurityInfo,sizeof(test_phev_register_remoteSecurityInfo));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_remote_security_prsnt_info_evt);
    test_phev_register_inHandlerSend = NULL;
}
void test_phev_register_should_get_reg_disp(void)
{
    test_register_reg_disp_evt = 0;
    phev_pipe_ctx_t * pipe = test_phev_register_create_pipe_helper();
    
    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .eventHandler = (phevPipeEventHandler_t) test_phev_register_event_handler,
    };
    
    phevRegisterCtx_t * ctx = phev_register_init(settings);

    test_phev_register_inHandlerSend = msg_utils_createMsg(test_phev_register_regDisplayResponse,sizeof(test_phev_register_regDisplayResponse));

    msg_pipe_loop(ctx->pipe->pipe);

    TEST_ASSERT_EQUAL(1,test_register_reg_disp_evt);
    test_phev_register_inHandlerSend = NULL;
}

// E2E registration test

static int test_phev_register_e2e_out_handler_out_stage = 0;
        
void test_phev_register_outHandlerOutE2E(messagingClient_t *client, message_t *message) 
{
    printf("test_phev_register_outHandlerOutE2E\n");
    //hexdump(TAG,message->data,message->length,0);
    const uint8_t stage1[] = {0xf2,0x0a,0x00,0x01,0x2f,0x0d,0xc2,0xc2,0x91,0x85,0x00,0xd3,0xf6,0x04,0x00,0xaa,0x00,0xa4};
    const uint8_t stage2[] = {0xf6,0x04,0x01,0x15,0x00,0x10};
    const uint8_t stage3[] = {0xf6,0x04,0x01,0x2a,0x00,0x25};
    const uint8_t stage4[] = {0xf6,0x04,0x01,0xc0,0x00,0xbb};
    const uint8_t stage5[] = {0xf6,0x04,0x01,0x03,0x00,0xfe};
    const uint8_t stage6[] = {0xf6,0x04,0x00,0x10,0x01,0x0b};
    
    if(memcmp((const void *) stage1,(const void *) message->data,message->length) == 0)
    {
        printf("STAGE 1 - Start message \n");
        test_phev_register_e2e_out_handler_out_stage ++;
        return;
    }
    if(memcmp((const void *) stage2,(const void *) message->data,message->length) == 0)
    {
        printf("STAGE 2\n");

        test_phev_register_e2e_out_handler_out_stage ++;
        return;
    }
    if(memcmp((const void *) stage3,(const void *) message->data,message->length) == 0)
    {
        printf("STAGE 3\n");

        test_phev_register_e2e_out_handler_out_stage ++;
        return;
    }
    if(memcmp((const void *) stage4,(const void *) message->data,message->length) == 0)
    {
        printf("STAGE 4\n");

        test_phev_register_e2e_out_handler_out_stage ++;
        return;
    }
    if(memcmp((const void *) stage5,(const void *) message->data,message->length) == 0)
    {
        printf("STAGE 5\n");

        test_phev_register_e2e_out_handler_out_stage ++;
        return;
    }
    if(memcmp((const void *) stage6,(const void *) message->data,message->length) == 0)
    {

        printf("STAGE 6\n");
        
        test_phev_register_e2e_out_handler_out_stage ++;
        return;
    }
    
    return;
}

message_t * test_phev_register_inHandlerOutE2E(messagingClient_t *client) 
{
    printf("test_phev_register_outHandlerOutE2E - state %d\n",test_phev_register_e2e_out_handler_stage);
    
    switch(test_phev_register_e2e_out_handler_stage) 
    {
        case 0: {
            printf("Sending start message\n");
            test_phev_register_e2e_out_handler_stage ++;
            return msg_utils_createMsg(test_phev_register_startMsg,sizeof(test_phev_register_startMsg));
        }
        case 1: {
            printf("Sending start message response\n");
            test_phev_register_e2e_out_handler_stage ++;
            return msg_utils_createMsg(test_phev_register_startMsgResponse,sizeof(test_phev_register_startMsgResponse));
        }
        case 2: {
            printf("Sending AA message ack\n");
            
            test_phev_register_e2e_out_handler_stage ++;
            return msg_utils_createMsg(test_phev_register_AAMsgResponse,sizeof(test_phev_register_AAMsgResponse));
        }
        case 3: {
            printf("Sending registration message\n");
            test_phev_register_e2e_out_handler_stage ++;
            return msg_utils_createMsg(test_phev_register_reg,sizeof(test_phev_register_reg));
        }
        case 4: {
            printf("Sending firmware message\n");
            
            test_phev_register_e2e_out_handler_stage ++;
            return msg_utils_createMsg(test_phev_register_firmware,sizeof(test_phev_register_firmware));
        }
        case 5: {
            printf("Sending remote sec info message\n");
            
            test_phev_register_e2e_out_handler_stage ++;
            return msg_utils_createMsg(test_phev_register_remoteSecurityInfo,sizeof(test_phev_register_remoteSecurityInfo));
        }
        case 6: {
            printf("Sending reg display message\n");
            
            test_phev_register_e2e_out_handler_stage = 7;
            return msg_utils_createMsg(test_phev_register_regDisplayResponse,sizeof(test_phev_register_regDisplayResponse));
        }  
        default: {
            return NULL;
        }
    }
}

void test_phev_register_e2e_complete(void)
{
    test_phev_register_e2e_completed = true;
}
void test_phev_register_end_to_end(void)
{
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_register_inHandlerIn,
        .outgoingHandler = test_phev_register_outHandlerIn,
        .start = NULL,
        .stop = NULL,
        .connect = NULL,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_register_inHandlerOutE2E,
        .outgoingHandler = test_phev_register_outHandlerOutE2E,
        .start = NULL,
        .stop = NULL,
        .connect = NULL,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevServiceCtx_t * srvCtx = malloc(sizeof(phevServiceCtx_t));

    phev_pipe_settings_t pipeSettings = {
        .ctx = srvCtx,
        .in = in,
        .out = out,
        .inputSplitter = NULL,
        .outputSplitter = (msg_pipe_splitter_t) phev_pipe_outputSplitter,
        .inputResponder = NULL,
        .outputResponder = (msg_pipe_responder_t) phev_pipe_commandResponder,
        .outputOutputTransformer = (msg_pipe_transformer_t) phev_pipe_outputEventTransformer,
        .preConnectHook = NULL,
        .outputInputTransformer = (msg_pipe_transformer_t) phev_pipe_outputChainInputTransformer,
    };

    
    phev_pipe_ctx_t * pipe = phev_pipe_createPipe(pipeSettings);

    phevRegisterSettings_t settings = {
        .pipe = pipe,
        .mac = {0x2f,0x0d,0xc2,0xc2,0x91,0x85},
        .eventHandler = (phevPipeEventHandler_t) phev_register_eventHandler,
        .complete = (phevRegistrationComplete_t) test_phev_register_e2e_complete,
        .ctx = pipe->ctx,
    };

    phevRegisterCtx_t * ctx = phev_register_init(settings);

    ((phevServiceCtx_t *) pipe->ctx)->registrationCtx = ctx;

    int i = 0;

    while(!test_phev_register_e2e_completed)
    {
        msg_pipe_loop(pipe->pipe);
        if(i > 100) break;
        i++;
    }
    
    TEST_ASSERT_EQUAL(7,test_phev_register_e2e_out_handler_stage);
    TEST_ASSERT_EQUAL(7,test_phev_register_e2e_out_handler_out_stage);
    TEST_ASSERT_EQUAL(true,ctx->registrationComplete);

}