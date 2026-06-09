/**
 * c-ternary.h — Balanced Ternary Substrate for SuperInstance Fleet
 *
 * C99 header providing {-1, 0, +1} trit gates, deadband conversion,
 * symmetry detection, and SAEP veto hierarchy for plato-* blocks.
 *
 * Architecture:
 *   int/double ──→ ternary_t ──→ ternary_vec_t ──→ conservation check
 *                      │              │
 *              deadband zone    hamming distance
 *                      │              │
 *                 veto_level     symmetry_check
 *
 * SPDX-License-Identifier: MIT
 * SuperInstance/c-ternary — https://github.com/SuperInstance/c-ternary
 */

#ifndef C_TERNARY_H
#define C_TERNARY_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Ternary Type ──────────────────────────────────────────────────────── */

/** Core ternary value: negative (-1), zero (0), or positive (+1). */
typedef int ternary_t;

#define TERNARY_NEG  (-1)
#define TERNARY_ZERO ( 0)
#define TERNARY_POS  (+1)

/* ─── Deadband / Lemniscal Zone ─────────────────────────────────────────── */

/** Default deadband boundaries for float→ternary conversion.
 *  Values in [0.3, 0.7] map to TERNARY_ZERO (lemniscal zone). */
#define TERNARY_DEADBAND_LOW  0.3
#define TERNARY_DEADBAND_HIGH 0.7

/* ─── SAEP Veto Levels ─────────────────────────────────────────────────── */

/** Veto hierarchy: Room → Sector → Portfolio → Market.
 *  Higher levels override lower. Used in plato-* alarm systems. */
typedef enum {
    TERNARY_VETO_ROOM     = 0,  /* Single sensor / agent */
    TERNARY_VETO_SECTOR   = 1,  /* Group of related sensors */
    TERNARY_VETO_PORTFOLIO = 2, /* Cross-sector aggregation */
    TERNARY_VETO_MARKET   = 3,  /* Global system override */
} ternary_veto_level;

#define TERNARY_VETO_COUNT 4

/* ─── Ternary Vector ───────────────────────────────────────────────────── */

/** Fixed-size array of ternary values with conservation tracking. */
typedef struct {
    ternary_t *data;
    size_t     len;
} ternary_vec_t;

/* ─── Inline Ternary Arithmetic ────────────────────────────────────────── */

/** Convert integer to ternary (clamp to -1/0/+1). */
static inline ternary_t ternary_from_int(int v) {
    if (v > 0) return TERNARY_POS;
    if (v < 0) return TERNARY_NEG;
    return TERNARY_ZERO;
}

/** Convert double to ternary with default deadband [0.3, 0.7]. */
static inline ternary_t ternary_from_double(double v) {
    if (v >= TERNARY_DEADBAND_HIGH) return TERNARY_POS;
    if (v <= TERNARY_DEADBAND_LOW)  return TERNARY_NEG;
    return TERNARY_ZERO;
}

/** Convert double with custom deadband boundaries. */
static inline ternary_t ternary_with_deadband(double v, double lo, double hi) {
    if (v >= hi) return TERNARY_POS;
    if (v <= lo) return TERNARY_NEG;
    return TERNARY_ZERO;
}

/** Negate: {-1→+1, 0→0, +1→-1}. */
static inline ternary_t ternary_negate(ternary_t t) {
    return -t;
}

/** Balanced ternary addition with carry: -1+1=0, -1+-1=-1 carry -1, etc.
 *  Returns the sum trit; stores carry in *carry if non-NULL. */
static inline ternary_t ternary_add(ternary_t a, ternary_t b, int *carry) {
    int sum = a + b;
    if (carry) *carry = 0;
    if (sum > 1)  { if (carry) *carry = 1;  return TERNARY_NEG; } /* +1 + +1 = -1 carry +1 */
    if (sum < -1) { if (carry) *carry = -1; return TERNARY_POS; } /* -1 + -1 = +1 carry -1 */
    return (ternary_t)sum;
}

/** Ternary multiplication: standard sign rules. */
static inline ternary_t ternary_multiply(ternary_t a, ternary_t b) {
    return a * b;
}

/** Convert ternary back to int. */
static inline int ternary_to_int(ternary_t t) {
    return t;
}

/** Convert ternary to double (-1.0, 0.0, +1.0). */
static inline double ternary_to_double(ternary_t t) {
    return (double)t;
}

/** Check if value is a valid ternary. */
static inline int ternary_valid(ternary_t t) {
    return t >= -1 && t <= 1;
}

