/*
 * c-ternary.h — A minimal C99 header-only library for ternary logic.
 *
 * Single-header, zero dependencies, no dynamic allocation.
 * Defines a trit type (Ternary digIT) occupying values {-1, 0, +1} in an int8_t.
 *
 * Usage (in exactly ONE .c file):
 *     #define C_TERNARY_IMPL
 *     #include "c-ternary.h"
 * Every other translation unit just needs the #include.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef C_TERNARY_H
#define C_TERNARY_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/*  Tuneable defaults (override before #include)                      */
/* ------------------------------------------------------------------ */

/** Lower bound of the Leminal Zone deadband. */
#ifndef CT_LEMINAL_LOW
#define CT_LEMINAL_LOW   0.30
#endif

/** Upper bound of the Leminal Zone deadband. */
#ifndef CT_LEMINAL_HIGH
#define CT_LEMINAL_HIGH  0.70
#endif

/** Epsilon for floating-point comparison. */
#ifndef CT_EPSILON
#define CT_EPSILON       1e-9
#endif

/* ------------------------------------------------------------------ */
/*  Types                                                              */
/* ------------------------------------------------------------------ */

/**
 * ct_trit_t — A ternary digit (trit) with three states.
 *
 *   CT_TRIT_NEG  = -1  (false / negative / down)
 *   CT_TRIT_ZERO =  0  (null / unknown / neutral / Leminal Zone)
 *   CT_TRIT_POS  = +1  (true / positive / up)
 */
typedef int8_t ct_trit_t;

#define CT_TRIT_NEG  ((ct_trit_t)-1)
#define CT_TRIT_ZERO ((ct_trit_t) 0)
#define CT_TRIT_POS  ((ct_trit_t)+1)

/* ------------------------------------------------------------------ */
/*  Core API                                                          */
/* ------------------------------------------------------------------ */

/**
 * ct_conviction_to_trit — Map a conviction ∈ [0,1] to a trit.
 *
 *   [0.0, CT_LEMINAL_LOW)  → CT_TRIT_NEG
 *   [CT_LEMINAL_LOW, CT_LEMINAL_HIGH] → CT_TRIT_ZERO  (Leminal Zone)
 *   (CT_LEMINAL_HIGH, 1.0] → CT_TRIT_POS
 *
 * Values outside [0,1] are clamped.
 */
ct_trit_t ct_conviction_to_trit(double conviction);

/**
 * ct_trit_to_conviction — Map a trit back to a representative conviction.
 *
 *   CT_TRIT_NEG  → 0.0
 *   CT_TRIT_ZERO → 0.5
 *   CT_TRIT_POS  → 1.0
 */
double ct_trit_to_conviction(ct_trit_t t);

/**
 * ct_not — Ternary NOT (inversion).
 *
 *   ¬(-1) = +1
 *   ¬( 0) =  0
 *   ¬(+1) = -1
 */
ct_trit_t ct_not(ct_trit_t t);

/**
 * ct_and — Ternary AND (minimum / pessimistic).
 *
 *   a ∧ b = min(a, b)  in the ordering  -1 < 0 < +1
 *
 * Truth table (a \ b):
 *       | -1   0  +1
 *   ----+-----------
 *   -1  | -1  -1  -1
 *    0  | -1   0   0
 *   +1  | -1   0  +1
 */
ct_trit_t ct_and(ct_trit_t a, ct_trit_t b);

/**
 * ct_or — Ternary OR (maximum / optimistic).
 *
 *   a ∨ b = max(a, b)  in the ordering  -1 < 0 < +1
 *
 * Truth table (a \ b):
 *       | -1   0  +1
 *   ----+-----------
 *   -1  | -1   0  +1
 *    0  |  0   0  +1
 *   +1  | +1  +1  +1
 */
ct_trit_t ct_or(ct_trit_t a, ct_trit_t b);

/**
 * ct_xor — Ternary XOR.
 *
 *   a ⊕ b:
 *     Same values → CT_TRIT_ZERO
 *     Different non-zero values → CT_TRIT_POS
 *     Zero with anything → the non-zero value
 *
 * Truth table (a \ b):
 *       | -1   0  +1
 *   ----+-----------
 *   -1  |  0  -1  +1
 *    0  | -1   0  +1
 *   +1  | +1  +1   0
 */
ct_trit_t ct_xor(ct_trit_t a, ct_trit_t b);

/**
 * ct_implies — Ternary implication (a → b).
 *
 *   a → b = ¬a ∨ b
 *
 * Truth table (a \ b):
 *       | -1   0  +1
 *   ----+-----------
 *   -1  | +1  +1  +1
 *    0  | -1   0  +1
 *   +1  | -1   0  +1
 */
