#include "unity.h"
#include "phev_model.h"
#include "logger.h"

void test_phev_model_create_model(void)
{
    phevModel_t * model = phev_model_create();

    TEST_ASSERT_NOT_NULL(model);
}
void test_phev_model_set_register(void)
{

    const uint8_t data[] = {1,2,3,4};

    uint8_t reg = 0x11;
    
    phevModel_t * model = phev_model_create();

    int ret = phev_model_setRegister(model,reg,data,4);

    TEST_ASSERT_EQUAL(1,ret);
} 
void test_phev_model_get_register(void)
{
    const uint8_t data[] = {1,2,3,4};

    phevModel_t * model = phev_model_create();

    int ret = phev_model_setRegister(model,0x11,data,4);

    phevRegister_t * msg2 = phev_model_getRegister(model,0x11);

    TEST_ASSERT_NOT_NULL(msg2);

    TEST_ASSERT_EQUAL_MEMORY(data,msg2->data,4);

} 
void test_phev_model_get_register_not_found(void)
{
    phevModel_t * model = phev_model_create();

    phevRegister_t * msg2 = phev_model_getRegister(model,0x12);

    TEST_ASSERT_NULL(msg2);

} 
void test_phev_model_update_register(void)
{
    const uint8_t data[] = {1,2,3,4};

    const uint8_t replacementData[] = {1,5,3,4};

    phevModel_t * model = phev_model_create();

    int ret = phev_model_setRegister(model,0x11,data,4);

    phevRegister_t * msg = phev_model_getRegister(model,0x11);

    TEST_ASSERT_NOT_NULL(msg);

    TEST_ASSERT_EQUAL_MEMORY(data,msg->data,4);

    ret = phev_model_setRegister(model,0x11,replacementData,4);

    phevRegister_t * msg2 = phev_model_getRegister(model,0x11);

    TEST_ASSERT_NOT_NULL(msg2);

    TEST_ASSERT_EQUAL_MEMORY(replacementData,msg2->data,4);
}
void test_phev_model_register_compare(void)
{
    const uint8_t data[] = {1,2,3,4};

    const uint8_t newData[] = {1,2,3,4};
    
    phevModel_t * model = phev_model_create();

    phev_model_setRegister(model,0x11,data,4);

    int ret = phev_model_compareRegister(model,0x11,newData);

    TEST_ASSERT_EQUAL(0,ret);

} 
void test_phev_model_register_compare_not_same(void)
{
    const uint8_t data[] = {1,2,3,4};

    const uint8_t newData[] = {1,2,4,4};
    
    phevModel_t * model = phev_model_create();

    phev_model_setRegister(model,0x11,data,4);

    int ret = phev_model_compareRegister(model,0x11,newData);

    TEST_ASSERT_NOT_EQUAL(0,ret);

} 
void test_phev_model_compare_not_set(void)
{
    const uint8_t data[] = {1,2,3,4};

    phevModel_t * model = phev_model_create();

    int ret = phev_model_compareRegister(model,0x11,data);

    TEST_ASSERT_NOT_EQUAL(0,ret);

}