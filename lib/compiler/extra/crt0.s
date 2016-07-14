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
	movhi   hi(__sram_start), r0, r5
	movea   lo(__sram_start), r5, r5
    movea	0x00C0,           r0, r6
sram_dummy_loop:
	ld.b    0[r5], r4
	add	    2,     r5
	add		-1,    r6
	bnz		sram_dummy_loop

/* setup stack, gp, and tp */
	movhi	hi(__gp), r0, r5
	movea   lo(__gp), r5, r5
	mov	    r5,       gp

/* initiallize .data section */
	movhi	hi(__data_start), r0, r4
	movea	lo(__data_start), r4, r4
	movhi	hi(__data_end),   r0, r5
	movea	lo(__data_end),   r5, r5
	movhi	hi(__data_vma),   r0, r6
	movea	lo(__data_vma),   r6, r6
	jr	end_init_data

top_init_data:
	ld.b	0[r4], r7
	st.b	r7,    0[r6]
	add	    1,     r4
	add	    1,     r6
end_init_data:
	cmp	    r5,    r6
	blt	    top_init_data

/* clear .bss section and unintialized RAM */
	movhi	hi(__bss_start), r0, r5
	movea	lo(__bss_start), r5, r5
	movhi	hi(__bss_end),   r0, r4
	movea	lo(__bss_end),   r4, r4
	jr	    end_init_bss
top_init_bss:
	st.h	r0, 0[r5]
	add	    1,  r5
end_init_bss:
	cmp	    r4, r5
	blt	    top_init_bss

/* clear .sram section and unintialized SRAM */
	movhi   hi(__sram_start),   r0, r5
	movea   lo(__sram_start),   r5, r5
	movhi   hi(__sram_end),     r0, r4
	movea   lo(__sram_end),     r4, r4
	jr      end_init_sram
top_init_sram:
	st.b    r0, 0[r5]
	add	    1,  r5
end_init_sram:
	cmp     r4, r5
	blt     top_init_sram

/* disable-clear-enable cache GCC 4.7 */
/*    ldsr    r0,chcw
    ori     0x8001,r0,r1
    ldsr    r1,chcw
    mov     2,r1
    ldsr    r1,chcw
*/

/* cache GCC 4.4 */
	ldsr	r0,sr5
	mov	2,r4
/*	ldsr	r4,sr14*/

/* VIP */
	movhi	0x0006,r0,r4
	movea	0xF800,r4,r4

	/* DPCTRL */
	ld.h	0x0020[r4],r5
	ori	0x0101,r5,r5
	st.h	r5,0x0022[r4]

	/* INTENB */
	st.h	r0,0x0002[r4]

	/* INTCLR */
	movea	0xE01F,r0,r5
	st.h	r5,0x0004[r4]

	/* XPCTRL */
	movea	0x0001,r0,r5
	st.h	r5,0x0042[r4]

	/* FRMCYC */
	st.h	r5,0x002E[r4]

	/* REST */
	st.h	r0,0x002A[r4]

	/* Column Table */
	/*jal	_setcoltable*/


/* wait until !(DTSTTS & 0x40) */
loop_top4:
	ld.h	0x0020[r4],r5
	andi	0x40,r5,r5
	be	loop_top4


/* clear VRAM\CHR */
	movhi	2,r0,r4
	movea	0xFFFF,r4,r4
	mov	r0,r5
	jr	loop_start3
loop_top3:
	st.h	r0,0[r5]
	add	2,r5
loop_start3:
	cmp	r4,r5
	blt	loop_top3


/* HW regs */
	movhi	0x200,r0,r4

	/* Link Port Transmit data */
	movea	0xFF80,r0,r5
	st.b	r5,0x0008[r4]

	/* Link Port Control Register */
	movea	0x0014,r0,r5
	st.b	r5,0x0000[r4]

	/* Link Port Control Register 2 */
	mov	-1,r5
	st.b	r5,0x0004[r4]

	/* Timer Control Register */
	st.b	r0,0x0020[r4]

	/* Keypad Control Register */
	movea	0x0080,r0,r5
	st.b	r5,0x0028[r4]


/* Audio regs */
	movhi	0x0100,r0,r4

	/* Main sound control register */
	mov	1,r5
	st.b	r5,0x0580[r4]

/* clear channel length and control registers */
	mov	r0,r5
	jr	loop_start5
loop_top5:
	mov	r5,r6

	shl	6,r6
	movhi	0x0100,r6,r4
	st.b	r0,0x0404[r4]
	st.b	r0,0x0400[r4]
	add	1,r5
loop_start5:
	cmp	6,r5
	blt	loop_top5


/* short wait loop */
	mov	r0,r5
	movea	0x2000,r0,r4
	jr	loop_start6
loop_top6:
	add	1,r5
loop_start6:
	cmp	r4,r5
	blt	loop_top6

	/* Main sound control register */
	movhi	0x0100,r0,r4
	st.b	r0,0x0580[r4]


