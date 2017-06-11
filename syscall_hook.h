#ifndef SYSCALL_HOOK_H
#define SYSCALL_HOOK_H

#define debug(fmt, args...)	pr_info("CPU %d " fmt, smp_processor_id(), ##args)
#define alert(fmt, args...)	pr_alert("CPU %d " fmt, smp_processor_id(), ##args)


#define MSR_LSTAR	0xc0000082
void (*syscall_handler)(void);
long unsigned int original_dispatcher;
extern void __fake_syscall_dispatcher(void);

static inline uint64_t get_dispatcher_from_msr(void)
{
	uint32_t low = 0, high = 0;
	uint64_t address;

	rdmsr(MSR_LSTAR,low, high);
	address = 0;
	address |= high;
	address = address << 32;
	address |= low;

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

	uint32_t low = 0, dummy = 0, high = 0;

	rdmsr(MSR_LSTAR, dummy, high);
	low = (uint32_t) (((uint64_t)__fake_syscall_dispatcher) & 0xffffffff);

	wrmsr(MSR_LSTAR, low, high);
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
#define CLONE_NUM	56
#define FORK_NUM	57
#define VFORK_NUM	58
#define EXECVE_NUM	59
#define EXIT_NUM	60
#define EXIT_GROUP_NUM	231

void pre_hooking(int num,  void *arg1, void *arg2)
{
	if (get_cpu_var(syscall_num) == CLONE_NUM && num == CLONE_NUM)
		debug("clone hook\tchild_stack: %p\n", arg2);
	else if (get_cpu_var(syscall_num) == FORK_NUM && num == FORK_NUM)
		debug("frok hook\n");
	else if (get_cpu_var(syscall_num) == VFORK_NUM && num == VFORK_NUM)
		debug("vfrok hook\n");
	else if (get_cpu_var(syscall_num) == EXECVE_NUM  && num == EXECVE_NUM)
		debug("execve hook: %s\n", arg1);
	else if (get_cpu_var(syscall_num) == EXIT_NUM && num == EXIT_NUM)
		debug("exit hook\n");
	else if (get_cpu_var(syscall_num) == EXIT_GROUP_NUM && num == EXIT_GROUP_NUM)
		debug("exit_group hook\n");
}
/////////////////////////////////////////////////////////////////////

#endif
