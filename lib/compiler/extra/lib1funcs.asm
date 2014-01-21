/* libgcc1 routines for NEC V810.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with files
   compiled with GCC to produce an executable, this does not cause
   the resulting executable to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

#ifdef	L_save_2
	.text
	.align	2
	.globl	__save_r2_r29
	.type	__save_r2_r29,@function
	/* Allocate space and save registers 2, 20 .. 29 on the stack */
	/* Called via:	jalr __save_r2_r29,r10 */
__save_r2_r29:
	addi	-44,sp,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	st.w	r25,16[sp]
	st.w	r24,20[sp]
	st.w	r23,24[sp]
	st.w	r22,28[sp]
	st.w	r21,32[sp]
	st.w	r20,36[sp]
	st.w	r2,40[sp]
	jmp	[r1]
	.size	__save_r2_r29,.-__save_r2_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r2_r29 */
	.align	2
	.globl	__return_r2_r29
	.type	__return_r2_r29,@function
__return_r2_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	ld.w	16[sp],r25
	ld.w	20[sp],r24
	ld.w	24[sp],r23
	ld.w	28[sp],r22
	ld.w	32[sp],r21
	ld.w	36[sp],r20
	ld.w	40[sp],r2
	addi	44,sp,sp
	jmp	[r31]
	.size	__return_r2_r29,.-__return_r2_r29
#endif /* L_save_2 */

#ifdef	L_save_20
	.text
	.align	2
	.globl	__save_r20_r29
	.type	__save_r20_r29,@function
	/* Allocate space and save registers 20 .. 29 on the stack */
	/* Called via:	jalr __save_r20_r29,r10 */
__save_r20_r29:
	addi	-40,sp,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	st.w	r25,16[sp]
	st.w	r24,20[sp]
	st.w	r23,24[sp]
	st.w	r22,28[sp]
	st.w	r21,32[sp]
	st.w	r20,36[sp]
	jmp	[r1]
	.size	__save_r20_r29,.-__save_r20_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r20_r29 */
	.align	2
	.globl	__return_r20_r29
	.type	__return_r20_r29,@function
__return_r20_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	ld.w	16[sp],r25
	ld.w	20[sp],r24
	ld.w	24[sp],r23
	ld.w	28[sp],r22
	ld.w	32[sp],r21
	ld.w	36[sp],r20
	addi	40,sp,sp
	jmp	[r31]
	.size	__return_r20_r29,.-__return_r20_r29
#endif /* L_save_20 */

#ifdef	L_save_21
	.text
	.align	2
	.globl	__save_r21_r29
	.type	__save_r21_r29,@function
	/* Allocate space and save registers 21 .. 29 on the stack */
	/* Called via:	jalr __save_r21_r29,r10 */
__save_r21_r29:
	addi	-36,sp,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	st.w	r25,16[sp]
	st.w	r24,20[sp]
	st.w	r23,24[sp]
	st.w	r22,28[sp]
	st.w	r21,32[sp]
	jmp	[r1]
	.size	__save_r21_r29,.-__save_r21_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r21_r29 */
	.align	2
	.globl	__return_r21_r29
	.type	__return_r21_r29,@function
__return_r21_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	ld.w	16[sp],r25
	ld.w	20[sp],r24
	ld.w	24[sp],r23
	ld.w	28[sp],r22
	ld.w	32[sp],r21
	addi	36,sp,sp
	jmp	[r31]
	.size	__return_r21_r29,.-__return_r21_r29
#endif /* L_save_21 */

#ifdef	L_save_22
	.text
	.align	2
	.globl	__save_r22_r29
	.type	__save_r22_r29,@function
	/* Allocate space and save registers 22 .. 29 on the stack */
	/* Called via:	jalr __save_r22_r29,r10 */
__save_r22_r29:
	addi	-32,sp,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	st.w	r25,16[sp]
	st.w	r24,20[sp]
	st.w	r23,24[sp]
	st.w	r22,28[sp]
	jmp	[r1]
	.size	__save_r22_r29,.-__save_r22_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r22_r29 */
	.align	2
	.globl	__return_r22_r29
	.type	__return_r22_r29,@function
