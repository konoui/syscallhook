#include <linux/linkage.h>
#include <asm/segment.h>
#include <asm/cache.h>
#include <asm/errno.h>
#include <asm/asm-offsets.h>
#include <asm/msr.h>
#include <asm/unistd.h>
#include <asm/thread_info.h>
#include <asm/hw_irq.h>
#include <asm/page_types.h>
#include <asm/irqflags.h>
#include <asm/paravirt.h>
#include <asm/percpu.h>
#include <asm/asm.h>
#include <asm/smap.h>
#include <asm/pgtable_types.h>
#include <linux/err.h>

/* Function Arguments  */
#define REG_A1		%rdi
#define REG_A2		%rsi
#define REG_A3		%rdx
#define REG_A4		%rcx
/* Volatile registers (caller-saved)  */
#define REG_V1		%rdx
#define REG_V2		%rcx
#define REG_V3		%rax
#define REG_V4		%r8
#define REG_V5		%r9
#define REG_V6		%r10
#define REG_V7		%r11
#define REG_V8		%rdi
#define REG_V9		%rsi

/* Systemcall Arguments */
#define SYS_NUM		%rax
#define REG_A1_SYS	REG_A1
#define REG_A2_SYS	REG_A2
#define REG_A3_SYS	REG_A3
#define REG_A4_SYS	%r10
#define REG_A5_SYS	%r8
#define REG_A6_SYS	%r9


.text

.global __fake_syscall_dispatcher
__fake_syscall_dispatcher:

	swapgs

	movq	%rsp, PER_CPU_VAR(rsp_scratch)
	movq	PER_CPU_VAR(cpu_current_top_of_stack), %rsp

	pushq	PER_CPU_VAR(rsp_scratch)

	ENABLE_INTERRUPTS(CLBR_NONE)

	/* store registers */
	pushq	%r11
	pushq	%rcx
	pushq	%rax	// syscall_num
	pushq	%rdi	// 1st arg
	pushq	%rsi	// 2nd arg
	pushq	%rdx	// 3rd arg
	pushq	%r10	// 4th arg
	pushq	%r8	// 5th arg
	pushq	%r9	// 6th arg

	pushq REG_A1_SYS
	pushq REG_A2_SYS
	mov SYS_NUM, PER_CPU_VAR(syscall_num)
	mov SYS_NUM, REG_A1
	popq REG_A3
	popq REG_A2
	call	pre_hooking

	DISABLE_INTERRUPTS(CLBR_NONE)

	/* load registers */
	popq	%r9
	popq	%r8
	popq	%r10
	popq	%rdx
	popq	%rsi
	popq	%rdi
	popq	%rax
	popq	%rcx
	popq	%r11
	popq	%rsp

	swapgs

	jmp	*syscall_handler
