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
/*  Vector Operations (element-wise on trit arrays)                   */
/* ------------------------------------------------------------------ */

/**
 * ct_and_vec — Element-wise ternary AND on two trit vectors.
 * Both a and b must have length n. Result written to out (may alias a or b).
 */
void ct_and_vec(const ct_trit_t *a, const ct_trit_t *b, ct_trit_t *out, int n);

/**
 * ct_or_vec — Element-wise ternary OR on two trit vectors.
 */
void ct_or_vec(const ct_trit_t *a, const ct_trit_t *b, ct_trit_t *out, int n);

/**
 * ct_neg_vec — Element-wise negation of a trit vector.
 */
void ct_neg_vec(const ct_trit_t *vec, ct_trit_t *out, int n);

/**
 * ct_sum — Integer sum of all trits in a vector.
 * Returns Σ vec[i] (as an int, not clamped to trit range).
 */
int ct_sum(const ct_trit_t *vec, int n);

/**
 * ct_dot — Ternary dot product: Σ a[i] * b[i]
 * Returns integer (not clamped to trit range).
 */
int ct_dot(const ct_trit_t *a, const ct_trit_t *b, int n);

/**
 * ct_norm — L1 norm (count of non-zero trits).
 */
int ct_norm(const ct_trit_t *vec, int n);

/**
 * ct_distance — Hamming-style distance: count of positions where a[i] != b[i].
 */
int ct_distance(const ct_trit_t *a, const ct_trit_t *b, int n);

/* ------------------------------------------------------------------ */
/*  I2I Wire Packing — encode/decode trit vectors for fleet transport */
/* ------------------------------------------------------------------ */

/** Maximum trits in a single I2I packet (2^32-1 practically unlimited). */
#define CT_I2I_MAX_TRITS 0x7FFFFFFF

/** I2I version byte (bump on wire format change). */
#define CT_I2I_VERSION 1

/**
 * ct_i2i_header_size — Total header size in bytes for I2I wire format.
 *     [version:1B][trit_count:4B LE] = 5 bytes
 */
#define CT_I2I_HEADER_SIZE 5

/**
 * ct_i2i_payload_size — Compute payload bytes needed for n trits.
 * Each trit = 2 bits → ceil(n * 2 / 8) bytes.
 */
int ct_i2i_payload_size(int n);

/**
 * ct_pack_trits — Pack trit vector into I2I wire format.
 *
 * Wire format:
 *   [version:1B][trit_count:4B LE][payload:ceil(n*2/8)B]
 *
 * Encoding per trit (2 bits, LSB first in each byte):
 *   CT_TRIT_NEG → 00
 *   CT_TRIT_ZERO→ 01
 *   CT_TRIT_POS → 10
 *   2 (reserved)→ 11
 *
 * @param trits    Input trit vector (length n)
 * @param n        Number of trits
 * @param out      Output buffer (must be >= CT_I2I_HEADER_SIZE + ct_i2i_payload_size(n))
 * @param out_len  Size of output buffer
 * @return Total bytes written, or -1 on error.
 */
int ct_pack_trits(const ct_trit_t *trits, int n, uint8_t *out, int out_len);

/**
 * ct_unpack_trits — Unpack I2I wire format back to trit vector.
 *
 * @param data     Input wire bytes
 * @param data_len Length of input bytes
 * @param out      Output trit buffer (must be >= header.trit_count)
 * @param out_len  Size of output buffer
 * @return Number of trits unpacked, or -1 on error.
 */
int ct_unpack_trits(const uint8_t *data, int data_len, ct_trit_t *out, int out_len);

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

/* ------------------------------------------------------------------ */
/*  Vector Operations                                                  */
/* ------------------------------------------------------------------ */

void ct_and_vec(const ct_trit_t *a, const ct_trit_t *b, ct_trit_t *out, int n)
{
    for (int i = 0; i < n; i++) {
        out[i] = ct_and(a[i], b[i]);
    }
}

void ct_or_vec(const ct_trit_t *a, const ct_trit_t *b, ct_trit_t *out, int n)
{
    for (int i = 0; i < n; i++) {
        out[i] = ct_or(a[i], b[i]);
    }
}

