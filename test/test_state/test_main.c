#include "unity.h"
#include "state.c"

static const soil_thresholds_t TH = { .dry_below_pct = 30, .wet_above_pct = 70, .hysteresis_pct = 5 };

void setUp(void) {}
void tearDown(void) {}

/* ---- thresholds ---- */
void test_thresholds_valid(void)
{
    TEST_ASSERT_TRUE(soil_thresholds_valid(&TH));
}

void test_thresholds_invalid(void)
{
    soil_thresholds_t inverted = { 70, 30, 5 };
    soil_thresholds_t equal    = { 50, 50, 0 };
    soil_thresholds_t negative = { -1, 70, 5 };
    soil_thresholds_t over     = { 30, 101, 5 };
    soil_thresholds_t negh     = { 30, 70, -1 };
    soil_thresholds_t overlap  = { 30, 50, 12 };   /* exit bands cross */
    TEST_ASSERT_FALSE(soil_thresholds_valid(&inverted));
    TEST_ASSERT_FALSE(soil_thresholds_valid(&equal));
    TEST_ASSERT_FALSE(soil_thresholds_valid(&negative));
    TEST_ASSERT_FALSE(soil_thresholds_valid(&over));
    TEST_ASSERT_FALSE(soil_thresholds_valid(&negh));
    TEST_ASSERT_FALSE(soil_thresholds_valid(&overlap));
    TEST_ASSERT_FALSE(soil_thresholds_valid(NULL));
}

/* ---- classification ---- */
void test_classify_basic(void)
{
    TEST_ASSERT_EQUAL_INT(SOIL_DRY,    soil_classify(10, &TH, SOIL_UNKNOWN));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, soil_classify(50, &TH, SOIL_UNKNOWN));
    TEST_ASSERT_EQUAL_INT(SOIL_WET,    soil_classify(90, &TH, SOIL_UNKNOWN));
}

void test_classify_boundaries(void)
{
    TEST_ASSERT_EQUAL_INT(SOIL_DRY,    soil_classify(29, &TH, SOIL_NORMAL));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, soil_classify(30, &TH, SOIL_NORMAL));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, soil_classify(70, &TH, SOIL_NORMAL));
    TEST_ASSERT_EQUAL_INT(SOIL_WET,    soil_classify(71, &TH, SOIL_NORMAL));
}

void test_hysteresis_leaving_dry(void)
{
    TEST_ASSERT_EQUAL_INT(SOIL_DRY,    soil_classify(33, &TH, SOIL_DRY));
    TEST_ASSERT_EQUAL_INT(SOIL_DRY,    soil_classify(34, &TH, SOIL_DRY));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, soil_classify(35, &TH, SOIL_DRY));
    /* from NORMAL, 33 is not dry */
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, soil_classify(33, &TH, SOIL_NORMAL));
}

void test_hysteresis_leaving_wet(void)
{
    TEST_ASSERT_EQUAL_INT(SOIL_WET,    soil_classify(67, &TH, SOIL_WET));
    TEST_ASSERT_EQUAL_INT(SOIL_WET,    soil_classify(66, &TH, SOIL_WET));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, soil_classify(65, &TH, SOIL_WET));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, soil_classify(69, &TH, SOIL_NORMAL));
}

void test_classify_dry_straight_to_wet(void)
{
    TEST_ASSERT_EQUAL_INT(SOIL_WET, soil_classify(90, &TH, SOIL_DRY));
}

void test_classify_invalid_input_is_unknown(void)
{
    soil_thresholds_t bad = { 70, 30, 5 };
    TEST_ASSERT_EQUAL_INT(SOIL_UNKNOWN, soil_classify(-1, &TH, SOIL_NORMAL));
    TEST_ASSERT_EQUAL_INT(SOIL_UNKNOWN, soil_classify(101, &TH, SOIL_NORMAL));
    TEST_ASSERT_EQUAL_INT(SOIL_UNKNOWN, soil_classify(50, &bad, SOIL_NORMAL));
    TEST_ASSERT_EQUAL_INT(SOIL_UNKNOWN, soil_classify(50, NULL, SOIL_NORMAL));
}

void test_state_names(void)
{
    TEST_ASSERT_EQUAL_STRING("DRY",     soil_state_name(SOIL_DRY));
    TEST_ASSERT_EQUAL_STRING("NORMAL",  soil_state_name(SOIL_NORMAL));
    TEST_ASSERT_EQUAL_STRING("WET",     soil_state_name(SOIL_WET));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", soil_state_name(SOIL_UNKNOWN));
}

