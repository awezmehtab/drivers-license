#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Awez");
MODULE_DESCRIPTION("Assignment 1, Problem 6");

static int __init hi(void)
{
	struct task_struct* curr;
	for_each_process(curr) {
		printk(KERN_INFO "PID: %d\tState: %ld\tName: %s\n",
				curr->pid,
				curr->state,
				curr->comm);
	}
	return 0;
}

static void __exit bye(void)
{
	printk(KERN_INFO "PS's job is over!\n");
}

module_init(hi);
module_exit(bye);
