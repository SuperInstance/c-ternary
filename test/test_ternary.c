/**
 * test_ternary.c — Tests for c-ternary.h
 *
 * Compile: cc -std=c99 -lm test_ternary.c -o test_ternary && ./test_ternary
 */

#include "c-ternary.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name, expr) do { \
    if (!(expr)) { \
        printf("FAIL %s\n", name); \
        tests_failed++; \
    } else { \
        printf("PASS %s\n", name); \
        tests_passed++; \
    } \
} while(0)

void test_conviction_mapping(void) {
    printf("\n--- Conviction Mapping ---\n");
    TEST("1.0 => TRUE",  ternary_from_conviction(1.0)  == TRIT_TRUE);
    TEST("0.85 => TRUE", ternary_from_conviction(0.85) == TRIT_TRUE);
    TEST("0.70 => TRUE", ternary_from_conviction(0.70) == TRIT_TRUE);
    TEST("0.50 => UNCERTAIN", ternary_from_conviction(0.50) == TRIT_UNCERTAIN);
    TEST("0.30 => FALSE", ternary_from_conviction(0.30) == TRIT_FALSE);
    TEST("0.0 => FALSE",  ternary_from_conviction(0.0)  == TRIT_FALSE);
    TEST("-0.5 clamped to FALSE", ternary_from_conviction(-0.5) == TRIT_FALSE);
    TEST("1.5 clamped to TRUE",   ternary_from_conviction(1.5)  == TRIT_TRUE);
}

void test_ternary_l_fusion(void) {
    printf("\n--- TernaryL Fusion ---\n");
    ternary_l_t a = ternary_l_make(0.90);
    ternary_l_t b = ternary_l_make(0.50);
    ternary_l_t fused = ternary_l_fuse(a, b);
    TEST("fused conviction is average", fused.conviction == 0.70);
    TEST("fused gate is TRUE at 0.70",  fused.gate == TRIT_TRUE);

    ternary_l_t c = ternary_l_make(1.0);
    ternary_l_t d = ternary_l_make(0.0);
    ternary_l_t e = ternary_l_fuse(c, d);
    TEST("fused 1.0+0.0 => 0.5", e.conviction == 0.50);
    TEST("fused 0.5 => UNCERTAIN", e.gate == TRIT_UNCERTAIN);
}

void test_ternary_logic(void) {
    printf("\n--- Ternary Logic ---\n");
    // AND
    TEST("TRUE AND TRUE = TRUE",   ternary_and(TRIT_TRUE, TRIT_TRUE)  == TRIT_TRUE);
    TEST("TRUE AND FALSE = FALSE", ternary_and(TRIT_TRUE, TRIT_FALSE) == TRIT_FALSE);
    TEST("FALSE AND TRUE = FALSE", ternary_and(TRIT_FALSE, TRIT_TRUE) == TRIT_FALSE);
    TEST("0 AND ANY = 0 or less",  ternary_and(TRIT_UNCERTAIN, TRIT_TRUE) <= TRIT_UNCERTAIN);

    // OR
    TEST("TRUE OR FALSE = TRUE",   ternary_or(TRIT_TRUE, TRIT_FALSE)  == TRIT_TRUE);
    TEST("FALSE OR FALSE = FALSE", ternary_or(TRIT_FALSE, TRIT_FALSE) == TRIT_FALSE);
    TEST("0 OR TRUE = TRUE",       ternary_or(TRIT_UNCERTAIN, TRIT_TRUE) == TRIT_TRUE);

    // NOT
    TEST("NOT TRUE = FALSE",  ternary_not(TRIT_TRUE)  == TRIT_FALSE);
    TEST("NOT FALSE = TRUE",  ternary_not(TRIT_FALSE) == TRIT_TRUE);
    TEST("NOT 0 = 0",         ternary_not(TRIT_UNCERTAIN) == TRIT_UNCERTAIN);
}

void test_veto(void) {
    printf("\n--- Veto Operations ---\n");
    veto_signal_t p = veto_pass();
    TEST("pass is not vetoed", !p.is_vetoed);
    TEST("pass has tier NONE", p.tier == VETO_NONE);

    veto_signal_t r = veto_at(VETO_ROOM, "local constraint");
    TEST("room veto is vetoed", r.is_vetoed);
    TEST("room veto tier is ROOM", r.tier == VETO_ROOM);

    veto_signal_t m = veto_at(VETO_MARKET, "global halt");
    TEST("market veto tier is highest", m.tier == VETO_MARKET);

    veto_signal_t combined = veto_combine(r, m);
    TEST("combine r+m picks market", combined.tier == VETO_MARKET);
    TEST("combine r+m is vetoed", combined.is_vetoed);
}

void test_symmetry(void) {
    printf("\n--- Symmetry Markers ---\n");
    symmetry_marker_t s = symmetry_marker(42, 0.01);
    TEST("wasserstein 0.01 is symmetric", s.is_symmetric == TRIT_TRUE);
    TEST("orbit_id preserved", s.orbit_id == 42);

    symmetry_marker_t t = symmetry_marker(99, 0.50);
    TEST("wasserstein 0.5 is NOT symmetric", t.is_symmetric == TRIT_FALSE);

    symmetry_marker_t u = symmetry_marker(7, 0.07);
    TEST("wasserstein 0.07 borderline", u.is_symmetric == TRIT_UNCERTAIN);
}

void test_formatting(void) {
    printf("\n--- Formatting ---\n");
    char buf[64];

    ternary_l_t t = ternary_l_make(0.85);
    ternary_l_format(t, buf, sizeof(buf));
    TEST("ternary_l_format contains conviction", strstr(buf, "0.850") != NULL);
    TEST("ternary_l_format contains gate",       strchr(buf, '+')  != NULL);

    veto_signal_t v = veto_at(VETO_SECTOR, "over limit");
    veto_format(v, buf, sizeof(buf));
    TEST("veto_format contains SECTOR", strstr(buf, "SECTOR") != NULL);
    TEST("veto_format contains reason", strstr(buf, "over limit") != NULL);
}

void test_utilities(void) {
    printf("\n--- Utility Checks ---\n");
    TEST("is_certain(TRUE)",  ternary_is_certain(TRIT_TRUE));
    TEST("is_certain(FALSE)", ternary_is_certain(TRIT_FALSE));
    TEST("!is_certain(0)",    !ternary_is_certain(TRIT_UNCERTAIN));
    TEST("is_true(TRUE)",     ternary_is_true(TRIT_TRUE));
    TEST("!is_true(FALSE)",   !ternary_is_true(TRIT_FALSE));
    TEST("is_false(FALSE)",   ternary_is_false(TRIT_FALSE));
}

int main(void) {
    printf("=== c-ternary.h Test Suite ===\n");
    test_conviction_mapping();
    test_ternary_l_fusion();
    test_ternary_logic();
    test_veto();
    test_symmetry();
    test_formatting();
    test_utilities();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