void ct_neg_vec(const ct_trit_t *vec, ct_trit_t *out, int n)
{
    for (int i = 0; i < n; i++) {
        out[i] = ct_not(vec[i]);
    }
}

int ct_sum(const ct_trit_t *vec, int n)
{
    int s = 0;
    for (int i = 0; i < n; i++) {
        s += (int)vec[i];
    }
    return s;
}

int ct_dot(const ct_trit_t *a, const ct_trit_t *b, int n)
{
    int s = 0;
    for (int i = 0; i < n; i++) {
        s += (int)a[i] * (int)b[i];
    }
    return s;
}

int ct_norm(const ct_trit_t *vec, int n)
{
    int c = 0;
    for (int i = 0; i < n; i++) {
        if (vec[i] != CT_TRIT_ZERO) c++;
    }
    return c;
}

int ct_distance(const ct_trit_t *a, const ct_trit_t *b, int n)
{
    int d = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) d++;
    }
    return d;
}

/* ------------------------------------------------------------------ */
/*  I2I Wire Packing                                                   */
/* ------------------------------------------------------------------ */

int ct_i2i_payload_size(int n)
{
    if (n < 0) return 0;
    return (n * 2 + 7) / 8;  /* ceil(n*2/8) */
}

int ct_pack_trits(const ct_trit_t *trits, int n, uint8_t *out, int out_len)
{
    if (!trits || !out || n < 0 || n > CT_I2I_MAX_TRITS) return -1;

    int payload = ct_i2i_payload_size(n);
    int total = CT_I2I_HEADER_SIZE + payload;
    if (out_len < total) return -1;

    /* Write header */
    out[0] = CT_I2I_VERSION;
    out[1] = (uint8_t)(n & 0xFF);
    out[2] = (uint8_t)((n >> 8) & 0xFF);
    out[3] = (uint8_t)((n >> 16) & 0xFF);
    out[4] = (uint8_t)((n >> 24) & 0xFF);

    /* Pack trits: 2 bits per trit, LSB first */
    for (int i = 0; i < n; i++) {
        int byte_idx = CT_I2I_HEADER_SIZE + (i / 4);
        int bit_shift = (i % 4) * 2;
        uint8_t bits;
        switch (trits[i]) {
            case CT_TRIT_NEG:  bits = 0x00; break;
            case CT_TRIT_ZERO: bits = 0x01; break;
            case CT_TRIT_POS:  bits = 0x02; break;
            default:           bits = 0x03; break;  /* reserved */
        }
        out[byte_idx] |= (bits << bit_shift);
    }

    return total;
}

int ct_unpack_trits(const uint8_t *data, int data_len, ct_trit_t *out, int out_len)
{
    if (!data || !out) return -1;
    if (data_len < CT_I2I_HEADER_SIZE) return -1;

    /* Check version */
    if (data[0] != CT_I2I_VERSION) return -1;

    /* Read trit count (32-bit LE) */
    int n = (int)data[1]
          | ((int)data[2] << 8)
          | ((int)data[3] << 16)
          | ((int)data[4] << 24);

    if (n < 0 || n > CT_I2I_MAX_TRITS) return -1;
    if (out_len < n) return -1;

    int payload = ct_i2i_payload_size(n);
    if (data_len < CT_I2I_HEADER_SIZE + payload) return -1;

    /* Unpack trits */
    const uint8_t *payload_start = data + CT_I2I_HEADER_SIZE;
    for (int i = 0; i < n; i++) {
        int byte_idx = i / 4;
        int bit_shift = (i % 4) * 2;
        uint8_t bits = (payload_start[byte_idx] >> bit_shift) & 0x03;
        switch (bits) {
            case 0x00: out[i] = CT_TRIT_NEG;  break;
            case 0x01: out[i] = CT_TRIT_ZERO; break;
            case 0x02: out[i] = CT_TRIT_POS;  break;
            default:   out[i] = CT_TRIT_ZERO; break;  /* reserved → ZERO */
        }
    }

    return n;
}

#endif /* C_TERNARY_IMPL */
#endif /* C_TERNARY_H */
