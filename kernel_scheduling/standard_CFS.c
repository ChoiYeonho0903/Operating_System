#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define Process_Num 21

int value[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}};
int output[3][3];

int main(int argc, char *argv[]) {
	int ret;
	int pid;
	int status;

	for (int i=0; i<Process_Num; i++) {
		pid = fork();
		if(pid < 0) {
			printf("fork Fail\n");
		}
		else if(pid > 0) {
			printf("%d process begins\n", pid);			
		}
		else {
			for(int t=0; t<500000; t++) {
				for(int i=0; i<3; i++) {
					for(int j=0; j<3; j++) {
						for(int k=0; k<3; k++) {
							output[i][j] += value[i][k] * value[k][j];
						}
					}
				}
			}
			exit(1);
		}
	}
	for (int i=0; i<Process_Num; i++) {
		ret = wait(&status);
		printf("%d process ends\n", ret);
	}
	printf("------------All processes end------------\n");
	return 0;
}
