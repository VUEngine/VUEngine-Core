
OUTPUT_FORMAT("elf32-v810", "elf32-v810", "elf32-v810")
OUTPUT("a.elf") /* force elf format output */
OUTPUT_ARCH(v810)
TARGET(elf32-v810)
ENTRY(_start)
SEARCH_DIR(.);

MEMORY {

	wram (!r) : ORIGIN = 0x05000000, LENGTH = 64K
	rom	(rx) : ORIGIN = 0x07000000, LENGTH = 16M
}

__text_start = 0x07000000;
__data_start = 0x05000000;

__gp = __data_start + 0x8000;

SECTIONS
{
  . = __text_start;
  .text : {
    *(.text)
    *(.text.*)
 		/* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t*)

  } >rom = 0xFF
  
  __text_end = .;

  .rodata : {
    *(.rodata)
    *all.rodata*(*)
    *(.roda)
    *(.rodata.*)
    *(.gnu.linkonce.r*)
    /* SORT(CONSTRUCTORS) */

  } >rom = 0xFF
  
  __data_lma = .;

  .data __data_start : AT(__data_lma) {
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d*)
    /* CONSTRUCTORS */

  } >wram = 0xFF

  .sdata : {
    *(.sdata)

  } >wram = 0xFF

  __data_end = .;

  .sbss : {
    *(.sbss)
    *(.scommon)
  } >wram

  .bss : {
    *(.dynbss)
    *(.bss)
    *(COMMON)
  } >wram

  /* Compute vector table address */

  /* This promotes . to a power of two */
  v = __data_lma + (__data_end - __data_start);
  v += 0x21F;   /* add size of vector table and rom header*/
  v &= 0x00FFFFFF;
  v |= v>>1;
  v |= v>>2;
  v |= v>>4;
  v |= v>>8;
  v |= v>>16;
  __vectors_end = __text_start + v;
  __vectors_lma = __vectors_end - 0x21F;

/*  __vectors_lma = 0x07FFFFFF - 0x21F; */

  /* Place interrupt and reset vectors at end of rom */

  .vectors __vectors_lma : {
    KEEP (*(.vectors))
  } >rom = 0xFF
}
