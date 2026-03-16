#define LOGGING_ON
#define LOG_LEVEL LOG_DEBUG
#define MY18

#include "unity.h"
#include "test_phev_register.c"

void setUp(void) { }
void tearDown(void) { }

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_phev_register_bootstrap);
    RUN_TEST(test_phev_register_should_send_register_on_vin);
    RUN_TEST(test_phev_register_should_trigger_aa_ack_event);
    RUN_TEST(test_phev_register_should_send_init);
    RUN_TEST(test_phev_register_should_call_complete_when_registered);
    RUN_TEST(test_phev_register_getVin);
    RUN_TEST(test_phev_register_should_error_when_too_many_registrations);
    RUN_TEST(test_phev_register_should_get_start_ack);
    RUN_TEST(test_phev_register_should_get_aa_ack);
    RUN_TEST(test_phev_register_should_get_registration);
    RUN_TEST(test_phev_register_should_get_ecu_version);
    RUN_TEST(test_phev_register_should_get_remote_security_present);
    RUN_TEST(test_phev_register_should_get_reg_disp);
    RUN_TEST(test_phev_register_end_to_end);

    return UNITY_END();
}
