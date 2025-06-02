#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Awez");
MODULE_DESCRIPTION("Assignment 1, Problem 5");

static int __init hi(void)
{
	printk(KERN_INFO "Hello Linux, my love <3\n");
	return 0;
}

static void __exit bye(void)
{
	return;
}

module_init(hi);
module_exit(bye);
