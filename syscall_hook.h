#ifndef SYSCALL_HOOK_H
#define SYSCALL_HOOK_H

#define debug(fmt, args...)	pr_info("CPU %d " fmt, smp_processor_id(), ##args)
#define alert(fmt, args...)	pr_alert("CPU %d " fmt, smp_processor_id(), ##args)

#include<asm/syscall.h>
#define MSR_LSTAR	0xc0000082
void (*syscall_handler)(void);
long unsigned int original_dispatcher;
extern void __fake_syscall_dispatcher(void);

static inline uint64_t get_dispatcher_from_msr(void)
{
	uint64_t address;

	rdmsrl(MSR_LSTAR, address);

	return address;
}

static inline void init_hook_syscall(void)
{
	uint64_t address = get_dispatcher_from_msr();

	original_dispatcher = address;
	syscall_handler = (void (*)(void)) address;
}

static inline void hook_syscall(void)
{
	debug("hook\n");
	uint32_t low;

	low = (uint32_t) (((uint64_t)__fake_syscall_dispatcher) & 0xffffffff);

	wrmsr(MSR_LSTAR, low, 0xffffffff);
}

static inline void unhook_syscall(void)
{
	debug("unhook\n");

	uint64_t address = get_dispatcher_from_msr();
	if ((uintptr_t)address != (uintptr_t)__fake_syscall_dispatcher ) {
		alert("already unhooked\ncurrent dispatcher: 0x%lx\toriginal dispatcher: 0x%lx\n", 
		       address, original_dispatcher);
	}

	wrmsr(MSR_LSTAR, (uint32_t) (original_dispatcher & 0xffffffff), 0xffffffff);
}

/////////////////////////////////////////////////////////////////////
// Hook System Call
////////////////////////////////////////////////////////////////////
DEFINE_PER_CPU(uint64_t, rsp_scratch);
DEFINE_PER_CPU(uint64_t, syscall_num);

void pre_hooking(int num,  void *arg1, void *arg2)
{
	if (num == __NR_clone)
		debug("clone hook\tchild_stack: %p\n", arg2);
	else if (num == __NR_fork)
		debug("frok hook\n");
	else if (num == __NR_vfork)
		debug("vfrok hook\n");
	else if (num == __NR_execve)
		debug("execve hook: %s\n", arg1);
	else if (num == __NR_exit)
		debug("exit hook\n");
	else if (num == __NR_exit_group)
		debug("exit_group hook\n");
}
/////////////////////////////////////////////////////////////////////

#endif
