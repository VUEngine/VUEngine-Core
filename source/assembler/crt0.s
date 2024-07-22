	.section .text
	.align	1


/*************************************************
 pointers
 *************************************************/

.global keyVector
keyVector = 0x0500FFC0

.global timVector
timVector = 0x0500FFC4

.global croVector
croVector = 0x0500FFC8

.global comVector
comVector = 0x0500FFCC

.global vipVector
vipVector = 0x0500FFD0

.global zeroDivisionVector
zeroDivisionVector = 0x0500FFD4

.global invalidOpcodeVector
invalidOpcodeVector = 0x0500FFD8

.global floatingPointVector
floatingPointVector = 0x0500FFDC


/*************************************************
  startup code
 *************************************************/
	.global	start

start:
	movhi	1536, r0, r10
	movea	0, r10, r10
	ld.b	0[r10], r18
	ld.b	2[r10], r19
	ld.b	4[r10], r20
	ld.b	6[r10], r21

/* store SRAM's sample */
	movhi	hi(_sramSample), r0, r10
	movea	lo(_sramSample), r10, r10
	st.b	r18,    0[r10]
	add		1, 		r10
	st.b	r19,    0[r10]
	add		1, 		r10
	st.b	r20,    0[r10]
	add		1, 		r10
	st.b	r21,    0[r10]

/* read SRAM's sample */
/*
	movhi	hi(_stack), r0,sp
	movea	lo(_stack), sp,sp
	movhi	hi(continue),r0,lp
	movea	lo(continue),lp,lp
	movhi	hi(readSRAM),r0,r1
	movea	lo(readSRAM),r1,r1
	jmp	    [r1]
*/

continue:

/* read WRAM's sample */
	mov		0, r10
	ld.b	1280[r10], r18
	add		2, r10
	ld.b	1280[r10], r19
	add		2, r10
	ld.b	1280[r10], r20
	add		2, r10
	ld.b	1280[r10], r21

/* store WRAM's sample */
	movhi	hi(_wramSample), r0, r10
	movea	lo(_wramSample), r10, r10
	st.b	r18,    0[r10]
	add		1, 		r10
	st.b	r19,    0[r10]
	add		1, 		r10
	st.b	r20,    0[r10]
	add		1, 		r10
	st.b	r21,    0[r10]

/* wait for WRAM */
	movea	0x2000, r0, r6
waitforwramloop:
	add		-1, r6
	bnz		waitforwramloop

/* dummy reads */
	movhi	hi(_dataStart),   r0, r7
	movea	lo(_dataStart),   r7, r7
	movea	0x0008, 			r0, r8
dummyreadcycle:
	ld.b	0[r7], r9
	add	    1,     r7
	cmp	    r8,    r7
	blt	    dummyreadcycle

/* initiallize .data section */
	movhi	hi(_dataLma), 	r0, r6
	movea	lo(_dataLma), 	r6, r6
	movhi	hi(_dataStart),   r0, r7
	movea	lo(_dataStart),   r7, r7
	movhi	hi(_dataEnd),		r0, r8
	movea	lo(_dataEnd),		r8, r8
	jr	    endinitdata

topinitdata:
	ld.b	0[r6], r9
	st.b	r9,    0[r7]
	add	    1,     r6
	add	    1,     r7
endinitdata:
	cmp	    r8,    r7
	blt	    topinitdata

/* initiallize .dramdata section */
	movhi	hi(_dramDataStart), r0, r7
	movea	lo(_dramDataStart), r7, r7
	movhi	hi(_dramDataEnd),   r0, r8
	movea	lo(_dramDataEnd),   r8, r8
	jr	    endinitdramdata

topinitdramdata:
	ld.b	0[r6], r9
	st.b	r9,    0[r7]
	add	    1,     r6
	add	    1,     r7
endinitdramdata:
	cmp	    r8,    r7
	blt	    topinitdramdata

/* initiallize .sramdata section */
	movhi	hi(_sramDataStart), r0, r7
	movea	lo(_sramDataStart), r7, r7
	movhi	hi(_sramDataEnd),   r0, r8
	movea	lo(_sramDataEnd),   r8, r8
	jr	    endinitsramData

topinitsramData:
	ld.b	0[r6], r9
	st.b	r9,    0[r7]
	add	    1,     r6
	add	    1,     r7
endinitsramData:
	cmp	    r8,    r7
	blt	    topinitsramData

/* clear .bss section */
	movhi	hi(_bssStart), r0, r6
	movea	lo(_bssStart), r6, r6
	movhi	hi(_bssEnd),   r0, r7
	movea	lo(_bssEnd),   r7, r7
	jr	    endinitbss
topinitbss:
	st.h	r0, 0[r6]
	add	    1,  r6
endinitbss:
	cmp	    r7, r6
	blt	    topinitbss

/* clear .drambss section */
	movhi   hi(_dramBssStart),   r0, r6
	movea   lo(_dramBssStart),   r6, r6
	movhi   hi(_dramBssEnd),     r0, r7
	movea   lo(_dramBssEnd),     r7, r7
	jr      endinitdrambss
topinitdrambss:
	st.b    r0, 0[r6]
	add	    1,  r6
