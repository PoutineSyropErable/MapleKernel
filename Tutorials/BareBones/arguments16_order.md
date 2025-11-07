| #  | Value    | Size (bits) | Memory Address            |
|----|---------|------------|---------------------------|
| 1  | gdt     | 80         | `[args16_start + 0]`      |
| 2  | esp     | 32         | `[args16_start + 10]`     |
| 3  | ret1    | 16         | `[args16_start + 14]`     |
| 4  | ret2    | 16         | `[args16_start + 16]`     |
| 5  | *fun    | 16         | `[args16_start + 18]`     |
| 6  | fun_cs  | 16         | `[args16_start + 20]`     |
| 7  | argc    | 16         | `[args16_start + 22]`     |
| 8  | arg_0   | 16         | `[args16_start + 24]`     |
| 9  | arg_1   | 16         | `[args16_start + 26]`     |
| 10 | arg_2   | 16         | `[args16_start + 28]`     |
| 11 | arg_3   | 16         | `[args16_start + 30]`     |
| 12 | arg_4   | 16         | `[args16_start + 32]`     |
| 13 | arg_5   | 16         | `[args16_start + 34]`     |
| 14 | arg_6   | 16         | `[args16_start + 36]`     |
| 15 | arg_7   | 16         | `[args16_start + 38]`     |
| 16 | arg_8   | 16         | `[args16_start + 40]`     |
| 17 | arg_9   | 16         | `[args16_start + 42]`     |
| 18 | arg_10  | 16         | `[args16_start + 44]`     |
| 19 | arg_11  | 16         | `[args16_start + 46]`     |

