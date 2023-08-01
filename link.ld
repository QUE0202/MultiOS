ENTRY(start)
SECTIONS
{
    . = 0x7C00;
    .text : { *(.text) }
    .bss  : { *(.bss)  }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
    . = 0x1000; /* Adres, gdzie zostanie załadowane jądro systemu */
    kernel : { *(.text) }
}
