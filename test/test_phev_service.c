#include <stdbool.h>
#include "unity.h"
#include "phev_service.h"

void test_phev_service_validateCommand(void)
{
    const char * command = "{ \"updateRegister\" : { \"reg\" : 1, \"value\" : 255 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_TRUE(ret);
}

void test_phev_service_validateCommand_empty(void)
{
    const char * command = "";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}

void test_phev_service_validateCommand_invalidJson(void)
{
    const char * command = "{ \"updateRegister\" :  }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}

void test_phev_service_validateCommand_updateRegister_invalid(void)
{
    const char * command = "{ \"updateRegister\" :  { \"someValue\" : 123 } }";
//    const char * command = "{ \"updateRegister\" :  { \"reg\" : 1, \"value\" : 255 } }";


    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}
void test_phev_service_validateCommand_updateRegister_valid(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 1, \"value\" : 255 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_TRUE(ret);
}
void test_phev_service_validateCommand_updateRegister_reg_out_of_range(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 555, \"value\" : 255 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}
void test_phev_service_validateCommand_updateRegister_value_out_of_range(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 1, \"value\" : 256 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}