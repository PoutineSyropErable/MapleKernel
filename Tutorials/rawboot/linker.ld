/* linker.ld example */
OUTPUT_FORMAT("binary")
ENTRY(reset)
SECTIONS
{
  . = 0xFFFFFFF0;
  .text :
  {
    *(.text16)
    *(.text32)
    *(.text64)
  }
  .data :
  {
    *(.data)
  }
}