/* ---- 3-reading confirmation ---- */
void test_tracker_initial_state_needs_three(void)
{
    state_tracker_t tr;
    state_tracker_init(&tr, 3);
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_NORMAL, true));
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_NORMAL, true));
    TEST_ASSERT_TRUE(state_tracker_update(&tr, SOIL_NORMAL, true));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, tr.confirmed);
}

void test_tracker_change_needs_three_in_a_row(void)
{
    state_tracker_t tr;
    state_tracker_init(&tr, 3);
    for (int i = 0; i < 3; i++) state_tracker_update(&tr, SOIL_NORMAL, true);

    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_DRY, true));
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_DRY, true));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, tr.confirmed);
    TEST_ASSERT_TRUE(state_tracker_update(&tr, SOIL_DRY, true));
    TEST_ASSERT_EQUAL_INT(SOIL_DRY, tr.confirmed);
}

void test_tracker_return_to_confirmed_resets_streak(void)
{
    state_tracker_t tr;
    state_tracker_init(&tr, 3);
    for (int i = 0; i < 3; i++) state_tracker_update(&tr, SOIL_NORMAL, true);

    state_tracker_update(&tr, SOIL_DRY, true);
    state_tracker_update(&tr, SOIL_DRY, true);
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_NORMAL, true));
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_DRY, true));
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_DRY, true));
    TEST_ASSERT_TRUE(state_tracker_update(&tr, SOIL_DRY, true));
}

void test_tracker_invalid_reading_breaks_streak(void)
{
    state_tracker_t tr;
    state_tracker_init(&tr, 3);
    for (int i = 0; i < 3; i++) state_tracker_update(&tr, SOIL_NORMAL, true);

    state_tracker_update(&tr, SOIL_DRY, true);
    state_tracker_update(&tr, SOIL_DRY, true);
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_DRY, false));   /* sensor fault */
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_DRY, true));
    TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_DRY, true));
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, tr.confirmed);
    TEST_ASSERT_TRUE(state_tracker_update(&tr, SOIL_DRY, true));
}

void test_tracker_invalid_never_confirms_first_state(void)
{
    state_tracker_t tr;
    state_tracker_init(&tr, 3);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_FALSE(state_tracker_update(&tr, SOIL_NORMAL, false));
    }
    TEST_ASSERT_EQUAL_INT(SOIL_UNKNOWN, tr.confirmed);
}

void test_tracker_flapping_never_confirms(void)
{
    state_tracker_t tr;
    state_tracker_init(&tr, 3);
    for (int i = 0; i < 3; i++) state_tracker_update(&tr, SOIL_NORMAL, true);

    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_FALSE(state_tracker_update(&tr, (i % 2) ? SOIL_WET : SOIL_DRY, true));
    }
    TEST_ASSERT_EQUAL_INT(SOIL_NORMAL, tr.confirmed);
}

void test_tracker_required_minimum_is_one(void)
{
    state_tracker_t tr;
    state_tracker_init(&tr, 0);
    TEST_ASSERT_EQUAL_INT(1, tr.required);
    TEST_ASSERT_TRUE(state_tracker_update(&tr, SOIL_WET, true));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_thresholds_valid);
    RUN_TEST(test_thresholds_invalid);
    RUN_TEST(test_classify_basic);
    RUN_TEST(test_classify_boundaries);
    RUN_TEST(test_hysteresis_leaving_dry);
    RUN_TEST(test_hysteresis_leaving_wet);
    RUN_TEST(test_classify_dry_straight_to_wet);
    RUN_TEST(test_classify_invalid_input_is_unknown);
    RUN_TEST(test_state_names);
    RUN_TEST(test_tracker_initial_state_needs_three);
    RUN_TEST(test_tracker_change_needs_three_in_a_row);
    RUN_TEST(test_tracker_return_to_confirmed_resets_streak);
    RUN_TEST(test_tracker_invalid_reading_breaks_streak);
    RUN_TEST(test_tracker_invalid_never_confirms_first_state);
    RUN_TEST(test_tracker_flapping_never_confirms);
    RUN_TEST(test_tracker_required_minimum_is_one);
    return UNITY_END();
}