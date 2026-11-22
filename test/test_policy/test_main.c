#include "unity.h"
#include "state.c"
#include "policy.c"

void setUp(void) {}
void tearDown(void) {}

/* ---- reason for a confirmed transition ---- */
void test_reason_initial_report(void)
{
    notify_reason_t r;
    TEST_ASSERT_TRUE(policy_reason_for_transition(SOIL_UNKNOWN, SOIL_NORMAL, &r));
    TEST_ASSERT_EQUAL_INT(REASON_INITIAL, r);
    TEST_ASSERT_TRUE(policy_reason_for_transition(SOIL_UNKNOWN, SOIL_DRY, &r));
    TEST_ASSERT_EQUAL_INT(REASON_INITIAL, r);
}

void test_reason_alert_into_dry_or_wet(void)
{
    notify_reason_t r;
    TEST_ASSERT_TRUE(policy_reason_for_transition(SOIL_NORMAL, SOIL_DRY, &r));
    TEST_ASSERT_EQUAL_INT(REASON_ALERT, r);
    TEST_ASSERT_TRUE(policy_reason_for_transition(SOIL_NORMAL, SOIL_WET, &r));
    TEST_ASSERT_EQUAL_INT(REASON_ALERT, r);
    TEST_ASSERT_TRUE(policy_reason_for_transition(SOIL_DRY, SOIL_WET, &r));
    TEST_ASSERT_EQUAL_INT(REASON_ALERT, r);
}

void test_reason_recovery(void)
{
    notify_reason_t r;
    TEST_ASSERT_TRUE(policy_reason_for_transition(SOIL_DRY, SOIL_NORMAL, &r));
    TEST_ASSERT_EQUAL_INT(REASON_RECOVERY, r);
    TEST_ASSERT_TRUE(policy_reason_for_transition(SOIL_WET, SOIL_NORMAL, &r));
    TEST_ASSERT_EQUAL_INT(REASON_RECOVERY, r);
}

void test_reason_none_when_no_change_or_unknown(void)
{
    notify_reason_t r = REASON_DEMO;
    TEST_ASSERT_FALSE(policy_reason_for_transition(SOIL_DRY, SOIL_DRY, &r));
    TEST_ASSERT_FALSE(policy_reason_for_transition(SOIL_NORMAL, SOIL_UNKNOWN, &r));
    TEST_ASSERT_FALSE(policy_reason_for_transition(SOIL_UNKNOWN, SOIL_UNKNOWN, &r));
    TEST_ASSERT_FALSE(policy_reason_for_transition(SOIL_DRY, SOIL_NORMAL, NULL));
    TEST_ASSERT_EQUAL_INT(REASON_DEMO, r);
}

void test_reason_labels(void)
{
    TEST_ASSERT_EQUAL_STRING("STATUS REPORT", notify_reason_label(REASON_INITIAL));
    TEST_ASSERT_EQUAL_STRING("ALERT",         notify_reason_label(REASON_ALERT));
    TEST_ASSERT_EQUAL_STRING("RECOVERY",      notify_reason_label(REASON_RECOVERY));
    TEST_ASSERT_EQUAL_STRING("DEMO TEST",     notify_reason_label(REASON_DEMO));
}

/* ---- cool-down ---- */
void test_cooldown_minimum_enforced(void)
{
    cooldown_t c;
    cooldown_init(&c, 60);
    TEST_ASSERT_EQUAL_INT(COOLDOWN_MIN_S, (int)c.cooldown_s);
    cooldown_init(&c, 1800);
    TEST_ASSERT_EQUAL_INT(1800, (int)c.cooldown_s);
}

void test_cooldown_blocks_duplicate_alert(void)
{
    cooldown_t c;
    cooldown_init(&c, 900);
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 1000));
    TEST_ASSERT_FALSE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 1300));
    TEST_ASSERT_FALSE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 1899));
}

void test_cooldown_allows_after_period(void)
{
    cooldown_t c;
    cooldown_init(&c, 900);
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 1000));
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 1900));   /* exactly 900 s */
    TEST_ASSERT_FALSE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 2000));  /* timer restarted */
}

void test_cooldown_is_per_state(void)
{
    cooldown_t c;
    cooldown_init(&c, 900);
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 1000));
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_WET, 1010));
    TEST_ASSERT_FALSE(policy_should_send(&c, REASON_ALERT, SOIL_WET, 1020));
}

void test_recovery_has_its_own_cooldown(void)
{
    cooldown_t c;
    cooldown_init(&c, 900);
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_RECOVERY, SOIL_NORMAL, 1000));
    TEST_ASSERT_FALSE(policy_should_send(&c, REASON_RECOVERY, SOIL_NORMAL, 1100));
}

void test_initial_and_demo_bypass_and_do_not_record(void)
{
    cooldown_t c;
    cooldown_init(&c, 900);
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_INITIAL, SOIL_DRY, 1000));
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_DEMO, SOIL_DRY, 1001));
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_DEMO, SOIL_DRY, 1002));
    /* the first real alert is still allowed, nothing was recorded */
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 1003));
}

void test_cooldown_survives_clock_jump_backwards(void)
{
    cooldown_t c;
    cooldown_init(&c, 900);
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 5000));
    TEST_ASSERT_TRUE(policy_should_send(&c, REASON_ALERT, SOIL_DRY, 100));   /* clock reset */
}

void test_cooldown_rejects_unknown_state_alert(void)
{
    cooldown_t c;
    cooldown_init(&c, 900);
    TEST_ASSERT_FALSE(policy_should_send(&c, REASON_ALERT, SOIL_UNKNOWN, 1000));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_reason_initial_report);
    RUN_TEST(test_reason_alert_into_dry_or_wet);
    RUN_TEST(test_reason_recovery);
    RUN_TEST(test_reason_none_when_no_change_or_unknown);
    RUN_TEST(test_reason_labels);
    RUN_TEST(test_cooldown_minimum_enforced);
    RUN_TEST(test_cooldown_blocks_duplicate_alert);
    RUN_TEST(test_cooldown_allows_after_period);
    RUN_TEST(test_cooldown_is_per_state);
    RUN_TEST(test_recovery_has_its_own_cooldown);
    RUN_TEST(test_initial_and_demo_bypass_and_do_not_record);
    RUN_TEST(test_cooldown_survives_clock_jump_backwards);
    RUN_TEST(test_cooldown_rejects_unknown_state_alert);
    return UNITY_END();
}