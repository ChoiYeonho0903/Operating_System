#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>

asmlinkage int sys_myadd(int x, int y) {
	printk("sys_myadd call\n");
	return x + y;
}

SYSCALL_DEFINE2(myadd, int, x, int, y)
{
	return sys_myadd(x,y);
}
