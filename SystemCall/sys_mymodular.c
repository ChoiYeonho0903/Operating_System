#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

asmlinkage int sys_mymodular(int x, int y) {
	printk("sys_mymodular call\n");
	return x % y;
}

SYSCALL_DEFINE2(mymodular, int, x, int, y)
{
	return sys_mymodular(x,y);
}
