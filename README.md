# c-ternary.h — C99 Ternary Logic Header Library

**c-ternary.h** is a minimal, zero-dependency, header-only C99 library
for ternary logic — the foundation of the Hybrid Manifold's ternary
ecosystem.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## What is Ternary Logic?

Classical Boolean logic operates on two values: **false (0)** and **true (1)**.
Ternary logic adds a third state — the **Leminal Zone (0)** — representing
uncertainty, neutrality, or "unknown." This maps naturally to systems like
conviction-based reasoning where a belief may be negative, neutral, or
positive.

| Value | Trit | Meaning |
|-------|------|---------|
| -1    | NEG  | Negative / False / Down |
|  0    | ZRO  | Neutral / Unknown / Leminal Zone |
| +1    | POS  | Positive / True / Up |

## API Overview

### Types

```c
typedef int8_t ct_trit_t;

#define CT_TRIT_NEG  ((ct_trit_t)-1)
#define CT_TRIT_ZERO ((ct_trit_t) 0)
#define CT_TRIT_POS  ((ct_trit_t)+1)
```

### Core Functions

| Function | Description |
|----------|-------------|
| `ct_conviction_to_trit()` | Map conviction ∈ [0,1] → trit with Leminal Zone deadband |
| `ct_trit_to_conviction()` | Map trit back to representative conviction |
| `ct_not(t)` | Ternary NOT (invert: -1↔+1, 0 stays 0) |
| `ct_and(a, b)` | Ternary AND (minimum / pessimistic) |
| `ct_or(a, b)` | Ternary OR (maximum / optimistic) |
| `ct_xor(a, b)` | Ternary XOR |
| `ct_implies(a, b)` | Ternary implication (¬a ∨ b) |
| `ct_equals(a, b)` | Check trit equality |

### Utility Functions

| Function | Description |
|----------|-------------|
| `ct_to_char(t)` | Single-char representation: `-`, `0`, `+` |
| `ct_to_str(t, buf)` | 3-letter string: `NEG`, `ZRO`, `POS` |
| `ct_from_char(c)` | Parse char to trit |
| `ct_from_int(v)` | Clamp int to valid trit range |

## Quick Start

### Single-File Example

```c
#define C_TERNARY_IMPL
#include "c-ternary.h"
#include <stdio.h>

int main(void)
{
    /* Map a conviction to a trit */
    double conviction = 0.85;         /* strong positive belief */
    ct_trit_t t = ct_conviction_to_trit(conviction);
    printf("conviction=%.2f → trit=%s\n", conviction, (char[4]){0}, ct_to_str(t, (char[4]){0}));

    /* Ternary logic gates */
    ct_trit_t a = CT_TRIT_POS;
    ct_trit_t b = CT_TRIT_ZERO;

    printf("  NOT %s = %s\n",  (char[4]){0}, ct_to_str(ct_not(a), (char[4]){0}));
    printf("  %s AND %s = %s\n", (char[4]){0}, ct_to_str(ct_and(a, b), (char[4]){0}));
    printf("  %s OR %s = %s\n",  (char[4]){0}, ct_to_str(ct_or(a, b), (char[4]){0}));

    /* Truth-table print */
    printf("\nTernary AND truth table:\n");
    printf("    | -1   0  +1\n");
    printf("----+-----------\n");
    for (ct_trit_t row = CT_TRIT_NEG; row <= CT_TRIT_POS; row++)
    {
        printf(" %s |", (char[4]){0}); ct_to_str(row, (char[4]){0}); printf(" ");
        for (ct_trit_t col = CT_TRIT_NEG; col <= CT_TRIT_POS; col++)
        {
            printf(" %s", (char[4]){0}); ct_to_str(ct_and(row, col), (char[4]){0}); printf(" ");
        }
        printf("\n");
    }

    return 0;
}
```

Compile with any C99-compatible compiler:

```sh
cc -std=c99 -o ternary_demo ternary_demo.c -lm
```

### Multi-File Projects

In exactly **one** `.c` file:

```c
#define C_TERNARY_IMPL
#include "c-ternary.h"
```

All other translation units just include the header normally:

```c
#include "c-ternary.h"
```

## Tuneable Constants

Override these by `#define`-ing before `#include`:

| Constant | Default | Description |
|----------|---------|-------------|
| `CT_LEMINAL_LOW` | 0.30 | Lower bound of the Leminal Zone deadband |
| `CT_LEMINAL_HIGH` | 0.70 | Upper bound of the Leminal Zone deadband |
| `CT_EPSILON` | 1e-9 | Floating-point comparison epsilon |

## Integration

c-ternary.h is the upstream source for all ternary trit operations in the
Hybrid Manifold ecosystem. It is pulled into downstream repos via:

- **plato-engine-block-c** — `plato_engine.h` embeds the TernARY shim
- **savanty** — `ternary_l.py` is the Python translation
- **pincher** — `hybrid-bridge` uses trit gates in the Veto Engine

## License

MIT — see [LICENSE](LICENSE).
