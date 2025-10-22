2 ^ 16 = 2=^10 * 2^6 

2^10 = 1024 
2^3 = 8 
2^6 = 64
2^16 = 64 * 1024 = 64 KB


# 16-bit Segmentation in x86

In **16-bit x86 modes**, segmentation is the mechanism that lets the CPU address more memory than a single 16-bit register (64 KB).  

---

## 1. Segment Registers

There are **four main segment registers** in 16-bit real mode:

| Register | Typical Use |
|----------|------------|
| **CS**  | Code Segment – where instructions are fetched from |
| **DS**  | Data Segment – default for most data accesses |
| **SS**  | Stack Segment – used for the stack |
| **ES**  | Extra Segment – additional data segment, often for string operations |

*Later x86 CPUs also have FS and GS, but in early 16-bit systems CS, DS, SS, ES are the primary ones.*

---

## 2. Physical Address Calculation

In 16-bit real mode, **physical addresses are computed using the segment:offset formula**:

Physical Address = (Segment × 16) + Offset


```c
- Segment register is **16-bit** → shifted left 4 bits (×16)  
- Offset is 16-bit → added to the shifted segment  
- Maximum addressable memory: **1 MB** (0xFFFFF)

**Example:**
```text
CS = 0x1234, IP = 0x5678
Physical Address = 0x12340 + 0x5678 = 0x179B8
```

Since the offset is 16 bit, and the segment is << 4. 
And we do segment + offset. 
Then that means we cover 16 + 4 = 20 bit 

2 ^ 20 = 2^10 * 2^10 = 1024 * 1024 = 1MB

logical address = SEGMENT:OFFSET
linear address = segment * 16 + offset

Hence, if offset = 0, the linear address is divisable by 16. 
So... The code entry point must be divisible by 16. 
Hence 

0xABC0
The last hex must be 0. 



# 3. Segmentation Behavior

| Segment | Usage |
|---------|-------|
| CS:IP   | Next instruction to execute |
| DS:BX   | General data accesses |
| SS:SP   | Stack top |
| ES:DI   | Extra data, often for string instructions like MOVSB or MOVSW |

# 4. Segment Limits

- Each segment in real mode is 64 KB maximum.  
- CPU does not enforce bounds in real mode — illegal memory accesses may wrap around.


# 5. Key Points (16-bit Real Mode)

- Segmentation is mandatory in 16-bit real mode to access memory beyond 64 KB.  
- Each segment can address up to 64 KB, but physical addresses can go up to 1 MB via segment:offset arithmetic.  
- Physical address calculation: `physical = segment × 16 + offset`.  
- Segments can overlap freely; the CPU does not enforce bounds in real mode.  
    - Enforcing bounds means you can't `(char *)array[OVERFLOW_NUMBER]`. It means going past the segment limit
- Segment registers define the base for memory accesses; there are no enforced limits or access rights.  
- CPU does **not** enforce privilege levels or permissions in real mode.


# Trivia. 
- The first cpu to have protected mode available, was a 16 bit cpu. 
- Hence, only on that old ass cpu does 16 bit protected mode exist
- For modern cpu, Protected Mode = 32 bit mode (AS I knew)




