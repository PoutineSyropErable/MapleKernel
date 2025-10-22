# MBR Partition Table Structure

The **MBR partition entry** (also called **partition table entry**) is **16 bytes long** — and there are **exactly four** of them (total 64 bytes) starting at offset **0x1BE** in the MBR sector (the 512-byte first sector of a disk).

## Partition Entry Layout

| Offset | Size | Field | Description |
|:------:|:----:|:------|:-------------|
| `0x00` | 1 | **Boot indicator** | `0x80` = active (bootable), `0x00` = inactive |
| `0x01` | 3 | **Starting CHS address** | Cylinder-Head-Sector of partition start (BIOS geometry, largely obsolete) |
| `0x04` | 1 | **Partition type** | Tells what filesystem / partition type (e.g. `0x83` = Linux, `0x07` = NTFS, `0x0B` = FAT32) |
| `0x05` | 3 | **Ending CHS address** | Cylinder-Head-Sector of partition end |
| `0x08` | 4 | **Starting LBA** | Little-endian 32-bit sector index of partition start |
| `0x0C` | 4 | **Total sectors** | Little-endian 32-bit number of sectors in partition |

Total: **16 bytes per entry × 4 entries = 64 bytes**.  
The final 2 bytes of the MBR (`0x55AA` at offset `0x1FE`) are the **boot signature**.

---

## Example (NASM Syntax)

```asm
; Partition 1: active, starts at LBA 1, 2048 sectors long
times 4 db 0x00           ; fill all 4 entries
partition1:
    db 0x80               ; bootable
    db 0x01,0x01,0x00     ; starting CHS (ignored, set to zeros or dummy)
    db 0xda               ; Linux partition type
    db 0xFE,0xFF,0xFF     ; ending CHS (ignored)
    dd 0x00000001         ; starting LBA
    dd 0x00000800         ; number of sectors (2048)
; partition2, partition3, partition4 would follow with same structure

times 510 - ($ - $$) db 0 ; fill remainder of boot sector
dw 0xAA55                 ; boot signature
```

---

## Notes

- CHS fields are historical; only **LBA** values matter for modern bootloaders.  
- MBR supports only **4 primary partitions**.  
- To exceed that limit, one partition can be an **extended partition**, pointing to **logical partitions** via a chain of **Extended Boot Records (EBRs)**.  
- `Starting LBA` is relative to disk start (sector 0 = MBR itself).  
- Each partition’s `Starting LBA` usually begins on a **sector boundary** (commonly 2048 = 1 MiB alignment).

