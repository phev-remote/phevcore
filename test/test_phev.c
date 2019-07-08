#include "unity.h"
#include "phev.h"

int test_phev_handler_connectCalled = 0;

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