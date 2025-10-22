# Available registers
# 16-bit x86 Registers

In **16-bit x86 real mode**, the CPU provides several types of registers: **general-purpose, segment, pointer/index, instruction pointer, and flags**.

---

## 1. General-Purpose Registers (16-bit)

| Register | Description | Notes |
|----------|------------|------|
| **AX**   | Accumulator | Often used in arithmetic, I/O, and string operations |
| **BX**   | Base | Often used as a pointer to data in memory |
| **CX**   | Count | Often used as a loop counter or for string operations |
| **DX**   | Data | Used in I/O, multiply/divide instructions, string operations |

**Each 16-bit general-purpose register can be split into two 8-bit halves:**

| 16-bit | 8-bit high | 8-bit low |
|--------|------------|-----------|
| AX     | AH         | AL        |
| BX     | BH         | BL        |
| CX     | CH         | CL        |
| DX     | DH         | DL        |

---

## 2. Segment Registers

| Register | Typical Use |
|----------|------------|
| **CS**  | Code Segment – instructions are fetched from here |
| **DS**  | Data Segment – default for most data accesses |
| **SS**  | Stack Segment – stack operations (push/pop) |
| **ES**  | Extra Segment – extra data, often for string operations |

> Note: Modern x86 also has FS and GS, but in classic 16-bit mode these were usually unused.

---

## 3. Pointer and Index Registers

| Register | Typical Use |
|----------|------------|
| **SP**  | Stack Pointer – offset within SS |
| **BP**  | Base Pointer – usually points to the base of the current stack frame |
| **SI**  | Source Index – used for string operations |
| **DI**  | Destination Index – used for string operations |

---

## 4. Instruction Pointer

| Register | Description |
|----------|------------|
| **IP**   | Instruction Pointer – offset within CS for the next instruction |

---

## 5. Flags Register

| Register | Description |
|----------|------------|
| **FLAGS** | Contains status and control flags such as Zero Flag (ZF), Carry Flag (CF), Sign Flag (SF), Interrupt Enable (IF), and others |

---

✅ **Summary:**

- **General-purpose**: AX, BX, CX, DX  
- **Segment**: CS, DS, SS, ES  
- **Pointer/index**: SP, BP, SI, DI  
- **Instruction pointer**: IP  
- **Flags**: FLAGS  

All memory access in 16-bit real mode uses **segment:offset addressing**.



---
# Calling Conventions 
Caller saved: 
AX, CX, DX 
AX: Return value

Callee saved: 
BX, SI, DI, BP, SP (stack pointer must be restored before returning)


Argument orders: 
No registers are used for arguments. 
- No dedicated argument registers. 

So, all arguments are pushed on the stack from left to right. 

```c

uint16_t func(uint16_t a, uint16_t b, uint16_t c) {
    return a+b+c;
}

void _start() {
uint16_t x = func(5,6,7);
op16(x);

}
```

```asm 

func: 
    push bp
    mov bp, sp
    push bx 


    mov ax, [bp + 8] ; a
    mov bx, [bp + 6] ; b
    
    add ax, bx      ; total = a + b
    mov bx, [bp + 4] ;  bx = c
    add ax, bx      ; total = (a + b) + c;

     
    pop bx
    mov sp, bp
    ret


_start:
    
    push ax 
    
    push 5 
    push 6 
    push 7
    call func
    
    op16 ax 
    ; Do some operation on the result you just obtained
    
    ; optional, revert ax to how it was before the push ax
    pop ax 
    op16_old_value ax

    ...
```
