| #  | Value    | Size (bits) | Memory Address            |
|----|---------|------------|---------------------------|
| 1  | gdt     | 48         | `[args16_start + 0]`      |
| 2  | pad     | 16         | `[args16_start + 6]`     | // The padding is added by gcc, not me
| 3  | esp     | 32         | `[args16_start + 8]`     |
| 4  | ret1    | 16         | `[args16_start + 10]`     |
| 9  | ret2    | 16         | `[args16_start + 12]`     |
| 10  | *fun    | 16         | `[args16_start + 14]`     |
| 7  | fun_cs  | 16         | `[args16_start + 16]`     |
| 8  | argc    | 16         | `[args16_start + 18]`     |
| 9  | arg_0   | 16         | `[args16_start + 20]`     |
| 10  | arg_1   | 16         | `[args16_start + 22]`     |
| 11 | arg_2   | 16          | `[args16_start + 24]`     |
| 10 | arg_3   | 16          | `[args16_start + 26]`     |
| 11 | arg_4   | 16          | `[args16_start + 28]`     |
| 12 | arg_5   | 16         | `[args16_start + 28]`     |
| 13 | arg_6   | 16         | `[args16_start + 30]`     |
| 14 | arg_7   | 16         | `[args16_start + 32]`     |
| 15 | arg_8   | 16         | `[args16_start + 34]`     |
| 16 | arg_9   | 16         | `[args16_start + 36]`     |
| 17 | arg_10  | 16         | `[args16_start + 38]`     |
| 18 | arg_11  | 16         | `[args16_start + 40]`     |
| 19 | arg_12  | 16         | `[args16_start + 42]`     |
| 20 | arg_13  | 16         | `[args16_start + 44]`     |

