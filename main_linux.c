#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/percpu.h>
#include <linux/smp.h>

#include "syscall_hook.h"

int __init main_init(void)
{

	init_hook_syscall();
	on_each_cpu((void (*)(void *))&hook_syscall, NULL, 0);

	return 0;
}
 
void __exit main_cleanup(void)
{
	on_each_cpu((void (*)(void *))&unhook_syscall, NULL, 0);

	printk("Exit\n");

	return;
}

module_init(main_init);
module_exit(main_cleanup);
MODULE_LICENSE("GPL");
