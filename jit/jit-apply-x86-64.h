/*
 * jit-apply-x86-64.h - Special definitions for x86-64 function application.
 *
 * Copyright (C) 2004  Southern Storm Software, Pty Ltd.
 *
 * This file is part of the libjit library.
 *
 * The libjit library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The libjit library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the libjit library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef	_JIT_APPLY_X86_64_H
#define	_JIT_APPLY_X86_64_H

/*
 * The "__builtin_apply" functionality in gcc orders the registers
 * in a strange way, which makes it difficult to use.  Our replacement
 * apply structure is laid out in the following order:
 *
 *		stack pointer
 *		%rdi, %rsi, %rdx, %rcx, %r8, %r9
 *		64-bit pad word
 *		%xmm0-%xmm7
 *
 * The total size of the apply structure is 192 bytes.  The return structure
 * is laid out as follows:
 *
 *		%rax, %rdx
 *		%xmm0
 *		%st0
 *
 * The total size of the return structure is 48 bytes.
 */

#if defined(__GNUC__)

#ifndef	JIT_MEMCPY
#define	JIT_MEMCPY	"jit_memcpy@PLT"
#endif

/*
 * We have to add all registers not saved by the caller to the clobber list
 * and not only the registers used for parameter passing because we call
 * arbitrary functions.
 * Maybe we should add the mmx* registers too?
 */
#define	jit_builtin_apply(func,args,size,return_float,return_buf)	\
		do { \
			void *__func = (void *)(func); \
			void *__args = (void *)(args); \
			long __size = (long)(size); \
			void *__return_buf = alloca(64); \
			(return_buf) = __return_buf; \
			__asm__ ( \
				"movq %1, %%rax\n\t" \
				"movq (%%rax), %%rsi\n\t" \
				"movq %2, %%rdx\n\t" \
				"subq %%rdx, %%rsp\n\t" \
				"movq %%rsp, %%rdi\n\t" \
				"callq " JIT_MEMCPY "\n\t" \
				"movq %1, %%rax\n\t" \
				"movq 0x08(%%rax), %%rdi\n\t" \
				"movq 0x10(%%rax), %%rsi\n\t" \
				"movq 0x18(%%rax), %%rdx\n\t" \
				"movq 0x20(%%rax), %%rcx\n\t" \
				"movq 0x28(%%rax), %%r8\n\t" \
				"movq 0x30(%%rax), %%r9\n\t" \
				"movaps 0x40(%%rax), %%xmm0\n\t" \
				"movaps 0x50(%%rax), %%xmm1\n\t" \
				"movaps 0x60(%%rax), %%xmm2\n\t" \
				"movaps 0x70(%%rax), %%xmm3\n\t" \
				"movaps 0x80(%%rax), %%xmm4\n\t" \
				"movaps 0x90(%%rax), %%xmm5\n\t" \
				"movaps 0xA0(%%rax), %%xmm6\n\t" \
				"movaps 0xB0(%%rax), %%xmm7\n\t" \
				"movq %0, %%rax\n\t" \
				"callq *%%rax\n\t" \
				"movq %3, %%rcx\n\t" \
				"movq %%rax, (%%rcx)\n\t" \
				"movq %%rdx, 0x08(%%rcx)\n\t" \
				"movaps %%xmm0, 0x10(%%rcx)\n\t" \
				"movq %2, %%rdx\n\t" \
				"addq %%rdx, %%rsp\n\t" \
				: : "m"(__func), "m"(__args), "m"(__size), "m"(__return_buf) \
				: "rax", "rcx", "rdx", "rdi", "rsi", "r8", "r9", \
				  "r10", "r11", \
				  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", \
				  "xmm5", "xmm6", "xmm7" \
			); \
			if((return_float)) \
			{ \
				__asm__ ( \
					"movq %0, %%rax\n\t" \
					"fstpt 0x20(%%rax)\n\t" \
					: : "m"(__return_buf) \
					: "rax", "st" \
				); \
			} \
		} while (0)

#define	jit_builtin_apply_args(type,args)	\
		do { \
			void *__args = alloca(192); \
			__asm__ ( \
				"pushq %%rdi\n\t" \
				"leaq 16(%%rbp), %%rdi\n\t" \
				"movq %0, %%rax\n\t" \
				"movq %%rdi, (%%rax)\n\t" \
				"popq %%rdi\n\t" \
				"movq %%rdi, 0x08(%%rax)\n\t" \
				"movq %%rsi, 0x10(%%rax)\n\t" \
				"movq %%rdx, 0x18(%%rax)\n\t" \
				"movq %%rcx, 0x20(%%rax)\n\t" \
				"movq %%r8, 0x28(%%rax)\n\t" \
				"movq %%r9, 0x30(%%rax)\n\t" \
				"movaps %%xmm0, 0x40(%%rax)\n\t" \
				"movaps %%xmm1, 0x50(%%rax)\n\t" \
				"movaps %%xmm2, 0x60(%%rax)\n\t" \
				"movaps %%xmm3, 0x70(%%rax)\n\t" \
				"movaps %%xmm4, 0x80(%%rax)\n\t" \
				"movaps %%xmm5, 0x90(%%rax)\n\t" \
				"movaps %%xmm6, 0xA0(%%rax)\n\t" \
				"movaps %%xmm7, 0xB0(%%rax)\n\t" \
				: : "m"(__args) \
				: "rax", "rcx", "rdx", "rdi", "rsi", "r8", "r9", \
				  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", \
				  "xmm5", "xmm6", "xmm7" \
			); \
			(args) = (type)__args; \
		} while (0)

#define	jit_builtin_return_int(return_buf)	\
		do { \
			__asm__ ( \
				"lea %0, %%rcx\n\t" \
				"movq (%%rcx), %%rax\n\t" \
				"movq 0x08(%%rcx), %%rdx\n\t" \
				"movaps 0x10(%%rcx), %%xmm0\n\t" \
				: : "m"(*(return_buf)) \
				: "rax", "rcx", "rdx", "xmm0" \
			); \
			return; \
		} while (0)

#define	jit_builtin_return_float(return_buf)	\
		do { \
			__asm__ ( \
				"lea %0, %%rcx\n\t" \
				"movaps 0x10(%%rcx), %%xmm0\n\t" \
				"fldt 0x20(%%rcx)\n\t" \
				: : "m"(*(return_buf)) \
				: "rcx", "xmm0", "st" \
			); \
			return; \
		} while (0)

#endif /* GNUC */

/*
 * The maximum number of bytes that are needed to represent a closure,
 * and the alignment to use for the closure.
 */
#define	jit_closure_size		0x90
#define	jit_closure_align		0x20

/*
 * The number of bytes that are needed for a redirector stub.
 * This includes any extra bytes that are needed for alignment.
 */
#define	jit_redirector_size		0x100

/*
 * The number of bytes that are needed for a indirector stub.
 * This includes any extra bytes that are needed for alignment.
 */
#define	jit_indirector_size		0x10


#endif	/* _JIT_APPLY_X86_64_H */
