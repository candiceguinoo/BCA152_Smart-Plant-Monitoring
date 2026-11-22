#include "unity.h"
#include "moisture.c"
#include "validity.c"

void setUp(void) {}
void tearDown(void) {}

/* ---- adc validity ---- */
void test_adc_rejects_rails_and_out_of_range(void)
{
    TEST_ASSERT_FALSE(adc_raw_is_valid(0));
    TEST_ASSERT_FALSE(adc_raw_is_valid(4095));
    TEST_ASSERT_FALSE(adc_raw_is_valid(-1));
    TEST_ASSERT_FALSE(adc_raw_is_valid(5000));
}

void test_adc_accepts_normal_values(void)
{
    TEST_ASSERT_TRUE(adc_raw_is_valid(1));
    TEST_ASSERT_TRUE(adc_raw_is_valid(2048));
    TEST_ASSERT_TRUE(adc_raw_is_valid(4094));
}

/* ---- calibration validity ---- */
void test_cal_valid_both_directions(void)
{
    moisture_cal_t a = { .raw_dry = 3000, .raw_wet = 1200 };
    moisture_cal_t b = { .raw_dry = 500,  .raw_wet = 3500 };
    TEST_ASSERT_TRUE(moisture_cal_is_valid(&a));
    TEST_ASSERT_TRUE(moisture_cal_is_valid(&b));
}

void test_cal_rejects_bad_values(void)
{
    moisture_cal_t same  = { .raw_dry = 2000, .raw_wet = 2000 };
    moisture_cal_t close = { .raw_dry = 2000, .raw_wet = 2100 };
    moisture_cal_t rail  = { .raw_dry = 4095, .raw_wet = 1000 };
    TEST_ASSERT_FALSE(moisture_cal_is_valid(&same));
    TEST_ASSERT_FALSE(moisture_cal_is_valid(&close));
    TEST_ASSERT_FALSE(moisture_cal_is_valid(&rail));
    TEST_ASSERT_FALSE(moisture_cal_is_valid(NULL));
}

/* ---- moisture index ---- */
void test_index_dry_higher_than_wet(void)
{
    moisture_cal_t cal = { .raw_dry = 3000, .raw_wet = 1200 };
    TEST_ASSERT_EQUAL_INT(0,   moisture_index_pct(3000, &cal));
    TEST_ASSERT_EQUAL_INT(100, moisture_index_pct(1200, &cal));
    TEST_ASSERT_EQUAL_INT(50,  moisture_index_pct(2100, &cal));
}

void test_index_wet_higher_than_dry(void)
{
    moisture_cal_t cal = { .raw_dry = 500, .raw_wet = 3500 };
    TEST_ASSERT_EQUAL_INT(0,   moisture_index_pct(500, &cal));
    TEST_ASSERT_EQUAL_INT(100, moisture_index_pct(3500, &cal));
    TEST_ASSERT_EQUAL_INT(50,  moisture_index_pct(2000, &cal));
}

void test_index_clamps_outside_reference_range(void)
{
    moisture_cal_t cal = { .raw_dry = 3000, .raw_wet = 1200 };
    TEST_ASSERT_EQUAL_INT(0,   moisture_index_pct(3900, &cal));
    TEST_ASSERT_EQUAL_INT(100, moisture_index_pct(300, &cal));
}

void test_index_invalid_inputs_return_minus_one(void)
{
    moisture_cal_t cal  = { .raw_dry = 3000, .raw_wet = 1200 };
    moisture_cal_t bad  = { .raw_dry = 2000, .raw_wet = 2000 };
    TEST_ASSERT_EQUAL_INT(-1, moisture_index_pct(0, &cal));
    TEST_ASSERT_EQUAL_INT(-1, moisture_index_pct(4095, &cal));
    TEST_ASSERT_EQUAL_INT(-1, moisture_index_pct(2000, &bad));
    TEST_ASSERT_EQUAL_INT(-1, moisture_index_pct(2000, NULL));
}

/* ---- DHT11 plausibility ---- */
void test_dht11_limits(void)
{
    TEST_ASSERT_TRUE(dht11_temp_is_valid(0));
    TEST_ASSERT_TRUE(dht11_temp_is_valid(50));
    TEST_ASSERT_FALSE(dht11_temp_is_valid(-1));
    TEST_ASSERT_FALSE(dht11_temp_is_valid(51));
    TEST_ASSERT_TRUE(dht11_rh_is_valid(20));
    TEST_ASSERT_TRUE(dht11_rh_is_valid(90));
    TEST_ASSERT_FALSE(dht11_rh_is_valid(19));
    TEST_ASSERT_FALSE(dht11_rh_is_valid(91));
}

/* ---- stale readings ---- */
void test_stale_detection(void)
{
    TEST_ASSERT_FALSE(reading_is_stale(100, 105, 15));
    TEST_ASSERT_FALSE(reading_is_stale(100, 115, 15));
    TEST_ASSERT_TRUE(reading_is_stale(100, 116, 15));
    TEST_ASSERT_TRUE(reading_is_stale(200, 100, 15));   /* time went backwards */
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_adc_rejects_rails_and_out_of_range);
    RUN_TEST(test_adc_accepts_normal_values);
    RUN_TEST(test_cal_valid_both_directions);
    RUN_TEST(test_cal_rejects_bad_values);
    RUN_TEST(test_index_dry_higher_than_wet);
    RUN_TEST(test_index_wet_higher_than_dry);
    RUN_TEST(test_index_clamps_outside_reference_range);
    RUN_TEST(test_index_invalid_inputs_return_minus_one);
    RUN_TEST(test_dht11_limits);
    RUN_TEST(test_stale_detection);
    return UNITY_END();
}