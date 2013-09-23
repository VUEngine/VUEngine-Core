OUTPUT_FORMAT("elf32-v810", "elf32-v810", "elf32-v810")
OUTPUT("a.elf") /* force elf format output */
OUTPUT_ARCH(v810)
TARGET(elf32-v810)
ENTRY(_start)
SEARCH_DIR(.);

MEMORY {
	ram	(!r): ORIGIN = 0x05000000, LENGTH = 64k /*64k*/
	rom	(rx): ORIGIN = 0x07000000, LENGTH = 16M
}

/* 
   If not defined, define interupt
   Handlers as system reset
*/

PROVIDE(_keyint   = 0x07FFFFF0);
PROVIDE(_timerint = 0x07FFFE10);
PROVIDE(_croint   = 0x07FFFFF0);
PROVIDE(_comint   = 0x07FFFFF0);
PROVIDE(_vpuint   = 0x07FFFFF0);


__text_vma = 0x07000000;
__data_vma = 0x05000000;

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  .text __text_vma : {
    *(.text)
    *(.text.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t*)
  } >rom = 0xFF

  .rodata : {
    *(.rodata)
    *all.rodata*(*)
    *(.roda)
    *(.rodata.*)
    *(.gnu.linkonce.r*)
    /* SORT(CONSTRUCTORS) */
  } >rom = 0xFF

  /* C++ Constructor/Destructor table */
  /*
  .ctors : {
    ___ctors = .;
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend(.ctors))
    ___ctors_end = .;
  } >rom = 0xFF

  .dtors : {
    ___dtors = .;
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))
    ___dtors_end = .;
  } >rom = 0xFF
  */

  /* Stop empty section from causing a compiler error */
  /DISCARD/ : {
    *(.comment)
  } 

  /* Place orphan ROM data here */
  /* . = .; */
  
v = .;

  /* Ram memory */

  __data_lma = .;
  .data __data_vma : AT(__data_lma) {
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d*)
    /* CONSTRUCTORS */
  } >ram = 0xFF

  __data_end = .;

  .bss : {
    *(.dynbss)
    *(.bss)
    *(COMMON)
  } >ram = 0xFF
  /* These must appear regardless of  .  */


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

/*  __vbvectors_lma = 0x07FFFFFF - 0x21F; */

  /* Place interupt and reset vector at end of rom */

  .vbvectors __vbvectors_lma : { 
    KEEP (*(.vbvectors))
  } >rom = 0xFF 
}