__return_r22_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	ld.w	16[sp],r25
	ld.w	20[sp],r24
	ld.w	24[sp],r23
	ld.w	28[sp],r22
	addi	32,sp,sp
	jmp	[r31]
	.size	__return_r22_r29,.-__return_r22_r29
#endif /* L_save_22 */

#ifdef	L_save_23
	.text
	.align	2
	.globl	__save_r23_r29
	.type	__save_r23_r29,@function
	/* Allocate space and save registers 23 .. 29 on the stack */
	/* Called via:	jalr __save_r23_r29,r10 */
__save_r23_r29:
	addi	-28,sp,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	st.w	r25,16[sp]
	st.w	r24,20[sp]
	st.w	r23,24[sp]
	jmp	[r1]
	.size	__save_r23_r29,.-__save_r23_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r23_r29 */
	.align	2
	.globl	__return_r23_r29
	.type	__return_r23_r29,@function
__return_r23_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	ld.w	16[sp],r25
	ld.w	20[sp],r24
	ld.w	24[sp],r23
	addi	28,sp,sp
	jmp	[r31]
	.size	__return_r23_r29,.-__return_r23_r29
#endif /* L_save_23 */

#ifdef	L_save_24
	.text
	.align	2
	.globl	__save_r24_r29
	.type	__save_r24_r29,@function
	/* Allocate space and save registers 24 .. 29 on the stack */
	/* Called via:	jalr __save_r24_r29,r10 */
__save_r24_r29:
	addi	-24,sp,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	st.w	r25,16[sp]
	st.w	r24,20[sp]
	jmp	[r1]
	.size	__save_r24_r29,.-__save_r24_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r24_r29 */
	.align	2
	.globl	__return_r24_r29
	.type	__return_r24_r29,@function
__return_r24_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	ld.w	16[sp],r25
	ld.w	20[sp],r24
	addi	24,sp,sp
	jmp	[r31]
	.size	__return_r24_r29,.-__return_r24_r29
#endif /* L_save_24 */

#ifdef	L_save_25
	.text
	.align	2
	.globl	__save_r25_r29
	.type	__save_r25_r29,@function
	/* Allocate space and save registers 25 .. 29 on the stack */
	/* Called via:	jalr __save_r25_r29,r10 */
__save_r25_r29:
	addi	-20,sp,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	st.w	r25,16[sp]
	jmp	[r1]
	.size	__save_r25_r29,.-__save_r25_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r25_r29 */
	.align	2
	.globl	__return_r25_r29
	.type	__return_r25_r29,@function
__return_r25_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	ld.w	16[sp],r25
	addi	20,sp,sp
	jmp	[r31]
	.size	__return_r25_r29,.-__return_r25_r29
#endif /* L_save_25 */

#ifdef	L_save_26
	.text
	.align	2
	.globl	__save_r26_r29
	.type	__save_r26_r29,@function
	/* Allocate space and save registers 26 .. 29 on the stack */
	/* Called via:	jalr __save_r26_r29,r10 */
__save_r26_r29:
	add	-16,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	st.w	r26,12[sp]
	jmp	[r1]
	.size	__save_r26_r29,.-__save_r26_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r26_r29 */
	.align	2
	.globl	__return_r26_r29
	.type	__return_r26_r29,@function
__return_r26_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	ld.w	12[sp],r26
	addi	16,sp,sp
	jmp	[r31]
	.size	__return_r26_r29,.-__return_r26_r29
#endif /* L_save_26 */

#ifdef	L_save_27
	.text
	.align	2
	.globl	__save_r27_r29
	.type	__save_r27_r29,@function
	/* Allocate space and save registers 27 .. 29 on the stack */
	/* Called via:	jalr __save_r27_r29,r10 */
__save_r27_r29:
	add	-12,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	st.w	r27,8[sp]
	jmp	[r1]
	.size	__save_r27_r29,.-__save_r27_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r27_r29 */
	.align	2
	.globl	__return_r27_r29
	.type	__return_r27_r29,@function
__return_r27_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	ld.w	8[sp],r27
	add	12,sp
	jmp	[r31]
	.size	__return_r27_r29,.-__return_r27_r29
#endif /* L_save_27 */

