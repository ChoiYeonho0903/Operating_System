#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
	int x = 10;
	int y = 3;
	int n1 = syscall(442, x, y);
	int n2 = syscall(443, x, y);
	int n3 = syscall(444, x, y);
	int n4 = syscall(445, x, y);

	printf("%d+%d=%d\n", x,y,n1);
	printf("%d-%d=%d\n", x,y,n2);
	printf("%d*%d=%d\n", x,y,n3);
	printf("%d%%%d=%d\n", x,y,n4);

	return 0;
}
