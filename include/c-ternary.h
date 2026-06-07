/**
 * c-ternary.h — A Pure C99 Ternary State Machine Library
 *
 * Part of the Hybrid Manifold ecosystem (SuperInstance).
 * Provides balanced ternary logic (+1, 0, -1) with continuous conviction
 * fusion and a "Leminal Zone" deadband for safe boundary detection.
 *
 * Key Concepts:
 * - Trit: A ternary digit in {+1 (TRUE), 0 (UNCERTAIN), -1 (FALSE)}.
 * - Conviction: A continuous weight [0.0, 1.0] indicating confidence.
 * - Leminal Zone: When conviction falls in (0.30, 0.70), the trit maps to
 *   0 (UNCERTAIN), preventing premature commitment in ambiguous regions.
 * - Veto: A governance signal that overrides execution (4-tier SAEP hierarchy).
 *
 * Usage:
 *   #include "c-ternary.h"
 *   trit_t gate = ternary_from_conviction(0.85);  // => +1
 *   if (ternary_is_certain(gate)) { ... }
 *
 * License: MIT (see LICENSE file)
 * Author: SuperInstance Fleet / Oracle2
 */

#ifndef C_TERNARY_H
#define C_TERNARY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

/* ─── Core Types ─────────────────────────────────────────────── */

/** @brief Balanced ternary trit: +1 (TRUE), 0 (UNCERTAIN), -1 (FALSE). */
typedef int8_t trit_t;

#define TRIT_TRUE       ((trit_t) 1)   /**< Positive certainty. */
#define TRIT_UNCERTAIN  ((trit_t) 0)   /**< Ambiguous / deadband. */
#define TRIT_FALSE      ((trit_t)-1)   /**< Negative certainty. */

/** @brief A continuous conviction weight fused with its trit gate. */
typedef struct {
    double    conviction;  /**< Confidence in [0.0, 1.0]. */
    trit_t    gate;        /**< Quantized ternary state. */
} ternary_l_t;

/** @brief A 4-tier SAEP veto signal. */
typedef enum {
    VETO_NONE    = 0,  /**< No veto; execution proceeds. */
    VETO_ROOM    = 1,  /**< Local agent-room veto. */
    VETO_SECTOR  = 2,  /**< Sector-level governance. */
    VETO_PORT    = 3,  /**< Portfolio-level constraint. */
    VETO_MARKET  = 4   /**< Global market/org veto. */
} veto_tier_t;

/** @brief A symmetry marker for topological state tracking. */
typedef struct {
    uint64_t orbit_id;       /**< Symmetry orbit identifier. */
    double   wasserstein;    /**< Distance from expected symmetry [0, ∞). */
    trit_t   is_symmetric;   /**< TRIT_TRUE if symmetric within tolerance. */
} symmetry_marker_t;

/* ─── Configurable Parameters ────────────────────────────────── */

/** @brief Default Leminal Zone bounds. Conviction in (LZ_LOW, LZ_HIGH) maps to 0. */
#ifndef C_TERNARY_LZ_LOW
#define C_TERNARY_LZ_LOW  0.30
#endif

#ifndef C_TERNARY_LZ_HIGH
#define C_TERNARY_LZ_HIGH 0.70
#endif

/** @brief Default Wasserstein tolerance for symmetry detection. */
#ifndef C_TERNARY_SYMMETRY_TOL
#define C_TERNARY_SYMMETRY_TOL 0.05
#endif

/* ─── Ternary Gate Operations ────────────────────────────────── */

/**
 * @brief Convert a continuous conviction [0,1] to a trit gate.
 *
 * Maps to TRIT_TRUE if conviction >= LZ_HIGH,
 * TRIT_FALSE if conviction <= LZ_LOW,
 * TRIT_UNCERTAIN otherwise (Leminal Zone).
 *
 * @param conviction Confidence value (clamped to [0,1]).
 * @return trit_t Quantized ternary decision.
 */
static inline trit_t ternary_from_conviction(double conviction) {
    if (conviction < 0.0) conviction = 0.0;
    if (conviction > 1.0) conviction = 1.0;

    if (conviction >= C_TERNARY_LZ_HIGH) return TRIT_TRUE;
    if (conviction <= C_TERNARY_LZ_LOW)  return TRIT_FALSE;
    return TRIT_UNCERTAIN;
}

/**
 * @brief Create a fused ternary_l_t from a raw conviction.
 */
static inline ternary_l_t ternary_l_make(double conviction) {
    ternary_l_t result;
    result.conviction = (conviction < 0.0 ? 0.0 : (conviction > 1.0 ? 1.0 : conviction));
    result.gate       = ternary_from_conviction(result.conviction);
    return result;
}

/** @brief Check if a trit is in a certain (non-zero) state. */
static inline bool ternary_is_certain(trit_t t) {
    return t != TRIT_UNCERTAIN;
}

/** @brief Check if a trit is TRUE (+1). */
static inline bool ternary_is_true(trit_t t) {
    return t == TRIT_TRUE;
}

/** @brief Check if a trit is FALSE (-1). */
static inline bool ternary_is_false(trit_t t) {
    return t == TRIT_FALSE;
}