#ifdef	L_save_28
	.text
	.align	2
	.globl	__save_r28_r29
	.type	__save_r28_r29,@function
	/* Allocate space and save registers 28,29 on the stack */
	/* Called via:	jalr __save_r28_r29,r10 */
__save_r28_r29:
	add	-8,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	st.w	r28,4[sp]
	jmp	[r1]
	.size	__save_r28_r29,.-__save_r28_r29

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r28_r29 */
	.align	2
	.globl	__return_r28_r29
	.type	__return_r28_r29,@function
__return_r28_r29:
	ld.w	0[sp],r29
	ld.w	4[sp],r28
	add	8,sp
	jmp	[r31]
	.size	__return_r28_r29,.-__return_r28_r29
#endif /* L_save_28 */

#ifdef	L_save_29
	.text
	.align	2
	.globl	__save_r29
	.type	__save_r29,@function
	/* Allocate space and save register 29 on the stack */
	/* Called via:	jalr __save_r29,r10 */
__save_r29:
	add	-4,sp
	mov	r31,r1
	mov	r10,r31
	st.w	r29,0[sp]
	jmp	[r1]
	.size	__save_r29,.-__save_r29

	/* Restore saved register 29, deallocate stack and return to the user */
	/* Called via:	jr __return_r29 */
	.align	2
	.globl	__return_r29
	.type	__return_r29,@function
__return_r29:
	ld.w	0[sp],r29
	add	4,sp
	jmp	[r31]
	.size	__return_r29,.-__return_r29
#endif /* L_save_29 */

#ifdef	L_save_2c
	.text
	.align	2
	.globl	__save_r2_r31
	.type	__save_r2_r31,@function
	/* Allocate space and save registers 20 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r2_r31,r10 */
__save_r2_r31:
	addi	-64,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r25,32[sp]
	st.w	r24,36[sp]
	st.w	r23,40[sp]
	st.w	r22,44[sp]
	st.w	r21,48[sp]
	st.w	r20,52[sp]
	st.w	r2,56[sp]
	st.w	r10,60[sp]
	jmp	[r31]
	.size	__save_r2_r31,.-__save_r2_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r20_r31 */
	.align	2
	.globl	__return_r2_r31
	.type	__return_r2_r31,@function
__return_r2_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r25
	ld.w	36[sp],r24
	ld.w	40[sp],r23
	ld.w	44[sp],r22
	ld.w	48[sp],r21
	ld.w	52[sp],r20
	ld.w	56[sp],r2
	ld.w	60[sp],r31
	addi	64,sp,sp
	jmp	[r31]
	.size	__return_r2_r31,.-__return_r2_r31
#endif /* L_save_2c */

#ifdef	L_save_20c
	.text
	.align	2
	.globl	__save_r20_r31
	.type	__save_r20_r31,@function
	/* Allocate space and save registers 20 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r20_r31,r10 */
__save_r20_r31:
	addi	-60,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r25,32[sp]
	st.w	r24,36[sp]
	st.w	r23,40[sp]
	st.w	r22,44[sp]
	st.w	r21,48[sp]
	st.w	r20,52[sp]
	st.w	r10,56[sp]
	jmp	[r31]
	.size	__save_r20_r31,.-__save_r20_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r20_r31 */
	.align	2
	.globl	__return_r20_r31
	.type	__return_r20_r31,@function
__return_r20_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r25
	ld.w	36[sp],r24
	ld.w	40[sp],r23
	ld.w	44[sp],r22
	ld.w	48[sp],r21
	ld.w	52[sp],r20
	ld.w	56[sp],r31
	addi	60,sp,sp
	jmp	[r31]
	.size	__return_r20_r31,.-__return_r20_r31
#endif /* L_save_20c */

#ifdef	L_save_21c
	.text
	.align	2
	.globl	__save_r21_r31
	.type	__save_r21_r31,@function
	/* Allocate space and save registers 21 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r21_r31,r10 */
__save_r21_r31:
	addi	-56,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r25,32[sp]
	st.w	r24,36[sp]
	st.w	r23,40[sp]
	st.w	r22,44[sp]
	st.w	r21,48[sp]
	st.w	r10,52[sp]
	jmp	[r31]
	.size	__save_r21_r31,.-__save_r21_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r21_r31 */
	.align	2
	.globl	__return_r21_r31
	.type	__return_r21_r31,@function
