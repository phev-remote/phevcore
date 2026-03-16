#define LOGGING_ON
#define LOG_LEVEL LOG_DEBUG
#define MY18

#include "unity.h"
#include "test_phev_model.c"

void setUp(void) { }
void tearDown(void) { }

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_phev_model_create_model);
    RUN_TEST(test_phev_model_set_register);
    RUN_TEST(test_phev_model_get_register);
    RUN_TEST(test_phev_model_get_register_not_found);
    RUN_TEST(test_phev_model_update_register);
    RUN_TEST(test_phev_model_register_compare);
    RUN_TEST(test_phev_model_register_compare_not_same);
    RUN_TEST(test_phev_model_compare_not_set);

    return UNITY_END();
}
