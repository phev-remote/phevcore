#include "greatest.h"
#include "phev_config.h"
#include <stdio.h>
#include <sys/stat.h>
#include "cJSON.h"

/* ── Fixture path passed from CMake, fallback to local ── */
#ifndef CONFIG_JSON_PATH
#define CONFIG_JSON_PATH "config.json"
#endif

/* ── Shared fixture state ── */
static char *buffer = NULL;

static void load_fixture(void)
{
    if (buffer != NULL) return; /* already loaded */

    FILE *f = fopen(CONFIG_JSON_PATH, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Cannot open fixture: %s\n", CONFIG_JSON_PATH);
        return;
    }

    struct stat st;
    stat(CONFIG_JSON_PATH, &st);
    size_t size = (size_t)st.st_size;

    buffer = malloc(size + 1);
    size_t num = fread(buffer, 1, size, f);
    buffer[num] = '\0';
    fclose(f);
}

static void free_fixture(void)
{
    free(buffer);
    buffer = NULL;
}

/* ================================================================
   Tests that require the JSON fixture
   ================================================================ */

TEST test_phev_config_bootstrap(void)
{
    load_fixture();
    ASSERT(buffer != NULL);

    phevConfig_t *config = phev_config_parseConfig(buffer);

    ASSERT(config != NULL);
    free(config);
    PASS();
}

TEST test_phev_config_updateConfig(void)
{
    load_fixture();
    ASSERT(buffer != NULL);

    phevConfig_t *config = phev_config_parseConfig(buffer);

    ASSERT(config != NULL);
    ASSERT(config->updateConfig.updatePath != NULL);
    ASSERT(config->updateConfig.updateHost != NULL);
    ASSERT(config->updateConfig.updateImageFullPath != NULL);

    ASSERT_EQ(999, config->updateConfig.latestBuild);
    ASSERT_EQ(BUILD_NUMBER, config->updateConfig.currentBuild);
    ASSERT_STR_EQ("storage.googleapis.com", config->updateConfig.updateHost);
    ASSERT_EQ(80, config->updateConfig.updatePort);
    ASSERT_STR_EQ("/espimages/develop/", config->updateConfig.updatePath);
    ASSERT_STR_EQ("/espimages/develop/firmware-0000000999.bin",
                   config->updateConfig.updateImageFullPath);
    ASSERT(config->updateConfig.updateOverPPP);
    ASSERT(config->updateConfig.forceUpdate);

    free(config);
    PASS();
}

TEST test_phev_config_updateConfig_wifi(void)
{
    load_fixture();
    ASSERT(buffer != NULL);

    phevConfig_t *config = phev_config_parseConfig(buffer);

    ASSERT(config != NULL);
    ASSERT(config->updateConfig.updateWifi.ssid[0] != '\0');
    ASSERT(config->updateConfig.updateWifi.password[0] != '\0');

    ASSERT_STR_EQ("ssid", config->updateConfig.updateWifi.ssid);
    ASSERT_STR_EQ("password", config->updateConfig.updateWifi.password);

    free(config);
    PASS();
}

TEST test_phev_config_connection(void)
{
    load_fixture();
    ASSERT(buffer != NULL);

    phevConfig_t *config = phev_config_parseConfig(buffer);

    ASSERT(config != NULL);
    ASSERT(config->connectionConfig.host != NULL);
    ASSERT(config->connectionConfig.carConnectionWifi.ssid[0] != '\0');
    ASSERT(config->connectionConfig.carConnectionWifi.password[0] != '\0');

    ASSERT_STR_EQ("192.168.8.46", config->connectionConfig.host);
    ASSERT_EQ(8080, config->connectionConfig.port);
    ASSERT_STR_EQ("REMOTE123456",
                   config->connectionConfig.carConnectionWifi.ssid);
    ASSERT_STR_EQ("abcde123456",
                   config->connectionConfig.carConnectionWifi.password);

    free(config);
    PASS();
}

TEST test_phev_config_state(void)
{
    load_fixture();
    ASSERT(buffer != NULL);

    phevConfig_t *config = phev_config_parseConfig(buffer);

    ASSERT_EQ(1, config->state.connectedClients);
    ASSERT(config->state.headLightsOn);
    ASSERT(config->state.parkLightsOn);
    ASSERT(config->state.airConOn);

    free(config);
    PASS();
}

TEST test_phev_config_check_for_option_present(void)
{
    load_fixture();
    ASSERT(buffer != NULL);

    cJSON *json = cJSON_Parse((const char *)buffer);
    ASSERT(json != NULL);

    ASSERT(phev_config_checkForOption(json, "carConnection"));

    cJSON_Delete(json);
    PASS();
}

TEST test_phev_config_check_for_option_not_present(void)
{
    load_fixture();
    ASSERT(buffer != NULL);

    cJSON *json = cJSON_Parse((const char *)buffer);
    ASSERT(json != NULL);

    ASSERT_FALSE(phev_config_checkForOption(json, "airConOnNotThere"));

    cJSON_Delete(json);
    PASS();
}

/* ================================================================
   Tests that do NOT need the fixture
   ================================================================ */

TEST test_phev_config_update_firmware(void)
{
    phevUpdateConfig_t config = {
        .latestBuild = 99,
        .currentBuild = 98,
        .forceUpdate = false,
    };

    ASSERT(phev_config_checkForFirmwareUpdate(&config));
    PASS();
}

TEST test_phev_config_force_update_firmware(void)
{
    phevUpdateConfig_t config = {
        .latestBuild = 99,
        .currentBuild = 99,
        .forceUpdate = true,
    };

    ASSERT(phev_config_checkForFirmwareUpdate(&config));
    PASS();
}

TEST test_phev_config_not_update_firmware(void)
{
    phevUpdateConfig_t config = {
        .latestBuild = 99,
        .currentBuild = 99,
        .forceUpdate = false,
    };

    ASSERT_FALSE(phev_config_checkForFirmwareUpdate(&config));
    PASS();
}

TEST test_phev_config_no_connections(void)
{
    phevState_t config = {
        .connectedClients = 0,
    };

    ASSERT_FALSE(phev_config_checkForConnection(&config));
    PASS();
}

TEST test_phev_config_has_connections(void)
{
    phevState_t config = {
        .connectedClients = 1,
    };

    ASSERT(phev_config_checkForConnection(&config));
    PASS();
}

/* ================================================================
   Suites
   ================================================================ */

SUITE(phev_config_fixture_suite)
{
    RUN_TEST(test_phev_config_bootstrap);
    RUN_TEST(test_phev_config_updateConfig);
    RUN_TEST(test_phev_config_updateConfig_wifi);
    RUN_TEST(test_phev_config_connection);
    RUN_TEST(test_phev_config_state);
    RUN_TEST(test_phev_config_check_for_option_present);
    RUN_TEST(test_phev_config_check_for_option_not_present);
}

SUITE(phev_config_no_fixture_suite)
{
    RUN_TEST(test_phev_config_update_firmware);
    RUN_TEST(test_phev_config_force_update_firmware);
    RUN_TEST(test_phev_config_not_update_firmware);
    RUN_TEST(test_phev_config_no_connections);
    RUN_TEST(test_phev_config_has_connections);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(phev_config_fixture_suite);
    RUN_SUITE(phev_config_no_fixture_suite);
    GREATEST_MAIN_END();
}