/* VIP */
	movhi	6, r0, r4
	movea	0xF800, r4, r4

	/* XPCTRL */
	movea	0x0001,r0,r5
	st.h	r5,0x0042[r4]

	/* DPCTRL */
	movea	0x0101,r0,r5
	st.h	r5,0x0022[r4]

	/* BRTx */
	movea	0x0020,r0,r5
	st.h	r5,0x0024[r4]
	st.h	r5,0x0028[r4]
	movea	0x0040,r0,r5
	st.h	r5,0x0026[r4]

	/* JPLTx\GPLTx */
	movea	0x00E4,r0,r5
	st.h	r5,0x0060[r4]
	st.h	r5,0x0062[r4]
	st.h	r5,0x0064[r4]
	st.h	r5,0x0066[r4]
	st.h	r5,0x0068[r4]
	st.h	r5,0x006A[r4]
	st.h	r5,0x006C[r4]
	st.h	r5,0x006E[r4]

	/* BKCOL */
	st.h	r0,0x0070[r4]

	/* SPTx */
	st.h	r0,0x004E[r4]
	st.h	r0,0x004C[r4]
	st.h	r0,0x004A[r4]
	st.h	r0,0x0048[r4]

	/* WORLD(31) = WRLD_END */
	movhi	4,r0,r4
	movea	0xDBE0,r4,r4
	movea	0x0040,r0,r5
	st.h	r5,0[r4]


/* setup stack */
	movhi	0x0501,r0,sp
	movea	0xFFC0,sp,sp

/* clear interrupt vectors */
	mov	sp,r4
	movea	0x0010,r0,r5
loop_intclear:
	st.w	r0,0[r4]
	add	4,r4
	add	-1,r5
	bne	loop_intclear


/* long call main function */
	movhi	hi(__end),r0,lp
	movea	lo(__end),lp,lp

	movhi	hi(_main),r0,r1
	movea	lo(_main),r1,r1
	jmp	[r1]

__end:

/* Reset when main returns */
	movhi   hi(0xFFFFFFF0),r0,lp
	movea	lo(0xffffFFF0),lp,lp
	jmp	[lp]


/* interrupt handler*/

jmp_r1:
	jmp	[r1]

	.global	__inthnd

__inthnd:
	addi	-0x0074,sp,sp
	st.w	lp,0x0000[sp]
	st.w	r30,0x0004[sp]
	st.w	r29,0x0008[sp]
	st.w	r28,0x000C[sp]
	st.w	r27,0x0010[sp]
	st.w	r26,0x0014[sp]
	st.w	r25,0x0018[sp]
	st.w	r24,0x001C[sp]
	st.w	r23,0x0020[sp]
	st.w	r22,0x0024[sp]
	st.w	r21,0x0028[sp]
	st.w	r20,0x002C[sp]
	st.w	r19,0x0030[sp]
	st.w	r18,0x0034[sp]
	st.w	r17,0x0038[sp]
	st.w	r16,0x003C[sp]
	st.w	r15,0x0040[sp]
	st.w	r14,0x0044[sp]
	st.w	r13,0x0048[sp]
	st.w	r12,0x004C[sp]
	st.w	r11,0x0050[sp]
	st.w	r10,0x0054[sp]
	st.w	r9,0x0058[sp]
	st.w	r8,0x005C[sp]
	st.w	r7,0x0060[sp]
	st.w	r6,0x0064[sp]
	st.w	r5,0x0068[sp]
	st.w	r4,0x006C[sp]
	st.w	r2,0x0070[sp]
	movhi	hi(_key_vector),r0,r1
	movea	lo(_key_vector),r1,r1
	stsr	sr5,r6
	shr	0x0E,r6
	andi	0x003C,r6,r6
	add	r6,r1
	ld.w	-4[r1],r1
	cmp	r0,r1
	be	__inthnd_end
	jal	jmp_r1
__inthnd_end:
	ld.w	0x0000[sp],lp
	ld.w	0x0004[sp],r30
	ld.w	0x0008[sp],r29
	ld.w	0x000C[sp],r28
	ld.w	0x0010[sp],r27
	ld.w	0x0014[sp],r26
	ld.w	0x0018[sp],r25
	ld.w	0x001C[sp],r24
	ld.w	0x0020[sp],r23
	ld.w	0x0024[sp],r22
	ld.w	0x0028[sp],r21
	ld.w	0x002C[sp],r20
	ld.w	0x0030[sp],r19
	ld.w	0x0034[sp],r18
	ld.w	0x0038[sp],r17
	ld.w	0x003C[sp],r16
	ld.w	0x0040[sp],r15
	ld.w	0x0044[sp],r14
	ld.w	0x0048[sp],r13
	ld.w	0x004C[sp],r12
	ld.w	0x0050[sp],r11
	ld.w	0x0054[sp],r10
	ld.w	0x0058[sp],r9
	ld.w	0x005C[sp],r8
	ld.w	0x0060[sp],r7
	ld.w	0x0064[sp],r6
	ld.w	0x0068[sp],r5
	ld.w	0x006C[sp],r4
	ld.w	0x0070[sp],r2
	ld.w	0x0074[sp],r1
	addi	0x0078,sp,sp
	reti


	.section ".vbvectors","ax"
	.align	1


.global _rom_title

    /* Rom info table (07FFFDE0h) */

_rom_title:
	.ascii "change this title   "	/* Game Title          */
	.byte  0x00,0x00,0x00,0x00,0x00 /* Reserved            */
	.ascii "MFGMID"			/* Manufacture/Game ID */
	.byte  0x01			/* Rom Version         */

    /* Hardware Interupt Vectors */

_int_table:

    /* INTKEY (7FFFE00h) - Controller Interrupt */
	add	-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp	[r1]

    /* INTTIM (7FFFE10h) - Timer Interrupt */
	add	-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp	[r1]

    /* INTCRO (7FFFE20h) - Expansion Port Interrupt */
	add	-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp	[r1]

    /* INTCOM (7FFFE30h) - Link Port Interrupt */
	add	-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp	[r1]

    /* INTVPU (7FFFE40h) - Video Retrace Interrupt */
	add	-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp	[r1]

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
	jmp	[r1]
	.fill	0x06
