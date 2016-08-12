OUTPUT_FORMAT("elf32-v810", "elf32-v810", "elf32-v810")
OUTPUT("a.elf") /* force elf format output */
OUTPUT_ARCH(v810)
TARGET(elf32-v810)
ENTRY(_start)
SEARCH_DIR(.);

MEMORY
{
    wram    (!r)    : ORIGIN =  0x05000000, LENGTH = 64k /*64k*/
    vram    (!r)    : ORIGIN =  0x0003B800, LENGTH = 8k
    sram    (!r)    : ORIGIN =  0x06000000, LENGTH = 16M
    rom     (rx)    : ORIGIN =  0x07000000, LENGTH = 16M
}

/*
If not defined, define interrupt
Handlers as system reset
*/

__text_vma = ORIGIN(rom);
__data_vma = ORIGIN(wram);
__sram_vma = ORIGIN(sram);
__vram_vma = ORIGIN(vram);
__stack = ORIGIN(wram) + LENGTH(wram) - 64;

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
    __data_lma = .;
    .data __data_vma : AT(__data_lma)
    {
    	PROVIDE (__data_start = .);
        *(.data*)
    } >wram = 0xFF

    .sdata ALIGN(2):
    {
    	PROVIDE (__gp = . + 0x8000);
        *(.sdata*)
    	PROVIDE (__data_end = .);
    } >wram = 0xFF

    .vram_data __vram_vma : AT(v + SIZEOF(.data) + SIZEOF(.sdata))
    {
    	PROVIDE (__vram_data_start = .);
        *(.vram_data*)
    	PROVIDE (__vram_data_end = .);
    } >vram = 0xFF

    .sram_data __sram_vma : AT(v + SIZEOF(.data) + SIZEOF(.sdata) + SIZEOF(.vram_data)) SUBALIGN(2)
    {
    	PROVIDE (__sram_data_start = .);
        *(.sram_data*)
    	PROVIDE (__sram_data_end = .);
    } >sram = 0xFF

    .sbss (NOLOAD):
    {
    	PROVIDE (__bss_start = .);
        *(.sbss*)
        *(.scommon*)
    } >wram = 0xFF

    .bss (NOLOAD):
    {
        *(.bss*)
        *(COMMON)
    	PROVIDE (__bss_end = .);
    } >wram = 0xFF

    .vram_bss (NOLOAD):
    {
        PROVIDE(__vram_bss_start = .);
        *(.vram_bss*)
        PROVIDE(__vram_bss_end = .);
    } >vram = 0xFF

    /* SRAM Work RAM */
    .sram_bss (NOLOAD): SUBALIGN(2)
    {
        PROVIDE(__sram_bss_start = .);
        *(.sram_bss*)
        PROVIDE(__sram_bss_end = .);
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
