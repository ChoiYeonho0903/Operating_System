#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

asmlinkage int sys_mymultiply(int x, int y) {
	printk("sys_mymultiply call\n");
	return x * y;
}

SYSCALL_DEFINE2(mymultiply, int, x, int, y)
{
	return sys_mymultiply(x,y);
}
