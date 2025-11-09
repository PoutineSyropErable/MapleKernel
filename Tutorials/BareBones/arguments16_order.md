| #  | Value   | Size (bits) |    Memory Address        |
|----|---------|-----------  |--------------------------|
| 1  | gdt     | 48  (6)     | `[args16_start + 0]`     |
| 2  | pad     | 16  (2)     | `[args16_start + 6]`     | // The padding is added by gcc, not me
| 3  | esp     | 32  (4)     | `[args16_start + 8]`     |
| 4  | ss      | 16  (2)     | `[args16_start + 12]`    |
| 5  | ret1    | 16  (2)     | `[args16_start + 14]`    |
| 6  | ret2    | 16  (2)     | `[args16_start + 16]`    |
| 7  | *fun    | 16  (2)     | `[args16_start + 18]`    |
| 8  | fun_cs  | 16  (2)     | `[args16_start + 20]`    |
| 9  | argc    | 16  (2)     | `[args16_start + 22]`    |
| 10 | arg_0   | 16  (2)     | `[args16_start + 24]`    |
| 11 | arg_1   | 16  (2)     | `[args16_start + 26]`    |
| 12 | arg_2   | 16  (2)     | `[args16_start + 28]`    |
| 13 | arg_3   | 16  (2)     | `[args16_start + 30]`    |
| 14 | arg_4   | 16  (2)     | `[args16_start + 32]`    |
| 15 | arg_5   | 16  (2)     | `[args16_start + 34]`    |
| 16 | arg_6   | 16  (2)     | `[args16_start + 36]`    |
| 17 | arg_7   | 16  (2)     | `[args16_start + 38]`    |
| 18 | arg_8   | 16  (2)     | `[args16_start + 40]`    |
| 19 | arg_9   | 16  (2)     | `[args16_start + 42]`    |
| 20 | arg_10  | 16  (2)     | `[args16_start + 44]`    |
| 21 | arg_11  | 16  (2)     | `[args16_start + 46]`    |

