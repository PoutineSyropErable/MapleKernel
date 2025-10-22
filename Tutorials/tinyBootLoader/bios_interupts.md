

To load a bigger bootloader, from the first 512 boot loader



```asm
mov ah, 0x02       ; BIOS read sectors
mov al, 0x02       ; Number of sectors to read
mov ch, 0x00       ; Cylinder
mov cl, 0x03       ; Sector (starts at 1)
mov dh, 0x00       ; Head
mov dl, 0x80       ; Drive number (first HDD)
mov bx, 0x8000     ; Load address in memory
int 0x13           ; BIOS disk interrupt
jc disk_error      ; Jump if carry set (error)

jump 0x8000

disk_error: 
    ; do some handling

```

This reads 2 sectors from the disk and puts them at 0x8000.

After that, the boot sector can jump there to continue execution.



Stages: 

Bios will copy and jump to the 512 byte of the drive. 
This is the mini bootloader. 

The mini boot loader will then read from another part of the drive, and read from it. Copy it to drive (Done manually)
This will load the real boot loader into drive. 

The 16 bit mode



```sql
+----------------------------+  <- Sector 0 (MBR)
| Miniboot code (446 bytes)  |
+----------------------------+
| Partition Table Entry 1    |  <- 16 bytes
+----------------------------+
| Partition Table Entry 2    |  <- 16 bytes
+----------------------------+
| Partition Table Entry 3    |  <- 16 bytes
+----------------------------+
| Partition Table Entry 4    |  <- 16 bytes
+----------------------------+
| Boot Signature 0x55AA      |  <- 2 bytes
+----------------------------+

+----------------------------+  <- Sector 1
| Partition 1 (16 bootloader)  |
|  ...                       |
+----------------------------+

+----------------------------+  <- Following sectors
| Partition 2 ( 32 bit chain bootloader )
|  ...                       |
+----------------------------+

+----------------------------+
| Partition 3 (32 bit transformer [32->64 bit])               |
|  ...                       |
+----------------------------+

+----------------------------+
| Partition 4 (64 bit kernel)
|  ...                       |
+----------------------------+



```
It only has 4 partition. Can't have more. 


And here's how the memory on the pc will be: 

```
Memory (physical addresses)
0x00000 ─────────────────────────────────────
| IVT (Interrupt Vector Table)              | 0x00000 – 0x003FF | (0-1kb)
| BIOS Data Area (BDA)                      | 0x00400 – 0x04FF  | (1kb - 1.25kb)
| Reserved / low memory                     | 0x0500 – 0x7BFF   | (1.25kb - 31.25kb)
0x07C00 ─────────────────────────────────────
| Mini bootloader (446 bytes)               | 0x07C00 – 0x07DBE | 31.25kb - 31.44 kb
| Partition table + boot signature          | 0x07Dbf – 0x07e00 | 31.44kb - 31.50kb
0x08000 ─────────────────────────────────────
| 16-bit full bootloader (boot16)           | 0x08000 – 0x0FFFF | 32kb - 64kb
| Stack for boot16                          | 0x10000 – 0x11FFF | 64kb - 72kb
0x20000 ─────────────────────────────────────
| 32-bit boot chain loader (bootchain32)    | 0x20000 – 0x2FFFF | 128kb - 192kb
| Stack / temp buffers for bootchain32      | 0x30000 – 0x3FFFF | 192kb - 256kb
0x40000 ─────────────────────────────────────
| 32-bit transformer (16->64 bit)           | 0x40000 – 0x5FFFF | 256kb - 384kb
| Stack for transformer                     | 0x60000 – 0x7FFFF | 384kb - 512kb
0x100000 ────────────────────────────────────
| 64-bit kernel (higher half mapping)       | 0x100000+ mapped to higher half virtual address
| Kernel heap / data                        | 0x120000 – ...
| Kernel stack                              | 0x140000 – ...    | physical address = 1 Mb 


```

Higher half means mapped for virtual address higher half. Not physical address higher half

