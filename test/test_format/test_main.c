#include "unity.h"
#include <string.h>
#include "state.c"
#include "policy.c"
#include "format.c"

void setUp(void) {}
void tearDown(void) {}

static message_data_t base(void)
{
    message_data_t d = {
        .plant_id = "plant-01",
        .temp_c = 28, .temp_valid = true,
        .rh_pct = 60, .rh_valid = true,
        .soil = SOIL_DRY, .moisture_pct = 22,
        .timestamp = "2026-11-22 18:57:00",
        .reason = REASON_ALERT
    };
    return d;
}

void test_alert_message_exact(void)
{
    char buf[200];
    message_data_t d = base();
    int n = format_message(buf, sizeof buf, &d);
    TEST_ASSERT_EQUAL_STRING(
        "[plant-01] ALERT | Soil: DRY (22 %) | Temp: 28 C | Humidity: 60 %RH | Time: 2026-11-22 18:57:00 | Reason: confirmed change to DRY",
        buf);
    TEST_ASSERT_EQUAL_INT((int)strlen(buf), n);
}

void test_message_contains_required_fields(void)
{
    char buf[200];
    message_data_t d = base();
    format_message(buf, sizeof buf, &d);
    TEST_ASSERT_TRUE(strstr(buf, "plant-01") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "28 C") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "60 %RH") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "DRY") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "2026-11-22 18:57:00") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "Reason:") != NULL);
}

void test_recovery_message(void)
{
    char buf[200];
    message_data_t d = base();
    d.reason = REASON_RECOVERY;
    d.soil = SOIL_NORMAL;
    d.moisture_pct = 48;
    format_message(buf, sizeof buf, &d);
    TEST_ASSERT_TRUE(strstr(buf, "RECOVERY") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "Soil: NORMAL (48 %)") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "soil returned to NORMAL") != NULL);
}

void test_initial_and_demo_labels(void)
{
    char buf[200];
    message_data_t d = base();
    d.reason = REASON_INITIAL;
    format_message(buf, sizeof buf, &d);
    TEST_ASSERT_TRUE(strstr(buf, "STATUS REPORT") != NULL);
    d.reason = REASON_DEMO;
    format_message(buf, sizeof buf, &d);
    TEST_ASSERT_TRUE(strstr(buf, "DEMO TEST") != NULL);
}

void test_invalid_sensor_values_never_shown_as_numbers(void)
{
    char buf[200];
    message_data_t d = base();
    d.temp_valid = false;
    d.rh_valid = false;
    d.temp_c = 99;
    d.rh_pct = 99;
    format_message(buf, sizeof buf, &d);
    TEST_ASSERT_TRUE(strstr(buf, "Temp: n/a (sensor fault)") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "Humidity: n/a (sensor fault)") != NULL);
    TEST_ASSERT_TRUE(strstr(buf, "99") == NULL);
}

void test_unavailable_moisture_omits_percentage(void)
{
    char buf[200];
    message_data_t d = base();
    d.moisture_pct = -1;
    d.soil = SOIL_UNKNOWN;
    format_message(buf, sizeof buf, &d);
    TEST_ASSERT_TRUE(strstr(buf, "Soil: UNKNOWN |") != NULL);
}

void test_truncation_returns_error_and_terminates(void)
{
    char buf[20];
    message_data_t d = base();
    TEST_ASSERT_EQUAL_INT(-1, format_message(buf, sizeof buf, &d));
    TEST_ASSERT_EQUAL_INT(19, (int)strlen(buf));
}

void test_bad_arguments(void)
{
    char buf[100];
    message_data_t d = base();
    TEST_ASSERT_EQUAL_INT(-1, format_message(NULL, 100, &d));
    TEST_ASSERT_EQUAL_INT(-1, format_message(buf, 0, &d));
    TEST_ASSERT_EQUAL_INT(-1, format_message(buf, sizeof buf, NULL));
    d.plant_id = NULL;
    TEST_ASSERT_EQUAL_INT(-1, format_message(buf, sizeof buf, &d));
    d = base();
    d.timestamp = NULL;
    TEST_ASSERT_EQUAL_INT(-1, format_message(buf, sizeof buf, &d));
}

void test_subject(void)
{
    char buf[80];
    message_data_t d = base();
    int n = format_subject(buf, sizeof buf, &d);
    TEST_ASSERT_EQUAL_STRING("[plant-01] ALERT: soil DRY", buf);
    TEST_ASSERT_EQUAL_INT((int)strlen(buf), n);
    TEST_ASSERT_EQUAL_INT(-1, format_subject(buf, 10, &d));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_alert_message_exact);
    RUN_TEST(test_message_contains_required_fields);
    RUN_TEST(test_recovery_message);
    RUN_TEST(test_initial_and_demo_labels);
    RUN_TEST(test_invalid_sensor_values_never_shown_as_numbers);
    RUN_TEST(test_unavailable_moisture_omits_percentage);
    RUN_TEST(test_truncation_returns_error_and_terminates);
    RUN_TEST(test_bad_arguments);
    RUN_TEST(test_subject);
    return UNITY_END();
}