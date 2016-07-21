OUTPUT_FORMAT("elf32-v810", "elf32-v810", "elf32-v810")
OUTPUT("a.elf") /* force elf format output */
OUTPUT_ARCH(v810)
TARGET(elf32-v810)
ENTRY(_start)
SEARCH_DIR(.);

MEMORY
{
    ram	(!r)    : ORIGIN =  0x05000000, LENGTH = 64k /*64k*/
    sram (rw)   : ORIGIN =  0x06000000, LENGTH = 8M
    rom	(rx)    : ORIGIN =  0x07000000, LENGTH = 16M
}

/*
If not defined, define interrupt
Handlers as system reset
*/

__text_vma = ORIGIN(rom);
__data_vma = ORIGIN(ram);
__sram_vma = ORIGIN(sram);
__stack = ORIGIN(ram) + LENGTH(ram) - 64;

SECTIONS
{
    /* Read-only sections, merged into text segment: */
    .text __text_vma :
    {
    	PROVIDE (__tp = .);
        *(.text*)
    } >rom = 0xFF

    .rodata :
    {
        *(.rodata*)
    } >rom = 0xFF

    v = .;

    /* Ram memory */

    __data_start = .;
    .data __data_vma : AT(__data_start)
    {
        *(.data*)
    } >ram = 0xFF

    .sdata :
    {
    	PROVIDE (__gp = . + 0x8000);
        *(.sdata*)
    } >ram = 0xFF
    __data_end = .;

    __bss_start = .;
    .sbss (NOLOAD):
    {
        *(.sbss*)
        *(.scommon*)
    } >ram = 0xFF

    .bss (NOLOAD):
    {
        *(.bss*)
        *(COMMON)
    } >ram = 0xFF
    __bss_end = .;

    /* SRAM Work RAM */
    .sram (NOLOAD):
    {
        . = ALIGN(4);
        PROVIDE(__sram_start = .);
        . = ALIGN(4);
        *(.sram*)
        . = ALIGN(4);
        PROVIDE(__sram_end = .);
    } >sram

    /* Prevent overlaps with vbvectors */
    /* The use of new variables is because GCC 4.7's linker doesn't override the v value */
    v1 = v + SIZEOF(.data) + SIZEOF(.sdata);

    /* Compute the vector address */
    /* This promotes . to a power of two */

    v2 = v1 + 0x21F; 		/* add size of jump table */
    v3 = v2 & 0x00FFFFFF;
    v4 = v3 | (v3 >> 1);
    v5 = v4 | (v4 >> 2);
    v6 = v5 | (v5 >> 4);
    v7 = v6 | (v6 >> 8);
    v8 = v7 | (v7 >> 16);

    __vbvectors_vma = __text_vma + v8 - 0x21F;

    /* Place interrupt and reset vector at end of rom */
    .vbvectors __vbvectors_vma :
    {
        KEEP (*(.vbvectors))
    } >rom = 0xFF
}
