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

	.section .text
	.align	1
	.global	_start
#---------------------------------------------------------------------------------
_start:
#---------------------------------------------------------------------------------
	movhi	hi(0x0500FFC0), r0, sp
	movea	lo(0x0500FFC0), sp, sp
	movhi	hi(__gp), r0, gp
	movea	lo(__gp), gp, gp

	ldsr	r0, psw
	ldsr	r0, chcw

	movea	0x2000, r0, r6
DummyLoop:
	add		-1, r6
	bnz		DummyLoop

# copy data section from LMA to VMA (ROM to RAM)

	movhi	hi(__data_lma), r0, r6
	movea	lo(__data_lma), r6, r6
	movhi	hi(__data_end), r0, r7
	movea	lo(__data_end), r7, r7
	movhi	hi(__data_start), r0, r8
	movea	lo(__data_start), r8, r8

	jr		end_initdata
initdata:
	ld.b	0[r6],r9
	st.b	r9,0[r8]
	add		1,r6
	add		1,r8
end_initdata:
	cmp		r7,r8
	blt		initdata

/* clear bss section and unintialized WRAM */
	movhi	0x0501, r0, r6
	jr		loop_start1
loop_top1:
	st.h	r0, 0[r7]
	add		2, r7
loop_start1:
	cmp		r6, r7
	blt		loop_top1

# clear VRM and DRAM

	mov		r0, r7
	movhi	4, r0, r6			# r6 = 0x40000
ClrLoop:
	st.w	r0, 0[r7]
	add		4, r7
	add		-4, r6
	bnz		ClrLoop


	jal		_setcoltable	# set column table data

# Set NVC control registers

	movhi	0x0200, r0, r6

	movea	0x0014, r0, r7
	st.b	r7, 0x00[r6]
	mov		-1, r7
	st.b	r7, 0x04[r6]

	st.b	r0, 0x20[r6]
	movea	0x0080, r0, r7
	st.b	r7, 0x28[r6]	# disable key input interrupt


/* long call main function */
	movhi	hi(__end), r0, lp
	movea	lo(__end), lp, lp

	movhi	hi(_main), r0, r1
	movea	lo(_main), r1, r1
	jmp		[r1]

__end:

/* Reset when main returns */
	movhi   hi(0xFFFFFFF0), r0, lp
	movea	lo(0xffffFFF0), lp, lp
	jmp		[lp]


/* Setup the default Column Table */
_setcoltable:
	movhi	0x0004,r0,r6
	movea	0xDC00,r6,r6
	movhi	hi(coltable),r0,r7
	movea	lo(coltable),r7,r7
	movea	256,r0,r8
	add		r6,r8
	jr		setcoltable_end
setcoltable_loop:
	ld.b	0x0000[r7],r9
	andi	0x00FF,r9,r9
	st.h	r9,0x0000[r6]
	st.h	r9,0x0200[r6]
	add		2,r6
	add		1,r7
setcoltable_end:
	cmp		r8,r6
	blt		setcoltable_loop
	movea	256,r0,r8
	add		r6,r8
	jr		setcoltable_end2
setcoltable_loop2:
	add		-1,r7
	ld.b	0x0000[r7],r9
	andi	0x00FF,r9,r9
	st.h	r9,0x0000[r6]
	st.h	r9,0x0200[r6]
	add		2,r6
setcoltable_end2:
	cmp		r8,r6
	blt		setcoltable_loop2
	jmp		[lp]

coltable:
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE
	.byte	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xE0,0xBC
	.byte	0xA6,0x96,0x8A,0x82,0x7A,0x74,0x6E,0x6A
	.byte	0x66,0x62,0x60,0x5C,0x5A,0x58,0x56,0x54
	.byte	0x52,0x50,0x50,0x4E,0x4C,0x4C,0x4A,0x4A
	.byte	0x48,0x48,0x46,0x46,0x46,0x44,0x44,0x44
	.byte	0x42,0x42,0x42,0x40,0x40,0x40,0x40,0x40
	.byte	0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3C
	.byte	0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C
	.byte	0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C


