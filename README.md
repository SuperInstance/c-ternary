# c-ternary — Balanced Ternary Substrate for SuperInstance Fleet

A production C99 header providing balanced ternary `{-1, 0, +1}` arithmetic, deadband conversion, symmetry detection, and SAEP veto hierarchy for embedded and systems-level fleet nodes.

## Why Balanced Ternary?

Balanced ternary is the natural number system for systems that reason about **opposition and neutrality** — not just on/off. Every fleet agent, sensor, and decision point expresses three states:

| Trit | Meaning | Examples |
|------|---------|---------|
| `-1` | Negative / Contra / Reject | Sensor alarm, agent veto, sell signal |
| `0` | Neutral / Lemniscal / Deadband | Sensor nominal, agent idle, hold |
| `+1` | Positive / Pro / Accept | Sensor clear, agent approve, buy signal |

The conservation law `Σ(ternary) = 0` holds for any balanced fleet gesture — a chord, a decision, a sensor array.

## Architecture

```
int/double ──→ ternary_t ──→ ternary_vec_t ──→ conservation check
                    │              │
            deadband zone    hamming distance
                    │              │
               veto_level     symmetry_check
```

### Deadband / Lemniscal Zone

The critical insight: **not all continuous values should map to ±1**. The range `[0.3, 0.7]` is the "lemniscal zone" — values here are ambiguous and map to `0` (neutral). This prevents sensor jitter and agent oscillation from propagating as decisions.

```
 -1.0    -0.3     0     0.3    0.7     1.0
  ────────┤  NEG  ├───────┤ ZERO ├───────┤  POS  ├────────
          ↑ deadband boundaries (configurable)
```

### SAEP Veto Hierarchy

Four-level veto chain from `pincher`'s SAEP architecture:

```
ROOM → SECTOR → PORTFOLIO → MARKET
  0       1         2          3
```

Any `TERNARY_NEG` at a level ≤ the max checked level triggers a veto. This is how plato-* alarm blocks decide whether to escalate.

## Quick Start

```c
#include "c-ternary.h"

/* Basic conversion */
ternary_t t = ternary_from_double(0.85);   // → +1 (above deadband)
ternary_t u = ternary_from_double(0.5);    // →  0 (lemniscal zone)
ternary_t v = ternary_from_double(0.1);    // → -1 (below deadband)

/* Vector conservation check */
ternary_vec_t chord = ternary_vec_new(4);
ternary_vec_set(&chord, 0, TERNARY_POS);   // major 3rd
ternary_vec_set(&chord, 1, TERNARY_POS);   // perfect 5th
ternary_vec_set(&chord, 2, TERNARY_NEG);   // minor 7th
ternary_vec_set(&chord, 3, TERNARY_NEG);   // root octave
int conserved = ternary_vec_is_balanced(&chord);  // 1 = yes

/* SAEP veto check */
ternary_t levels[4] = {TERNARY_POS, TERNARY_NEG, TERNARY_POS, TERNARY_POS};
int vetoed = ternary_veto_propagate(levels, TERNARY_VETO_PORTFOLIO);  // 1 (SECTOR veto)
```

## API Reference

### Type Conversion
| Function | Description |
|----------|-------------|
| `ternary_from_int(int)` | Clamp to {-1, 0, +1} |
| `ternary_from_double(double)` | Default deadband [0.3, 0.7] |
| `ternary_with_deadband(double, lo, hi)` | Custom deadband |
| `ternary_to_int(ternary_t)` | → int |
| `ternary_to_double(ternary_t)` | → double |
| `ternary_valid(ternary_t)` | Check valid range |

### Arithmetic
| Function | Description |
|----------|-------------|
| `ternary_negate(t)` | -1↔+1, 0→0 |
| `ternary_add(a, b, &carry)` | Balanced ternary addition |
| `ternary_multiply(a, b)` | Sign multiplication |

### Vectors
| Function | Description |
|----------|-------------|
| `ternary_vec_new(n)` | Allocate n-element vector (zeroed) |
| `ternary_vec_free(&v)` | Deallocate |
| `ternary_vec_get/set` | Element access |
| `ternary_vec_sum(v)` | Σ elements |
| `ternary_vec_conservation(v)` | Same as sum (naming alias) |
| `ternary_vec_is_balanced(v)` | sum == 0? |

### Symmetry
| Function | Description |
|----------|-------------|
| `ternary_symmetry_check(data, n)` | XOR mask for topological identity |
| `ternary_hamming(a, b, n)` | Hamming distance |
| `ternary_vec_hamming(va, vb)` | Vec Hamming distance |

### Veto (SAEP)
| Function | Description |
|----------|-------------|
| `ternary_veto_check(level, value)` | Single-level check |
| `ternary_veto_propagate(levels[], max)` | Hierarchy check |

### String
| Function | Description |
|----------|-------------|
| `ternary_to_char(t)` | → '-', '0', or '+' |
| `ternary_vec_to_str(v, buf, size)` | → "-0+-+" |

## Building

```bash
make test    # Build and run tests
make clean   # Remove build artifacts
```

No dependencies beyond C99 standard library and `-lm`.

## Integration with Fleet

This header unblocks these SuperInstance repos:
- **plato-block-a** — Ternary sensor fusion
- **plato-engine-block-c** — Ternary alarm state machine
- **plato-block-i** — Ternary I2C interface
- **plato-engine-block-s** — Ternary system control

## License

MIT