/* ─── Ternary Logic ──────────────────────────────────────────── */

/**
 * @brief Ternary AND: returns the minimum (most negative) of two trits.
 *
 * Truth table:
 *   AND  | +1   0  -1
 *   -----+------------
 *   +1   | +1   0  -1
 *   0    |  0   0  -1
 *   -1   | -1  -1  -1
 */
static inline trit_t ternary_and(trit_t a, trit_t b) {
    return (a < b) ? a : b;
}

/**
 * @brief Ternary OR: returns the maximum (most positive) of two trits.
 *
 * Truth table:
 *   OR   | +1   0  -1
 *   -----+------------
 *   +1   | +1  +1  +1
 *   0    | +1   0   0
 *   -1   | +1   0  -1
 */
static inline trit_t ternary_or(trit_t a, trit_t b) {
    return (a > b) ? a : b;
}

/**
 * @brief Ternary NOT: negate a trit.
 *   NOT(+1) = -1, NOT(0) = 0, NOT(-1) = +1
 */
static inline trit_t ternary_not(trit_t t) {
    return (trit_t)(-t);
}

/**
 * @brief Fuse two ternary_l_t values, combining conviction and gating.
 *
 * Conviction is averaged; gate is re-derived from the average.
 */
static inline ternary_l_t ternary_l_fuse(ternary_l_t a, ternary_l_t b) {
    return ternary_l_make((a.conviction + b.conviction) * 0.5);
}

/**
 * @brief Convert a trit to a human-readable character.
 *   '+': TRUE, '0': UNCERTAIN, '-': FALSE
 */
static inline char ternary_to_char(trit_t t) {
    switch (t) {
        case TRIT_TRUE:      return '+';
        case TRIT_UNCERTAIN: return '0';
        case TRIT_FALSE:     return '-';
        default:             return '?';
    }
}

/* ─── Veto Operations ────────────────────────────────────────── */

/**
 * @brief A 4-tier SAEP veto signal with context.
 */
typedef struct {
    veto_tier_t tier;        /**< Highest violating tier. */
    bool        is_vetoed;   /**< True if execution should halt. */
    const char *reason;      /**< Human-readable reason (may be NULL). */
} veto_signal_t;

/** @brief Create a "no veto" signal (execution proceeds). */
static inline veto_signal_t veto_pass(void) {
    veto_signal_t v = { VETO_NONE, false, NULL };
    return v;
}

/** @brief Create a veto at the given tier with a reason. */
static inline veto_signal_t veto_at(veto_tier_t tier, const char *reason) {
    veto_signal_t v = { tier, (tier > VETO_NONE), reason };
    return v;
}

/** @brief Combine two vetoes: the harsher (higher tier) wins. */
static inline veto_signal_t veto_combine(veto_signal_t a, veto_signal_t b) {
    if (b.tier > a.tier) return b;
    return a;
}

/* ─── Symmetry Markers ───────────────────────────────────────── */

/**
 * @brief Evaluate whether a Wasserstein distance is within symmetry tolerance.
 *
 * @param distance Computed Wasserstein distance.
 * @return trit_t TRIT_TRUE if symmetric, TRIT_FALSE if not, TRIT_UNCERTAIN if borderline.
 */
static inline trit_t symmetry_check(double distance) {
    if (distance < 0.0) distance = -distance;
    if (distance < C_TERNARY_SYMMETRY_TOL * 0.5) return TRIT_TRUE;
    if (distance > C_TERNARY_SYMMETRY_TOL * 2.0) return TRIT_FALSE;
    return TRIT_UNCERTAIN;
}

/**
 * @brief Create a symmetry marker for an orbit.
 *
 * @param orbit_id   The orbit identifier.
 * @param distance   Wasserstein distance from expected symmetry.
 * @return symmetry_marker_t
 */
static inline symmetry_marker_t symmetry_marker(uint64_t orbit_id, double distance) {
    symmetry_marker_t m;
    m.orbit_id       = orbit_id;
    m.wasserstein    = distance;
    m.is_symmetric   = symmetry_check(distance);
    return m;
}

/* ─── Utility / Debugging ────────────────────────────────────── */

/**
 * @brief Format a ternary_l_t as a string into a provided buffer.
 *
 * Buffer should be at least 32 bytes. Format: "{conviction} [{gate_char}]"
 */
static inline void ternary_l_format(ternary_l_t t, char *buf, size_t buf_size) {
    if (buf_size < 8) return;
    snprintf(buf, buf_size, "%.3f [%c]", t.conviction, ternary_to_char(t.gate));
}

/**
 * @brief Format a veto_signal_t as a string.
 */
static inline void veto_format(veto_signal_t v, char *buf, size_t buf_size) {
    static const char *tier_names[] = {
        "NONE", "ROOM", "SECTOR", "PORTFOLIO", "MARKET"
    };
    if (buf_size < 16) return;
    if (v.is_vetoed) {
        snprintf(buf, buf_size, "VETOED@%s: %s",
                 (size_t)v.tier <= 4 ? tier_names[v.tier] : "???",
                 v.reason ? v.reason : "no reason");
    } else {
        snprintf(buf, buf_size, "PASS");
    }
}

#endif /* C_TERNARY_H */
