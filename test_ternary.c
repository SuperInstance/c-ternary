#define C_TERNARY_IMPL
#include "c-ternary.h"
#include <stdio.h>
#include <assert.h>

int main(void)
{
    /* Test conviction-to-trit mapping */
    assert(ct_conviction_to_trit(0.0)  == CT_TRIT_NEG);
    assert(ct_conviction_to_trit(0.29) == CT_TRIT_NEG);
    assert(ct_conviction_to_trit(0.30) == CT_TRIT_ZERO);
    assert(ct_conviction_to_trit(0.50) == CT_TRIT_ZERO);
    assert(ct_conviction_to_trit(0.70) == CT_TRIT_ZERO);
    assert(ct_conviction_to_trit(0.71) == CT_TRIT_POS);
    assert(ct_conviction_to_trit(1.0)  == CT_TRIT_POS);

    /* Test round-trip */
    assert(ct_trit_to_conviction(CT_TRIT_NEG)  == 0.0);
    assert(ct_trit_to_conviction(CT_TRIT_ZERO) == 0.5);
    assert(ct_trit_to_conviction(CT_TRIT_POS)  == 1.0);

    /* Test NOT */
    assert(ct_not(CT_TRIT_NEG)  == CT_TRIT_POS);
    assert(ct_not(CT_TRIT_ZERO) == CT_TRIT_ZERO);
    assert(ct_not(CT_TRIT_POS)  == CT_TRIT_NEG);

    /* Test AND truth table */
    assert(ct_and(CT_TRIT_NEG, CT_TRIT_NEG) == CT_TRIT_NEG);
    assert(ct_and(CT_TRIT_NEG, CT_TRIT_POS) == CT_TRIT_NEG);
    assert(ct_and(CT_TRIT_POS, CT_TRIT_NEG) == CT_TRIT_NEG);
    assert(ct_and(CT_TRIT_ZERO, CT_TRIT_POS) == CT_TRIT_ZERO);
    assert(ct_and(CT_TRIT_POS, CT_TRIT_POS) == CT_TRIT_POS);

    /* Test OR truth table */
    assert(ct_or(CT_TRIT_NEG, CT_TRIT_POS)  == CT_TRIT_POS);
    assert(ct_or(CT_TRIT_NEG, CT_TRIT_NEG)  == CT_TRIT_NEG);
    assert(ct_or(CT_TRIT_ZERO, CT_TRIT_NEG) == CT_TRIT_ZERO);
    assert(ct_or(CT_TRIT_ZERO, CT_TRIT_POS) == CT_TRIT_POS);

    /* Test XOR */
    assert(ct_xor(CT_TRIT_NEG, CT_TRIT_NEG) == CT_TRIT_ZERO);
    assert(ct_xor(CT_TRIT_NEG, CT_TRIT_POS) == CT_TRIT_POS);
    assert(ct_xor(CT_TRIT_ZERO, CT_TRIT_NEG) == CT_TRIT_NEG);
    assert(ct_xor(CT_TRIT_ZERO, CT_TRIT_ZERO) == CT_TRIT_ZERO);

    /* Test implies */
    assert(ct_implies(CT_TRIT_NEG, CT_TRIT_NEG) == CT_TRIT_POS); /* ¬(-1) ∨ -1 = (+1) ∨ -1 = +1 */
    assert(ct_implies(CT_TRIT_POS, CT_TRIT_NEG) == CT_TRIT_NEG); /* ¬(+1) ∨ -1 = (-1) ∨ -1 = -1 */

    /* Test from_char / to_char round-trip */
    assert(ct_from_char('-') == CT_TRIT_NEG);
    assert(ct_from_char('0') == CT_TRIT_ZERO);
    assert(ct_from_char('+') == CT_TRIT_POS);
    assert(ct_to_char(CT_TRIT_NEG) == '-');
    assert(ct_to_char(CT_TRIT_ZERO) == '0');
    assert(ct_to_char(CT_TRIT_POS) == '+');

    /* Test from_int */
    assert(ct_from_int(-5) == CT_TRIT_NEG);
    assert(ct_from_int(0) == CT_TRIT_ZERO);
    assert(ct_from_int(42) == CT_TRIT_POS);

    printf("All tests passed!\n");
    return 0;
}
