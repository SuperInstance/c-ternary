/**
 * test_ternary.c — Comprehensive test suite for c-ternary.h
 *
 * Tests: type conversion, arithmetic, vectors, symmetry, veto, deadband
 */

#include "c-ternary.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  %-50s", name);
#define PASS() do { printf("✅\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("❌ %s\n", msg); tests_failed++; } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { FAIL(#a " != " #b); return; } } while(0)
#define ASSERT_NEQ(a, b) do { if ((a) == (b)) { FAIL(#a " == " #b " (expected !=)"); return; } } while(0)

/* ─── Type Conversion Tests ────────────────────────────────────────────── */

static void test_from_int(void) {
    TEST("ternary_from_int");
    ASSERT_EQ(ternary_from_int(5), TERNARY_POS);
    ASSERT_EQ(ternary_from_int(-3), TERNARY_NEG);
    ASSERT_EQ(ternary_from_int(0), TERNARY_ZERO);
    ASSERT_EQ(ternary_from_int(1), TERNARY_POS);
    ASSERT_EQ(ternary_from_int(-1), TERNARY_NEG);
    ASSERT_EQ(ternary_from_int(100), TERNARY_POS);
    PASS();
}

static void test_from_double_default_deadband(void) {
    TEST("ternary_from_double (default deadband)");
    ASSERT_EQ(ternary_from_double(0.8), TERNARY_POS);
    ASSERT_EQ(ternary_from_double(0.2), TERNARY_NEG);
    ASSERT_EQ(ternary_from_double(0.5), TERNARY_ZERO);  /* inside deadband */
    ASSERT_EQ(ternary_from_double(0.3), TERNARY_NEG);   /* boundary low */
    ASSERT_EQ(ternary_from_double(0.7), TERNARY_POS);   /* boundary high */
    ASSERT_EQ(ternary_from_double(1.0), TERNARY_POS);
    ASSERT_EQ(ternary_from_double(-0.5), TERNARY_NEG);
    PASS();
}

static void test_from_double_custom_deadband(void) {
    TEST("ternary_with_deadband (custom)");
    ASSERT_EQ(ternary_with_deadband(0.9, 0.1, 0.9), TERNARY_POS);
    ASSERT_EQ(ternary_with_deadband(0.05, 0.1, 0.9), TERNARY_NEG);
    ASSERT_EQ(ternary_with_deadband(0.5, 0.1, 0.9), TERNARY_ZERO);
    ASSERT_EQ(ternary_with_deadband(0.4, 0.3, 0.5), TERNARY_ZERO); /* inside deadband */
    PASS();
}

/* ─── Arithmetic Tests ─────────────────────────────────────────────────── */

static void test_negate(void) {
    TEST("ternary_negate");
    ASSERT_EQ(ternary_negate(TERNARY_POS), TERNARY_NEG);
    ASSERT_EQ(ternary_negate(TERNARY_NEG), TERNARY_POS);
    ASSERT_EQ(ternary_negate(TERNARY_ZERO), TERNARY_ZERO);
    PASS();
}

static void test_add(void) {
    TEST("ternary_add");
    int carry = 99;
    ASSERT_EQ(ternary_add(TERNARY_POS, TERNARY_NEG, &carry), TERNARY_ZERO);
    ASSERT_EQ(carry, 0);

    ASSERT_EQ(ternary_add(TERNARY_POS, TERNARY_POS, &carry), TERNARY_NEG);
    ASSERT_EQ(carry, 1);

    ASSERT_EQ(ternary_add(TERNARY_NEG, TERNARY_NEG, &carry), TERNARY_POS);
    ASSERT_EQ(carry, -1);

    ASSERT_EQ(ternary_add(TERNARY_ZERO, TERNARY_POS, NULL), TERNARY_POS);
    ASSERT_EQ(ternary_add(TERNARY_ZERO, TERNARY_NEG, NULL), TERNARY_NEG);
    PASS();
}

static void test_multiply(void) {
    TEST("ternary_multiply");
    ASSERT_EQ(ternary_multiply(TERNARY_POS, TERNARY_POS), TERNARY_POS);
    ASSERT_EQ(ternary_multiply(TERNARY_POS, TERNARY_NEG), TERNARY_NEG);
    ASSERT_EQ(ternary_multiply(TERNARY_NEG, TERNARY_NEG), TERNARY_POS);
    ASSERT_EQ(ternary_multiply(TERNARY_ZERO, TERNARY_POS), TERNARY_ZERO);
    ASSERT_EQ(ternary_multiply(TERNARY_ZERO, TERNARY_NEG), TERNARY_ZERO);
    PASS();
}