/* ─── Ternary Vector Operations ────────────────────────────────────────── */

/** Allocate a ternary vector of n elements, initialized to TERNARY_ZERO. */
static inline ternary_vec_t ternary_vec_new(size_t n) {
    ternary_vec_t v;
    v.len = n;
    v.data = (ternary_t *)calloc(n, sizeof(ternary_t));
    return v;
}

/** Free a ternary vector. */
static inline void ternary_vec_free(ternary_vec_t *v) {
    if (v && v->data) {
        free(v->data);
        v->data = NULL;
        v->len = 0;
    }
}

/** Get element at index. Returns 0 on out-of-bounds. */
static inline ternary_t ternary_vec_get(const ternary_vec_t *v, size_t i) {
    if (!v || !v->data || i >= v->len) return TERNARY_ZERO;
    return v->data[i];
}

/** Set element at index. No-op on out-of-bounds. */
static inline void ternary_vec_set(ternary_vec_t *v, size_t i, ternary_t t) {
    if (v && v->data && i < v->len) v->data[i] = t;
}

/** Sum of all elements. Conservation law: Σ = 0 for balanced ternary. */
static inline int ternary_vec_sum(const ternary_vec_t *v) {
    int sum = 0;
    if (!v || !v->data) return 0;
    for (size_t i = 0; i < v->len; i++) sum += v->data[i];
    return sum;
}

/** Conservation value (same as sum). Zero means perfectly conserved. */
static inline int ternary_vec_conservation(const ternary_vec_t *v) {
    return ternary_vec_sum(v);
}

/** Check if vector is balanced (sum == 0). */
static inline int ternary_vec_is_balanced(const ternary_vec_t *v) {
    return ternary_vec_sum(v) == 0;
}

/* ─── Symmetry Detection ───────────────────────────────────────────────── */

/** XOR mask of all ternary values — used for topological identity.
 *  Two vectors with same XOR mask have the same parity structure. */
static inline int ternary_symmetry_check(const ternary_t *data, size_t n) {
    int mask = 0;
    for (size_t i = 0; i < n; i++) mask ^= data[i];
    return mask;
}

/** Hamming distance between two ternary vectors (count of differing positions).
 *  Returns -1 if lengths differ. */
static inline int ternary_hamming(const ternary_t *a, const ternary_t *b, size_t n) {
    int dist = 0;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) dist++;
    }
    return dist;
}

/** Hamming distance for ternary_vec_t. Returns -1 if lengths differ. */
static inline int ternary_vec_hamming(const ternary_vec_t *a, const ternary_vec_t *b) {
    if (!a || !b || a->len != b->len) return -1;
    return ternary_hamming(a->data, b->data, a->len);
}

/* ─── SAEP Veto ────────────────────────────────────────────────────────── */

/** Check if a ternary value triggers a veto at the given level.
 *  TERNARY_NEG at any level means "veto" (reject/stop).
 *  Returns 1 if vetoed, 0 if allowed. */
static inline int ternary_veto_check(ternary_veto_level level, ternary_t value) {
    (void)level;
    return value == TERNARY_NEG;
}

/** Propagate veto through hierarchy.
 *  If any level at or below `max_level` returns TERNARY_NEG, the result is veto.
 *  levels[] must have TERNARY_VETO_COUNT elements (one per veto level). */
static inline int ternary_veto_propagate(const ternary_t levels[], ternary_veto_level max_level) {
    for (int i = 0; i <= (int)max_level && i < TERNARY_VETO_COUNT; i++) {
        if (levels[i] == TERNARY_NEG) return 1;
    }
    return 0;
}

/* ─── Ternary-to-String ────────────────────────────────────────────────── */

/** Single trit to char: -1→'-', 0→'0', +1→'+'. */
static inline char ternary_to_char(ternary_t t) {
    if (t == TERNARY_POS) return '+';
    if (t == TERNARY_NEG) return '-';
    return '0';
}

/** Write ternary vector as string like "-0+-0+". Caller provides buffer.
 *  Returns number of chars written (not including null terminator). */
static inline size_t ternary_vec_to_str(const ternary_vec_t *v, char *buf, size_t buf_size) {
    if (!v || !buf || buf_size == 0) return 0;
    size_t n = v->len < buf_size - 1 ? v->len : buf_size - 1;
    for (size_t i = 0; i < n; i++) {
        buf[i] = ternary_to_char(v->data[i]);
    }
    buf[n] = '\0';
    return n;
}

#ifdef __cplusplus
}
#endif

#endif /* C_TERNARY_H */
