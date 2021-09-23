#include "header.h"

//kib 단위에서 mib로 변환
myProc proc;
int kib_to_mib(int kib) {
	return kib/1024;	
}

//구조체 변수 구하기
void add_proc(void) {
	FILE *fp;
	char buf[BUFFER_SIZE];
	
	if((fp = fopen("/proc/cpuinfo", "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", "/proc/cpuinfo");
		exit(1);
	}
	int cnt = 0;

	//vendor ID 획득
	while(cnt < VENDOR_ID) {
		memset(buf, '\0', BUFFER_SIZE);
		fgets(buf, BUFFER_SIZE, fp);
		cnt++;
	}
	char *ptr = buf;
	for(int i=0; i<strlen(buf); i++) {
		if(buf[i]==':') {
			ptr = buf+i+1;
			break;
		}
	}
	sscanf(ptr, "%s", proc.venId);

	//CPU_MODEL_NAME 획득
	while(cnt < CPU_MODEL_NAME) {
		memset(buf, '\0', BUFFER_SIZE);
		fgets(buf, BUFFER_SIZE, fp);
		cnt++;
	}

	ptr = buf;
	for(int i=0; i<strlen(buf); i++) {
		if(buf[i]==':') {
			ptr = buf+i+1;
			break;
		}
	}

	memcpy(proc.cpuName, ptr, strlen(ptr)-1);

	//CPU 속도 획득
		while(cnt < CPU_MHZ) {
		memset(buf, '\0', BUFFER_SIZE);
		fgets(buf, BUFFER_SIZE, fp);
		cnt++;
	}

	ptr = buf;
	for(int i=0; i<strlen(buf); i++) {
		if(buf[i]==':') {
			ptr = buf+i+1;
			break;
		}
	}
	sscanf(ptr, "%s", proc.cpuHz);


	//Cache Size 획득
	proc.cache[0]=0;
	proc.cache[1]=0;
	proc.cache[2]=0;
	proc.cache[3]=0;

	DIR *dirp;
	if((dirp = opendir("/sys/devices/system/cpu"))==NULL) {
		fprintf(stderr, "dirp error for %s\n", "/sys/devices/system/cpu");
		exit(1);
	}
	struct dirent *dentry;
	while((dentry = readdir(dirp)) != NULL){	// /proc 디렉터리 내 하위 파일들 탐색 시작

		char path[PATH_LEN];			//디렉터리의 절대 경로 저장
		memset(path, '\0', PATH_LEN);
		strcpy(path, "/sys/devices/system/cpu");
		strcat(path, "/");
		strcat(path, dentry->d_name);

		struct stat statbuf;
		if(stat(path, &statbuf) < 0){	//디렉터리의 stat 획득
			fprintf(stderr, "stat error for %s\n", path);
			exit(1);
		}

		if(!S_ISDIR(statbuf.st_mode))	//디렉터리가 아닐 경우 skip
			continue;

		int len = strlen(dentry->d_name);
		bool isCpu = false;
		for(int i = 0; i < len; i++){
			if(isdigit(dentry->d_name[i])){	//디렉터리명 중 숫자 있는 경우
				isCpu = true;
				break;
			}
		}
		if(!isCpu)				//PID 디렉터리가 아닌 경우 skip
			continue;
		
		char *search =  "cpu";
		if(strstr(dentry->d_name, search)!=NULL) {  //"cpu" 문자열이 들어있는 directory찾기
			strcat(path, "/cache");
			for(int i=0; i<4; i++) {
				char cpath[BUFFER_SIZE];
				char str[BUFFER_SIZE];
				char ptr[BUFFER_SIZE];
				memset(cpath, '\0', BUFFER_SIZE);
				memset(str, '\0', BUFFER_SIZE);
				memset(ptr, '\0', BUFFER_SIZE);
				strcat(cpath, path);
				sprintf(str, "/index%d/size", i);
				strcat(cpath, str);
				FILE *fp;
				if ((fp = fopen(cpath, "r")) == NULL){	
					fprintf(stderr, "fopen error for %s\n", cpath);
        			exit(1);
    			}
				fgets(ptr, BUFFER_SIZE, fp);
				int cach = 0;
				sscanf(ptr, "%dK", &cach);
				proc.cache[i] += cach;
				if(proc.cache[i]>10000) {
					proc.cache[i] = kib_to_mib(proc.cache[i]);
				}
			}
		}
	}
	
}


int main(void) {
	add_proc();
	printf("Vender ID: %s\n", proc.venId);
	printf("Model name:%s\n", proc.cpuName);
	printf("CPU MHz: %s\n", proc.cpuHz);
	printf("L1d cache: %d KiB\n", proc.cache[0]);
	printf("L1i cache: %d kiB\n", proc.cache[1]);
	printf("L2 cache: %d KiB\n", proc.cache[2]);
	printf("L3 cache: %d MiB\n", proc.cache[3]);
	return 0;
}