.macro pushall
	addi	-112, sp, sp					 
	st.w	lp, 0[sp]
	st.w	r30, 4[sp]
	st.w	r29, 8[sp]
	st.w	r28, 12[sp]
	st.w	r27, 16[sp]
	st.w	r26, 20[sp]
	st.w	r25, 24[sp]
	st.w	r24, 28[sp]
	st.w	r23, 32[sp]
	st.w	r22, 36[sp]
	st.w	r21, 40[sp]
	st.w	r20, 44[sp]
	st.w	r19, 48[sp]
	st.w	r18, 52[sp]
	st.w	r17, 56[sp]
	st.w	r16, 60[sp]
	st.w	r15, 64[sp]
	st.w	r14, 68[sp]
	st.w	r13, 72[sp]
	st.w	r12, 76[sp]
	st.w	r11, 80[sp]
	st.w	r10, 84[sp]
	st.w	r9, 88[sp]
	st.w	r8, 92[sp]
	st.w	r7, 96[sp]
	st.w	r6, 100[sp]
	st.w	r5, 104[sp]
	st.w	r2, 108[sp]
.endm

.macro popall
	ld.w	0[sp], lp
	ld.w	4[sp], r30
	ld.w	8[sp], r29
	ld.w	12[sp], r28
	ld.w	16[sp], r27
	ld.w	20[sp], r26
	ld.w	24[sp], r25
	ld.w	28[sp], r24
	ld.w	32[sp], r23
	ld.w	36[sp], r22
	ld.w	40[sp], r21
	ld.w	44[sp], r20
	ld.w	48[sp], r19
	ld.w	52[sp], r18
	ld.w	56[sp], r17
	ld.w	60[sp], r16
	ld.w	64[sp], r15
	ld.w	68[sp], r14
	ld.w	72[sp], r13
	ld.w	76[sp], r12
	ld.w	80[sp], r11
	ld.w	84[sp], r10
	ld.w	88[sp], r9
	ld.w	92[sp], r8
	ld.w	96[sp], r7
	ld.w	100[sp], r6
	ld.w	104[sp], r5
	ld.w	108[sp], r2
	addi	112, sp, sp
.endm


/* interrupt handler */

jmp_r1:
	jmp		[r1]

	.global	__inthnd

__inthnd:
	pushall
	movhi	hi(_key_vector),r0,r1	 
	movea	lo(_key_vector),r1,r1
	stsr	psw,r6			
	shr		0x0E,r6
	andi	0x003C,r6,r6
	add		r6,r1
	ld.w	-4[r1],r1
	cmp		r0,r1
	be		__inthnd_end
	jal		jmp_r1
__inthnd_end:
	popall
	ld.w	0[sp], r1
	add		4, sp
	reti


	.section ".vectors", "ax"
	.align	1

	.global _rom_header

_rom_header:
	.ascii	"                    "		# Game Title (7FFFDE0h)
	.byte	0x00,0x00,0x00,0x00,0x00	# Reserved (7FFFDF4h)
	.ascii	"  V  E"					# Maker Code, Game Code
	.byte	0x00						# ROM Version No (7FFFDFFh)


    /* Hardware Interrupt Vectors */

_int_table:

    /* INTKEY (7FFFE00h) - Controller Interrupt */
	add		-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp		[r1]

    /* INTTIM (7FFFE10h) - Timer Interrupt */
	add		-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp		[r1]

    /* INTCRO (7FFFE20h) - Expansion Port Interrupt */
	add		-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp		[r1]

    /* INTCOM (7FFFE30h) - Link Port Interrupt */
	add		-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp		[r1]

    /* INTVPU (7FFFE40h) - VIP Interrupt */
	add		-4, sp
	st.w	r1, 0[sp]
	movhi	hi(__inthnd), r0, r1
	movea	lo(__inthnd), r1, r1
	jmp		[r1]

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

    /* Reset Vector (7FFFFF0h) - program start address */
	movhi	hi(_start), r0, r1
	movea	lo(_start), r1, r1
	jmp		[r1]
	.fill	0x06