static void test_roundtrip(void) {
    TEST("int → ternary → int roundtrip");
    ASSERT_EQ(ternary_to_int(ternary_from_int(0)), 0);
    ASSERT_EQ(ternary_to_int(ternary_from_int(7)), 1);
    ASSERT_EQ(ternary_to_int(ternary_from_int(-7)), -1);
    PASS();
}

/* ─── Vector Tests ─────────────────────────────────────────────────────── */

static void test_vec_create_destroy(void) {
    TEST("ternary_vec new/free");
    ternary_vec_t v = ternary_vec_new(5);
    ASSERT_EQ(v.len, 5);
    ASSERT_NEQ(v.data, NULL);
    for (size_t i = 0; i < 5; i++) ASSERT_EQ(v.data[i], TERNARY_ZERO);
    ternary_vec_free(&v);
    ASSERT_EQ(v.data, NULL);
    ASSERT_EQ(v.len, 0);
    PASS();
}

static void test_vec_get_set(void) {
    TEST("ternary_vec get/set");
    ternary_vec_t v = ternary_vec_new(4);
    ternary_vec_set(&v, 0, TERNARY_POS);
    ternary_vec_set(&v, 1, TERNARY_NEG);
    ternary_vec_set(&v, 2, TERNARY_ZERO);
    ternary_vec_set(&v, 3, TERNARY_POS);
    ASSERT_EQ(ternary_vec_get(&v, 0), TERNARY_POS);
    ASSERT_EQ(ternary_vec_get(&v, 1), TERNARY_NEG);
    ASSERT_EQ(ternary_vec_get(&v, 2), TERNARY_ZERO);
    ASSERT_EQ(ternary_vec_get(&v, 3), TERNARY_POS);
    ASSERT_EQ(ternary_vec_get(&v, 99), TERNARY_ZERO); /* OOB */
    ternary_vec_free(&v);
    PASS();
}

static void test_vec_conservation_balanced(void) {
    TEST("ternary_vec conservation (balanced)");
    ternary_vec_t v = ternary_vec_new(4);
    ternary_vec_set(&v, 0, TERNARY_POS);
    ternary_vec_set(&v, 1, TERNARY_POS);
    ternary_vec_set(&v, 2, TERNARY_NEG);
    ternary_vec_set(&v, 3, TERNARY_NEG);
    ASSERT_EQ(ternary_vec_sum(&v), 0);
    ASSERT_EQ(ternary_vec_conservation(&v), 0);
    ASSERT_EQ(ternary_vec_is_balanced(&v), 1);
    ternary_vec_free(&v);
    PASS();
}

static void test_vec_conservation_unbalanced(void) {
    TEST("ternary_vec conservation (unbalanced)");
    ternary_vec_t v = ternary_vec_new(3);
    ternary_vec_set(&v, 0, TERNARY_POS);
    ternary_vec_set(&v, 1, TERNARY_POS);
    ternary_vec_set(&v, 2, TERNARY_ZERO);
    ASSERT_EQ(ternary_vec_sum(&v), 2);
    ASSERT_EQ(ternary_vec_is_balanced(&v), 0);
    ternary_vec_free(&v);
    PASS();
}

/* ─── Symmetry Tests ───────────────────────────────────────────────────── */

static void test_symmetry_check(void) {
    TEST("ternary_symmetry_check (XOR mask)");
    ternary_t a[] = {TERNARY_POS, TERNARY_NEG, TERNARY_POS, TERNARY_ZERO};
    int mask = ternary_symmetry_check(a, 4);
    ASSERT_EQ(mask, (1 ^ (-1) ^ 1 ^ 0));  /* = 1 */
    PASS();
}

static void test_hamming_distance(void) {
    TEST("ternary_hamming distance");
    ternary_t a[] = {TERNARY_POS, TERNARY_NEG, TERNARY_ZERO, TERNARY_POS};
    ternary_t b[] = {TERNARY_POS, TERNARY_POS, TERNARY_ZERO, TERNARY_NEG};
    ASSERT_EQ(ternary_hamming(a, b, 4), 2);
    PASS();
}

