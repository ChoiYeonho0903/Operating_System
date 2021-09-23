#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <utmp.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define PATH_LEN 1024
#define VENDOR_ID 2
#define CPU_MODEL_NAME 5
#define CPU_MHZ 8

typedef struct {
	char venId[1024];       //Vendor ID
	char cpuName[1024];		//CPU MODEL 이름	
	char cpuHz[1024];   	//CPU 속도
	int cache[4]; 			//cache size
}myProc;

void add_proc(void);
