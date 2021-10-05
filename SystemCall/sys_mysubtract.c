#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

asmlinkage int sys_mysubtract(int x, int y) {
	printk("sys_mysubtract call\n");
	return x - y;
}

SYSCALL_DEFINE2(mysubtract, int, x, int, y)
{
	return sys_mysubtract(x,y);
}