static void test_vec_hamming(void) {
    TEST("ternary_vec_hamming");
    ternary_vec_t a = ternary_vec_new(3);
    ternary_vec_t b = ternary_vec_new(3);
    ternary_vec_set(&a, 0, TERNARY_POS);
    ternary_vec_set(&a, 1, TERNARY_NEG);
    ternary_vec_set(&a, 2, TERNARY_ZERO);
    ternary_vec_set(&b, 0, TERNARY_POS);
    ternary_vec_set(&b, 1, TERNARY_POS);
    ternary_vec_set(&b, 2, TERNARY_ZERO);
    ASSERT_EQ(ternary_vec_hamming(&a, &b), 1);
    ternary_vec_free(&a);
    ternary_vec_free(&b);
    PASS();
}

/* ─── Veto Tests ───────────────────────────────────────────────────────── */

static void test_veto_check(void) {
    TEST("ternary_veto_check");
    ASSERT_EQ(ternary_veto_check(TERNARY_VETO_ROOM, TERNARY_NEG), 1);
    ASSERT_EQ(ternary_veto_check(TERNARY_VETO_ROOM, TERNARY_POS), 0);
    ASSERT_EQ(ternary_veto_check(TERNARY_VETO_MARKET, TERNARY_ZERO), 0);
    PASS();
}

static void test_veto_propagate(void) {
    TEST("ternary_veto_propagate (hierarchy)");
    ternary_t levels[TERNARY_VETO_COUNT] = {
        TERNARY_POS,   /* ROOM: ok */
        TERNARY_NEG,   /* SECTOR: veto! */
        TERNARY_POS,   /* PORTFOLIO: ok */
        TERNARY_POS,   /* MARKET: ok */
    };
    ASSERT_EQ(ternary_veto_propagate(levels, TERNARY_VETO_PORTFOLIO), 1);

    /* All clear — no veto */
    ternary_t clear[TERNARY_VETO_COUNT] = {
        TERNARY_POS, TERNARY_POS, TERNARY_POS, TERNARY_POS
    };
    ASSERT_EQ(ternary_veto_propagate(clear, TERNARY_VETO_MARKET), 0);
    PASS();
}

/* ─── String Tests ─────────────────────────────────────────────────────── */

static void test_to_char(void) {
    TEST("ternary_to_char");
    ASSERT_EQ(ternary_to_char(TERNARY_POS), '+');
    ASSERT_EQ(ternary_to_char(TERNARY_NEG), '-');
    ASSERT_EQ(ternary_to_char(TERNARY_ZERO), '0');
    PASS();
}

static void test_vec_to_str(void) {
    TEST("ternary_vec_to_str");
    ternary_vec_t v = ternary_vec_new(5);
    ternary_vec_set(&v, 0, TERNARY_NEG);
    ternary_vec_set(&v, 1, TERNARY_ZERO);
    ternary_vec_set(&v, 2, TERNARY_POS);
    ternary_vec_set(&v, 3, TERNARY_NEG);
    ternary_vec_set(&v, 4, TERNARY_POS);
    char buf[16];
    size_t n = ternary_vec_to_str(&v, buf, sizeof(buf));
    ASSERT_EQ(n, 5);
    ASSERT_EQ(strcmp(buf, "-0+-+"), 0);
    ternary_vec_free(&v);
    PASS();
}

/* ─── Validity ─────────────────────────────────────────────────────────── */

static void test_valid(void) {
    TEST("ternary_valid");
    ASSERT_EQ(ternary_valid(TERNARY_POS), 1);
    ASSERT_EQ(ternary_valid(TERNARY_NEG), 1);
    ASSERT_EQ(ternary_valid(TERNARY_ZERO), 1);
    ASSERT_EQ(ternary_valid(2), 0);
    ASSERT_EQ(ternary_valid(-2), 0);
    PASS();
}

/* ─── Main ─────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== c-ternary Test Suite ===\n\n");

    /* Conversion */
    test_from_int();
    test_from_double_default_deadband();
    test_from_double_custom_deadband();

    /* Arithmetic */
    test_negate();
    test_add();
    test_multiply();
    test_roundtrip();

    /* Vectors */
    test_vec_create_destroy();
    test_vec_get_set();
    test_vec_conservation_balanced();
    test_vec_conservation_unbalanced();

    /* Symmetry */
    test_symmetry_check();
    test_hamming_distance();
    test_vec_hamming();

    /* Veto */
    test_veto_check();
    test_veto_propagate();

    /* String */
    test_to_char();
    test_vec_to_str();

    /* Validity */
    test_valid();

    printf("\n─── Results: %d passed, %d failed ───\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