endinitdrambss:
	cmp     r7, r6
	blt     topinitdrambss

/* clear .srambss section */
	movhi   hi(_sramBssStart),   r0, r6
	movea   lo(_sramBssStart),   r6, r6
	movhi   hi(_sramBssEnd),     r0, r7
	movea   lo(_sramBssEnd),     r7, r7
	jr      endinitsrambss
topinitsrambss:
	st.b    r0, 0[r6]
	add	    1,  r6
endinitsrambss:
	cmp     r7, r6
	blt     topinitsrambss

/* clean psw */
	ldsr	r0, psw

/* setup sp, fp, gp, and tp */
	movhi	hi(_stack), r0,sp
	movea	lo(_stack), sp,sp

	movhi	hi(_gp), r0, gp
	movea   lo(_gp), gp, gp

	movhi	hi(_textStart), r0,tp
	movea   lo(_textStart), tp, tp

/* long call setup classes */
	.global	setupClasses

	movhi	hi(_initengine),r0,lp
	movea	lo(_initengine),lp,lp

	movhi	hi(setupClasses), r0, r1
	movea	lo(setupClasses), r1, r1
	jmp	    [r1]

/* long call setup engine */
_initengine:

	movhi	hi(_callmain),r0,lp
	movea	lo(_callmain),lp,lp

	movhi	hi(VUEngine_init), r0, r1
	movea	lo(VUEngine_init), r1, r1
	jmp	    [r1]

_callmain:
/* long call main function */
	movhi	hi(_end),r0,lp
	movea	lo(_end),lp,lp

	movhi	hi(main),r0,r1
	movea	lo(main),r1,r1

/* disable-clear-enable cache GCC 4.7 */
    ldsr    r0, chcw
    mov     1, r6
    ldsr    r6, chcw
    mov     2, r6
    ldsr    r6, chcw

	jmp	    [r1]
_end:

/* Reset when main returns */
	movhi   hi(0xFFFFFFF0),r0,lp
	movea	lo(0xffffFFF0),lp,lp
	jmp	    [lp]

/* interrupt handler*/
_interrupthandler:
	jmp	    [r1]

	.global	_interrupthandlerprolog

_interrupthandlerprolog:
	addi	-0x0050,sp,sp
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
	stsr	eipc,r1
	st.w	r1,0x0048[sp]
	stsr	eipsw,r1
	st.w	r1,0x004c[sp]
	movhi	hi(keyVector),r0,r1
	movea	lo(keyVector),r1,r1
	stsr	sr5,r6
	shr	    0x0E,r6
	andi	0x003C,r6,r6
	add	    r6,r1
	ld.w	-4[r1],r1
	jal     _interrupthandler

_interrupthandlerepilogue:
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
	ld.w	0x0048[sp],r1
	ldsr	r1,eipc
	ld.w	0x004c[sp],r1
	ldsr	r1,eipsw
	ld.w	0x0044[sp],r1
	addi	0x0050,sp,sp
	reti

_zeroexception:
	movhi	hi(zeroDivisionVector), r0, r1
	movea	lo(zeroDivisionVector), r1, r1
	ld.w	0[r1],r1
	jmp	    [r1]

_invalidopcodeexception:
	movhi	hi(invalidOpcodeVector), r0, r1
	movea	lo(invalidOpcodeVector), r1, r1
	ld.w	0[r1],r1
	jmp	    [r1]

_floatingpointexception:
	movhi	hi(floatingPointVector), r0, r1
	movea	lo(floatingPointVector), r1, r1
	ld.w	0[r1],r1
	jmp	    [r1]

	.section ".vbvectors","ax"
	.align	1

/* Hardware Interupt Vectors */
interrupttable:

    /* INTKEY (7FFFE00h) - Controller Interrupt */
	jr _interrupthandlerprolog
	.fill	0x0c

    /* INTTIM (7FFFE10h) - Timer Interrupt */
	jr _interrupthandlerprolog
	.fill	0x0c

    /* INTCRO (7FFFE20h) - Expansion Port Interrupt */
	jr _interrupthandlerprolog
	.fill	0x0c

    /* INTCOM (7FFFE30h) - Link Port Interrupt */
	jr _interrupthandlerprolog
	.fill	0x0c

    /* INTVPU (7FFFE40h) - Video Retrace Interrupt */
	jr _interrupthandlerprolog
	.fill	0x0c

    /* Unused vectors (7FFFE50h-7FFFF5Fh) */
	.fill	0x0110

    /* (7FFFF60h) - Float exception */
	jr _floatingpointexception
	.fill	0x0c

    /* Unused vector */
	.fill	0x10

    /* (7FFFF80h) - Divide by zero exception */
	jr _zeroexception
	.fill	0x0c

    /* (7FFFF90h) - Invalid Opcode exception */
	jr _invalidopcodeexception
	.fill	0x0c

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
	.fill	0x0E

    /* Unused vector */
	.fill	0x10

    /* Reset Vector (7FFFFF0h) - This is how the ROM boots */
	movhi	hi(start), r0, r1
	movea	lo(start), r1, r1
	jmp	    [r1]
	.fill	0x06

