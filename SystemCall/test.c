#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char *x = (char*)malloc(sizeof(char)*20);
    char *y = (char*)malloc(sizeof(char)*20);
    char *operator = (char *)malloc(sizeof(char));
    int xx = 0;
    int yy = 0;
	int answer = 0;
    for (int i = 0; i <strlen(argv[1]); i++)
    {
        if(!isdigit(argv[1][i])) {
            strncpy(x, argv[1], i);
            strncpy(x+i, "\0", 1);
            strncpy(operator, argv[1]+i, 1);
            strncpy(operator+1, "\0", 1);
            strncpy(y, argv[1]+i+1, strlen(argv[1])-i-1);
            strncpy(y+strlen(argv[1])-i-1, "\0", 1);
            break;
        }
    }
    xx = atoi(x);
    yy = atoi(y);
    if(operator[0] =='+') {
		answer = syscall(442,xx,yy);
		printf("%d+%d=%d\n", xx,yy,answer);
	}
	else if(operator[0] =='-') {
		answer = syscall(443, xx, yy);
		printf("%d-%d=%d\n", xx,yy,answer);
	}
	else if(operator[0] == '*') {
		answer = syscall(444, xx, yy);
		printf("%d*%d=%d\n", xx,yy,answer);
	}
	else if(operator[0] == '%') {
		answer = syscall(445, xx, yy);
		printf("%d%%%d=%d\n", xx,yy,answer);
	}
	else {
		printf("잘못된 연산자입니다.");
	}

	return 0;
}
