| #  | Value   | Size (bits) |    Memory Address        |
|----|---------|-----------  |--------------------------|
| 1  | gdt     | 48  (6)     | `[args16_start + 0]`     |
| 2  | pad     | 16  (2)     | `[args16_start + 6]`     | // The padding is added by gcc, not me
| 3  | esp     | 32  (4)     | `[args16_start + 8]`     |
| 4  | ss      | 16  (2)     | `[args16_start + 12]`    |
| 4  | cs      | 16  (2)     | `[args16_start + 14]`    |
| 5  | ret1    | 16  (2)     | `[args16_start + 16]`    |
| 8  | ret2    | 16  (2)     | `[args16_start + 18]`    |
| 9  | *fun    | 16  (2)     | `[args16_start + 20]`    |
| 10 | fun_cs  | 16  (2)     | `[args16_start + 22]`    |
| 11 | argc    | 16  (2)     | `[args16_start + 24]`    |
| 12 | arg_0   | 16  (2)     | `[args16_start + 26]`    |
| 13 | arg_1   | 16  (2)     | `[args16_start + 28]`    |
| 14 | arg_2   | 16  (2)     | `[args16_start + 30]`    |
| 15 | arg_3   | 16  (2)     | `[args16_start + 32]`    |
| 16 | arg_4   | 16  (2)     | `[args16_start + 34]`    |
| 17 | arg_5   | 16  (2)     | `[args16_start + 36]`    |
| 18 | arg_6   | 16  (2)     | `[args16_start + 38]`    |
| 19 | arg_7   | 16  (2)     | `[args16_start + 40]`    |
| 20 | arg_8   | 16  (2)     | `[args16_start + 42]`    |
| 21 | arg_9   | 16  (2)     | `[args16_start + 44]`    |
| 22 | arg_10  | 16  (2)     | `[args16_start + 46]`    |

