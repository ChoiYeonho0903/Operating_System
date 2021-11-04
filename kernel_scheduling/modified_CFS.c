#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define Process_Num 21

int value[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}};
int output[3][3];

int main(int argc, char *argv[]) {
	int pid;
	int status;
	int wpid;

	for (int i=0; i<Process_Num; i++) {
		pid = fork();
		if(pid < 0) {
			printf("fork Fail\n");
		}
		else if(pid > 0) {
			printf("%d process begins\n", pid);			
		}
		else {
			if(i<7) {
				int num = nice(19);
				for(int t=0; t<250000; t++) {
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
			else if(i<14) {
				int num = nice(9);
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
			else {
				int num = nice(0);
				for(int t=0; t<1000000; t++) {
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
	}
	for (int i=0; i<Process_Num; i++) {
		wpid = wait(&status);
		printf("%d process ends\n", wpid);
	}
	printf("------------All processes end------------\n");
	return 0;
}