__return_r21_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r25
	ld.w	36[sp],r24
	ld.w	40[sp],r23
	ld.w	44[sp],r22
	ld.w	48[sp],r21
	ld.w	52[sp],r31
	addi	56,sp,sp
	jmp	[r31]
	.size	__return_r21_r31,.-__return_r21_r31
#endif /* L_save_21c */

#ifdef	L_save_22c
	.text
	.align	2
	.globl	__save_r22_r31
	.type	__save_r22_r31,@function
	/* Allocate space and save registers 22 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r22_r31,r10 */
__save_r22_r31:
	addi	-52,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r25,32[sp]
	st.w	r24,36[sp]
	st.w	r23,40[sp]
	st.w	r22,44[sp]
	st.w	r10,48[sp]
	jmp	[r31]
	.size	__save_r22_r31,.-__save_r22_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r22_r31 */
	.align	2
	.globl	__return_r22_r31
	.type	__return_r22_r31,@function
__return_r22_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r25
	ld.w	36[sp],r24
	ld.w	40[sp],r23
	ld.w	44[sp],r22
	ld.w	48[sp],r31
	addi	52,sp,sp
	jmp	[r31]
	.size	__return_r22_r31,.-__return_r22_r31
#endif /* L_save_22c */

#ifdef	L_save_23c
	.text
	.align	2
	.globl	__save_r23_r31
	.type	__save_r23_r31,@function
	/* Allocate space and save registers 23 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r23_r31,r10 */
__save_r23_r31:
	addi	-48,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r25,32[sp]
	st.w	r24,36[sp]
	st.w	r23,40[sp]
	st.w	r10,44[sp]
	jmp	[r31]
	.size	__save_r23_r31,.-__save_r23_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r23_r31 */
	.align	2
	.globl	__return_r23_r31
	.type	__return_r23_r31,@function
__return_r23_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r25
	ld.w	36[sp],r24
	ld.w	40[sp],r23
	ld.w	44[sp],r31
	addi	48,sp,sp
	jmp	[r31]
	.size	__return_r23_r31,.-__return_r23_r31
#endif /* L_save_23c */

#ifdef	L_save_24c
	.text
	.align	2
	.globl	__save_r24_r31
	.type	__save_r24_r31,@function
	/* Allocate space and save registers 24 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r24_r31,r10 */
__save_r24_r31:
	addi	-44,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r25,32[sp]
	st.w	r24,36[sp]
	st.w	r10,40[sp]
	jmp	[r31]
	.size	__save_r24_r31,.-__save_r24_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r24_r31 */
	.align	2
	.globl	__return_r24_r31
	.type	__return_r24_r31,@function
__return_r24_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r25
	ld.w	36[sp],r24
	ld.w	40[sp],r31
	addi	44,sp,sp
	jmp	[r31]
	.size	__return_r24_r31,.-__return_r24_r31
#endif /* L_save_24c */

#ifdef	L_save_25c
	.text
	.align	2
	.globl	__save_r25_r31
	.type	__save_r25_r31,@function
	/* Allocate space and save registers 25 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r25_r31,r10 */
__save_r25_r31:
	addi	-40,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r25,32[sp]
	st.w	r10,36[sp]
	jmp	[r31]
	.size	__save_r25_r31,.-__save_r25_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r25_r31 */
	.align	2
	.globl	__return_r25_r31
	.type	__return_r25_r31,@function
__return_r25_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r25
	ld.w	36[sp],r31
	addi	40,sp,sp
	jmp	[r31]
	.size	__return_r25_r31,.-__return_r25_r31
#endif /* L_save_25c */

#ifdef	L_save_26c
	.text
	.align	2
	.globl	__save_r26_r31
	.type	__save_r26_r31,@function
	/* Allocate space and save registers 26 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r26_r31,r10 */
__save_r26_r31:
	addi	-36,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r26,28[sp]
	st.w	r10,32[sp]
	jmp	[r31]
	.size	__save_r26_r31,.-__save_r26_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r26_r31 */
	.align	2
	.globl	__return_r26_r31
	.type	__return_r26_r31,@function