ct_trit_t ct_implies(ct_trit_t a, ct_trit_t b);

/**
 * ct_equals — Check if two trits are equal.
 */
bool ct_equals(ct_trit_t a, ct_trit_t b);

/**
 * ct_from_int — Safely cast an int to ct_trit_t, clamping to valid range.
 */
ct_trit_t ct_from_int(int v);

/* ------------------------------------------------------------------ */
/*  Utility / Pretty-printing                                         */
/* ------------------------------------------------------------------ */

/**
 * ct_to_char — Return a single-character representation.
 *
 *   CT_TRIT_NEG  → '-'
 *   CT_TRIT_ZERO → '0'
 *   CT_TRIT_POS  → '+'
 *   anything else → '?'
 */
char ct_to_char(ct_trit_t t);

/**
 * ct_to_str — Write a human-readable string into buf[4].
 *
 *   CT_TRIT_NEG  → "NEG"
 *   CT_TRIT_ZERO → "ZRO"
 *   CT_TRIT_POS  → "POS"
 *   anything else → "???"
 */
void ct_to_str(ct_trit_t t, char buf[4]);

/**
 * ct_from_char — Parse a single character to a trit.
 *
 *   '-', 'N', 'n' → CT_TRIT_NEG
 *   '0', 'Z', 'z' → CT_TRIT_ZERO
 *   '+', 'P', 'p' → CT_TRIT_POS
 *   anything else  → CT_TRIT_ZERO
 */
ct_trit_t ct_from_char(char c);

#ifdef __cplusplus
}
#endif

/* ================================================================== */
/*  Implementation                                                    */
/* ================================================================== */
#ifdef C_TERNARY_IMPL

ct_trit_t ct_conviction_to_trit(double conviction)
{
    if (conviction < 0.0) conviction = 0.0;
    if (conviction > 1.0) conviction = 1.0;
    if (conviction < CT_LEMINAL_LOW)  return CT_TRIT_NEG;
    if (conviction > CT_LEMINAL_HIGH) return CT_TRIT_POS;
    return CT_TRIT_ZERO;
}

double ct_trit_to_conviction(ct_trit_t t)
{
    switch (t)
    {
    case CT_TRIT_NEG:  return 0.0;
    case CT_TRIT_POS:  return 1.0;
    default:           return 0.5;
    }
}

ct_trit_t ct_not(ct_trit_t t)
{
    switch (t)
    {
    case CT_TRIT_NEG:  return CT_TRIT_POS;
    case CT_TRIT_POS:  return CT_TRIT_NEG;
    default:           return CT_TRIT_ZERO;
    }
}

ct_trit_t ct_and(ct_trit_t a, ct_trit_t b)
{
    return (a < b) ? a : b;
}

ct_trit_t ct_or(ct_trit_t a, ct_trit_t b)
{
    return (a > b) ? a : b;
}

ct_trit_t ct_xor(ct_trit_t a, ct_trit_t b)
{
    if (a == CT_TRIT_ZERO) return b;
    if (b == CT_TRIT_ZERO) return a;
    if (a == b)            return CT_TRIT_ZERO;
    return CT_TRIT_POS;
}

ct_trit_t ct_implies(ct_trit_t a, ct_trit_t b)
{
    return ct_or(ct_not(a), b);
}

bool ct_equals(ct_trit_t a, ct_trit_t b)
{
    return a == b;
}

ct_trit_t ct_from_int(int v)
{
    if (v <= -1) return CT_TRIT_NEG;
    if (v >=  1) return CT_TRIT_POS;
    return CT_TRIT_ZERO;
}

char ct_to_char(ct_trit_t t)
{
    switch (t)
    {
    case CT_TRIT_NEG:  return '-';
    case CT_TRIT_ZERO: return '0';
    case CT_TRIT_POS:  return '+';
    default:           return '?';
    }
}

void ct_to_str(ct_trit_t t, char buf[4])
{
    switch (t)
    {
    case CT_TRIT_NEG:  memcpy(buf, "NEG", 4); break;
    case CT_TRIT_ZERO: memcpy(buf, "ZRO", 4); break;
    case CT_TRIT_POS:  memcpy(buf, "POS", 4); break;
    default:           memcpy(buf, "???", 4); break;
    }
}

ct_trit_t ct_from_char(char c)
{
    switch (c)
    {
    case '-': case 'N': case 'n': return CT_TRIT_NEG;
    case '+': case 'P': case 'p': return CT_TRIT_POS;
    default:                      return CT_TRIT_ZERO;
    }
}

#endif /* C_TERNARY_IMPL */
#endif /* C_TERNARY_H */
