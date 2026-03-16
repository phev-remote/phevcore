#define LOGGING_ON
#define LOG_LEVEL LOG_DEBUG
#define MY18

#include "greatest.h"
#include "phev_model.h"
#include "logger.h"

TEST test_phev_model_create_model(void)
{
    phevModel_t * model = phev_model_create();

    ASSERT(model != NULL);
    PASS();
}
TEST test_phev_model_set_register(void)
{

    const uint8_t data[] = {1,2,3,4};

    uint8_t reg = 0x11;

    phevModel_t * model = phev_model_create();

    int ret = phev_model_setRegister(model,reg,data,4);

    ASSERT_EQ_FMT(1, ret, "%d");
    PASS();
}
TEST test_phev_model_get_register(void)
{
    const uint8_t data[] = {1,2,3,4};

    phevModel_t * model = phev_model_create();

    int ret = phev_model_setRegister(model,0x11,data,4);

    phevRegister_t * msg2 = phev_model_getRegister(model,0x11);

    ASSERT(msg2 != NULL);

    ASSERT_MEM_EQ(data,msg2->data,4);
    PASS();
}
TEST test_phev_model_get_register_not_found(void)
{
    phevModel_t * model = phev_model_create();

    phevRegister_t * msg2 = phev_model_getRegister(model,0x12);

    ASSERT_EQ(NULL, msg2);
    PASS();
}
TEST test_phev_model_update_register(void)
{
    const uint8_t data[] = {1,2,3,4};

    const uint8_t replacementData[] = {1,5,3,4};

    phevModel_t * model = phev_model_create();

    int ret = phev_model_setRegister(model,0x11,data,4);

    phevRegister_t * msg = phev_model_getRegister(model,0x11);

    ASSERT(msg != NULL);

    ASSERT_MEM_EQ(data,msg->data,4);

    ret = phev_model_setRegister(model,0x11,replacementData,4);

    phevRegister_t * msg2 = phev_model_getRegister(model,0x11);

    ASSERT(msg2 != NULL);

    ASSERT_MEM_EQ(replacementData,msg2->data,4);
    PASS();
}
TEST test_phev_model_register_compare(void)
{
    const uint8_t data[] = {1,2,3,4};

    const uint8_t newData[] = {1,2,3,4};

    phevModel_t * model = phev_model_create();

    phev_model_setRegister(model,0x11,data,4);

    int ret = phev_model_compareRegister(model,0x11,newData);

    ASSERT_EQ_FMT(0, ret, "%d");
    PASS();
}
TEST test_phev_model_register_compare_not_same(void)
{
    const uint8_t data[] = {1,2,3,4};

    const uint8_t newData[] = {1,2,4,4};

    phevModel_t * model = phev_model_create();

    phev_model_setRegister(model,0x11,data,4);

    int ret = phev_model_compareRegister(model,0x11,newData);

    ASSERT_NEQ(0, ret);
    PASS();
}
TEST test_phev_model_compare_not_set(void)
{
    const uint8_t data[] = {1,2,3,4};

    phevModel_t * model = phev_model_create();

    int ret = phev_model_compareRegister(model,0x11,data);

    ASSERT_NEQ(0, ret);
    PASS();
}

SUITE(phev_model)
{
    RUN_TEST(test_phev_model_create_model);
    RUN_TEST(test_phev_model_set_register);
    RUN_TEST(test_phev_model_get_register);
    RUN_TEST(test_phev_model_get_register_not_found);
    RUN_TEST(test_phev_model_update_register);
    RUN_TEST(test_phev_model_register_compare);
    RUN_TEST(test_phev_model_register_compare_not_same);
    RUN_TEST(test_phev_model_compare_not_set);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(phev_model);
    GREATEST_MAIN_END();
}