__return_r26_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r26
	ld.w	32[sp],r31
	addi	36,sp,sp
	jmp	[r31]
	.size	__return_r26_r31,.-__return_r26_r31
#endif /* L_save_26c */

#ifdef	L_save_27c
	.text
	.align	2
	.globl	__save_r27_r31
	.type	__save_r27_r31,@function
	/* Allocate space and save registers 27 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r27_r31,r10 */
__save_r27_r31:
	addi	-32,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r27,24[sp]
	st.w	r10,28[sp]
	jmp	[r31]
	.size	__save_r27_r31,.-__save_r27_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r27_r31 */
	.align	2
	.globl	__return_r27_r31
	.type	__return_r27_r31,@function
__return_r27_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r27
	ld.w	28[sp],r31
	addi	32,sp,sp
	jmp	[r31]
	.size	__return_r27_r31,.-__return_r27_r31
#endif /* L_save_27c */

#ifdef	L_save_28c
	.text
	.align	2
	.globl	__save_r28_r31
	.type	__save_r28_r31,@function
	/* Allocate space and save registers 28 .. 29, 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r28_r31,r10 */
__save_r28_r31:
	addi	-28,sp,sp
	st.w	r29,16[sp]
	st.w	r28,20[sp]
	st.w	r10,24[sp]
	jmp	[r31]
	.size	__save_r28_r31,.-__save_r28_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r28_r31 */
	.align	2
	.globl	__return_r28_r31
	.type	__return_r28_r31,@function
__return_r28_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r28
	ld.w	24[sp],r31
	addi	28,sp,sp
	jmp	[r31]
	.size	__return_r28_r31,.-__return_r28_r31
#endif /* L_save_28c */

#ifdef	L_save_29c
	.text
	.align	2
	.globl	__save_r29_r31
	.type	__save_r29_r31,@function
	/* Allocate space and save registers 29 & 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r29_r31,r10 */
__save_r29_r31:
	addi	-24,sp,sp
	st.w	r29,16[sp]
	st.w	r10,20[sp]
	jmp	[r31]
	.size	__save_r29_r31,.-__save_r29_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r29_r31 */
	.align	2
	.globl	__return_r29_r31
	.type	__return_r29_r31,@function
__return_r29_r31:
	ld.w	16[sp],r29
	ld.w	20[sp],r31
	addi	24,sp,sp
	jmp	[r31]
	.size	__return_r29_r31,.-__return_r29_r31
#endif /* L_save_29c */

#ifdef	L_save_31c
	.text
	.align	2
	.globl	__save_r31
	.type	__save_r31,@function
	/* Allocate space and save register 31 on the stack */
	/* Also allocate space for the argument save area */
	/* Called via:	jalr __save_r29_r31,r10 */
__save_r31:
	addi	-20,sp,sp
	st.w	r10,16[sp]
	jmp	[r31]
	.size	__save_r31,.-__save_r31

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r31 */
	.align	2
	.globl	__return_r31
	.type	__return_r31,@function
__return_r31:
	ld.w	16[sp],r31
	addi	20,sp,sp
	jmp	[r31]
	.size	__return_r31,.-__return_r31
#endif /* L_save_31c */

#ifdef L_save_varargs
	.text
	.align	2
	.globl	__save_r6_r9
	.type	__save_r6_r9,@function
	/* Save registers 6 .. 9 on the stack for variable argument functions */
	/* Called via:	jalr __save_r6_r9,r10 */
__save_r6_r9:
	addi	-20,sp,sp
	mov	r31, r1
	mov	r10, r31
	st.w	r6,0[sp]
	st.w	r7,4[sp]
	st.w	r8,8[sp]
	st.w	r9,12[sp]
	st.w	r10,16[sp]
	jmp	[r1]
	.size	__save_r6_r9,.-__save_r6_r9

	/* Restore saved registers, deallocate stack and return to the user */
	/* Called via:	jr __return_r6_r9 */
	.align	2
	.globl	__return_r6_r9
	.type	__return_r6_r9,@function
