	.section .text
	.align	1


/*************************************************
 pointers
 *************************************************/

.global _key_vector
_key_vector = 0x0500FFC0

.global _tim_vector
_tim_vector = 0x0500FFC4

.global _cro_vector
_cro_vector = 0x0500FFC8

.global _com_vector
_com_vector = 0x0500FFCC

.global _vpu_vector
_vpu_vector = 0x0500FFD0



/*************************************************
  startup code
 *************************************************/
	.global	_start

_start:

/* wait for WRAM */
	movea	0x2000, r0, r6
wait_for_wram_loop:
	add		-1, r6
	bnz		wait_for_wram_loop

/* dummy SRAM reads */
	movhi   hi(__sram_start), r0, r1
	movea   lo(__sram_start), r1, r1
    movea	0x00C0,           r0, r6
sram_dummy_loop:
	ld.b    0[r1], r7
	add	    2,     r1
	add		-1,    r6
	bnz		sram_dummy_loop

/* initiallize .data section */
	movhi	hi(__data_start), r0, r7
	movea	lo(__data_start), r7, r7
	movhi	hi(__data_end),   r0, r1
	movea	lo(__data_end),   r1, r1
	movhi	hi(__data_vma),   r0, r6
	movea	lo(__data_vma),   r6, r6
	jr	    end_init_data

top_init_data:
	ld.b	0[r7], r8
	st.b	r8,    0[r6]
	add	    1,     r7
	add	    1,     r6
end_init_data:
	cmp	    r1,    r6
	blt	    top_init_data

/* clear .bss section and unintialized RAM */
	movhi	hi(__bss_start), r0, r1
	movea	lo(__bss_start), r1, r1
	movhi	hi(__bss_end),   r0, r7
	movea	lo(__bss_end),   r7, r7
	jr	    end_init_bss
top_init_bss:
	st.h	r0, 0[r1]
	add	    1,  r1
end_init_bss:
	cmp	    r7, r1
	blt	    top_init_bss

/* clear .sram section and unintialized SRAM */
	movhi   hi(__sram_start),   r0, r1
	movea   lo(__sram_start),   r1, r1
	movhi   hi(__sram_end),     r0, r7
	movea   lo(__sram_end),     r7, r7
	jr      end_init_sram
top_init_sram:
	st.b    r0, 0[r1]
	add	    1,  r1
end_init_sram:
	cmp     r7, r1
	blt     top_init_sram

/* disable-clear-enable cache GCC 4.7 */
	ldsr	r0, sr5
    ldsr    r0, chcw
    mov     2, r1
    ldsr    r1, chcw

/* setup sp, fp, gp, and tp */
	movhi	hi(__stack),r0,sp
	movea	lo(__stack),sp,sp

	movhi	hi(__gp), r0, gp
	movea   lo(__gp), gp, gp

	movhi	hi(__tp),r0,tp
	movea   lo(__tp), tp, tp

/* long call main function */
	movhi	hi(__end),r0,lp
	movea	lo(__end),lp,lp

	movhi	hi(_main),r0,r1
	movea	lo(_main),r1,r1
	jmp	    [r1]
__end:

/* Reset when main returns */
	movhi   hi(0xFFFFFFF0),r0,lp
	movea	lo(0xffffFFF0),lp,lp
	jmp	    [lp]

/* interrupt handler*/

__interrupt_handler:
	jmp	    [r1]

	.global	__interrupt_handler_prolog

__interrupt_handler_prolog:
	addi	-0x0048,sp,sp
	st.w	lp,0x0000[sp]
	st.w	r30,0x0004[sp]
	st.w	r19,0x0008[sp]
	st.w	r18,0x000c[sp]
	st.w	r17,0x0010[sp]
	st.w	r16,0x0014[sp]
	st.w	r15,0x0018[sp]
	st.w	r14,0x001c[sp]
	st.w	r13,0x0020[sp]
	st.w	r12,0x0024[sp]
	st.w	r11,0x0028[sp]
	st.w	r10,0x002c[sp]
	st.w	r9,0x0030[sp]
	st.w	r8,0x0034[sp]
	st.w	r7,0x0038[sp]
	st.w	r6,0x003c[sp]
	st.w	r2,0x0040[sp]
	st.w	r1,0x0044[sp]
	movhi	hi(_key_vector),r0,r1
	movea	lo(_key_vector),r1,r1
	stsr	sr5,r6
	shr	    0x0E,r6
	andi	0x003C,r6,r6
	add	    r6,r1
	ld.w	-4[r1],r1
	cmp	    r0,r1
	be	    __interrupt_handler_epilogue
	jal	    __interrupt_handler


__interrupt_handler_epilogue:
	ld.w	0x0000[sp],lp
	ld.w	0x0004[sp],r30
	ld.w	0x0008[sp],r19
	ld.w	0x000c[sp],r18
	ld.w	0x0010[sp],r17
	ld.w	0x0014[sp],r16
	ld.w	0x0018[sp],r15
	ld.w	0x001c[sp],r14
	ld.w	0x0020[sp],r13
	ld.w	0x0024[sp],r12
	ld.w	0x0028[sp],r11
	ld.w	0x002c[sp],r10
	ld.w	0x0030[sp],r9
	ld.w	0x0034[sp],r8
	ld.w	0x0038[sp],r7
	ld.w	0x003c[sp],r6
	ld.w	0x0040[sp],r2
	ld.w	0x0044[sp],r1
	addi	0x0048,sp,sp
	reti

	.section ".vbvectors","ax"
	.align	1

.global _rom_title

/* Rom info table (07FFFDE0h) */
_rom_title:
	.ascii  "change this title   "		/* Game Title          */
	.byte   0x00,0x00,0x00,0x00,0x00 	/* Reserved            */
	.ascii  "MFGMID"					/* Manufacture/Game ID */
	.byte   0x01						/* Rom Version         */


/* Hardware Interupt Vectors */
_interrupt_table:

    /* INTKEY (7FFFE00h) - Controller Interrupt */
	jr __interrupt_handler_prolog
	.fill	0x0c

    /* INTTIM (7FFFE10h) - Timer Interrupt */
	jr __interrupt_handler_prolog
	.fill	0x0c

    /* INTCRO (7FFFE20h) - Expansion Port Interrupt */
	jr __interrupt_handler_prolog
	.fill	0x0c

    /* INTCOM (7FFFE30h) - Link Port Interrupt */
	jr __interrupt_handler_prolog
	.fill	0x0c

    /* INTVPU (7FFFE40h) - Video Retrace Interrupt */
	jr __interrupt_handler_prolog
	.fill	0x0c

    /* Unused vectors (7FFFE50h-7FFFF5Fh) */
	.fill	0x010F

    /* (7FFFF60h) - Float exception */
	reti
	.fill	0x0E

    /* Unused vector */
	.fill	0x10

    /* (7FFFF80h) - Divide by zero exception */
	reti
	.fill	0x0E

    /* (7FFFF90h) - Invalid Opcode exception */
	reti
	.fill	0x0E

    /* (7FFFFA0h) - Trap 0 exception */
	reti
	.fill	0x0E

    /* (7FFFFB0h) - Trap 1 exception */
	reti
	.fill	0x0E

    /* (7FFFFC0h) - Trap Address exception */
	reti
	.fill	0x0E

    /* (7FFFFD0h) - NMI/Duplex exception */
	reti
	.fill	0x0F

    /* Unused vector */
	.fill	0x10

    /* Reset Vector (7FFFFF0h) - This is how the ROM boots */
	movhi	hi(_start), r0, r1
	movea	lo(_start), r1, r1
	jmp	    [r1]
	.fill	0x06
