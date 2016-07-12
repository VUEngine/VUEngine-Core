OUTPUT_FORMAT("elf32-v810", "elf32-v810", "elf32-v810")
OUTPUT("a.elf") /* force elf format output */
OUTPUT_ARCH(v810)
TARGET(elf32-v810)
ENTRY(_start)
SEARCH_DIR(.);

MEMORY
{
    ram	(!r): ORIGIN =  0x05000000, LENGTH = 64k /*64k*/
    sram_wram (rw) : ORIGIN = 0x06000000, LENGTH = 8M
    rom	(rx): ORIGIN =  0x07000000, LENGTH = 16M
}

/*
If not defined, define interrupt
Handlers as system reset
*/

PROVIDE(_key_vector = 0x0500FFC0);
PROVIDE(_tim_vector = 0x0500FFC4);
PROVIDE(_cro_vector = 0x0500FFC8);
PROVIDE(_com_vector = 0x0500FFCC);
PROVIDE(_vpu_vector = 0x0500FFD0);

__text_vma = ORIGIN(rom);
__data_vma = ORIGIN(ram);
__data_sram_vma = ORIGIN(sram_wram);
__tp = ORIGIN(rom);
__gp = ORIGIN(ram) + LENGTH(ram) / 2;
__gps = ORIGIN(sram) + LENGTH(sram) / 2;

SECTIONS
{
    /* Read-only sections, merged into text segment: */
    .text __text_vma :
    {
        *(.text*)
    } >rom = 0xFF

    .rodata :
    {
        *(.rodata*)
    } >rom = 0xFF

    /* Stop empty section from causing a compiler error */
    /DISCARD/ :
    {
        *(.comment*)
    }

    v = .;

    /* Ram memory */

    __data_lma = .;
    .data __data_vma : AT(__data_lma)
    {
        *(.data*)
    } >ram = 0xFF
    __data_end = .;

    .sda :
    {
        *(.sda*)
    } >ram = 0xFF

    .tda :
    {
        *(.tda*)
    } >ram = 0xFF

    .bss :
    {
        *(.bss*)
    } >ram = 0xFF

    .after_bss :
    {
        *(.after_bss*)
        __lastDataVariable = .;
    } >ram = 0xFF

    /* These must appear regardless of  .  */
    /* prevent overlaps with vbvectors */
    v += SIZEOF(.data) + SIZEOF(.sda) + SIZEOF(.bss) + SIZEOF(.after_bss);

    /* Compute the vector address */
    /* This promotes . to a power of two */

    v += 0x21F; 		/* add size of jump table */
    v &= 0x00FFFFFF;
    v |= v>>1;
    v |= v>>2;
    v |= v>>4;
    v |= v>>8;
    v |= v>>16;
    __vbvectors_end = __text_vma + v;
    __vbvectors_lma = __vbvectors_end - 0x21F;
    /*__vbvectors_lma = 0x07FFFFFF - 0x21F;*/

    /* Place interrupt and reset vector at end of rom */
    .vbvectors __vbvectors_lma :
    {
        KEEP (*(.vbvectors))
    } >rom = 0xFF

    /* SRAM Work RAM */
    __data_sram_lma = .;
    .sram_bss __data_sram_vma : AT(__data_sram_lma)
    {
        . = ALIGN(4);
        PROVIDE(__sram_bss_start = .);
        *(.sram_bss)
        . = ALIGN(4);
        *(.sram_bss.*)
        . = ALIGN(4);
        PROVIDE(__sram_bss_end = .);
    } >sram_wram

}