__return_r6_r9:
	ld.w	0[sp],r6
	ld.w	4[sp],r7
	ld.w	8[sp],r8
	ld.w	12[sp],r9
	ld.w	16[sp],r31
	addi	20,sp,sp
	jmp	[r31]
	.size	__return_r6_r9,.-__return_r6_r9
#endif /* L_save_varargs */

#ifdef	L_save_interrupt
	.text
	.align	2
	.globl	__save_interrupt
	.type	__save_interrupt,@function
	/* Save registers r1, r4 on stack and load up with expected values */
	/* Note, 12 bytes of stack have already been allocated. */
	/* Called via:	jalr __save_interrupt,r10 */
__save_interrupt:
	st.w	r30,0[sp]
	st.w	r10,4[sp]
	st.w	gp,8[sp]
	st.w	r1,12[sp]
	movhi	hi(__gp),r0,gp
	movea	lo(__gp),gp,gp
	jmp	[r31]
	.size	__save_interrupt,.-__save_interrupt

	/* Restore saved registers, deallocate stack and return from the interrupt */
	/* Called via:	jr __return_interrupt */
	.align	2
	.globl	__return_interrupt
	.type	__return_interrupt,@function
__return_interrupt:
	ld.w	0[sp],r30
	ld.w	4[sp],r10
	ld.w	8[sp],gp
	ld.w	12[sp],r1
	ld.w	16[sp],r31
	addi	20,sp,sp
	reti
	.size	__return_interrupt,.-__return_interrupt
#endif /* L_save_interrupt */

#ifdef L_save_all_interrupt
	.text
	.align	2
	.globl	__save_all_interrupt
	.type	__save_all_interrupt,@function
	/* Save all registers except for those saved in __save_interrupt */
	/* allocate enough stack for all of the registers & 16 bytes of space */
	/* Called via:	jalr __save_all_interrupt,r10 */
__save_all_interrupt:
	addi	-116,sp,sp
	st.w	r2,112[sp]
	st.w	r5,108[sp]
	st.w	r6,104[sp]
	st.w	r7,100[sp]
	st.w	r8,96[sp]
	st.w	r9,92[sp]
	st.w	r11,88[sp]
	st.w	r12,84[sp]
	st.w	r13,80[sp]
	st.w	r14,76[sp]
	st.w	r15,72[sp]
	st.w	r16,68[sp]
	st.w	r17,64[sp]
	st.w	r18,60[sp]
	st.w	r19,56[sp]
	st.w	r20,52[sp]
	st.w	r21,48[sp]
	st.w	r22,44[sp]
	st.w	r23,40[sp]
	st.w	r24,36[sp]
	st.w	r25,32[sp]
	st.w	r26,28[sp]
	st.w	r27,24[sp]
	st.w	r28,20[sp]
	st.w	r29,16[sp]
	jmp	[r31]
	.size	__save_all_interrupt,.-__save_all_interrupt

	.globl	__restore_all_interrupt
	.type	__restore_all_interrupt,@function
	/* Restore all registers saved in __save_all_interrupt */
	/* & deallocate the stack space */
	/* Called via:	jalr __restore_all_interrupt,r10 */
__restore_all_interrupt:
	ld.w	112[sp],r2
	ld.w	108[sp],r5
	ld.w	104[sp],r6
	ld.w	100[sp],r7
	ld.w	96[sp],r8
	ld.w	92[sp],r9
	ld.w	88[sp],r11
	ld.w	84[sp],r12
	ld.w	80[sp],r13
	ld.w	76[sp],r14
	ld.w	72[sp],r15
	ld.w	68[sp],r16
	ld.w	64[sp],r17
	ld.w	60[sp],r18
	ld.w	56[sp],r19
	ld.w	52[sp],r20
	ld.w	48[sp],r21
	ld.w	44[sp],r22
	ld.w	40[sp],r23
	ld.w	36[sp],r24
	ld.w	32[sp],r25
	ld.w	28[sp],r26
	ld.w	24[sp],r27
	ld.w	20[sp],r28
	ld.w	16[sp],r29
	addi	116,sp,sp
	jmp	[r31]
	.size	__restore_all_interrupt,.-__restore_all_interrupt
#endif /* L_save_all_interrupt */
