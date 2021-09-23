#include "header.h"

extern myProc procList[PROCESS_MAX];
extern int procCnt;
extern unsigned long uptime;
extern time_t before;
extern time_t now;
extern unsigned long memTotal;
extern int hertz;

extern pid_t myPid;					//자기 자신의 pid
extern uid_t myUid;					//자기 자신의 uid
extern char myPath[PATH_LEN];		//자기 자신의 path

//src를 소숫점 아래 rdx+1자리에서 반올림하는 함수
long double round_double(long double src, int rdx)
{
	if(!rdx)
		return (long double)((unsigned long long)src);
	long double tmp = src;
	for(int i = 0; i <= rdx; i++)	//소숫점 아래 rdx+1번째 값 구하기
		tmp *= 10;
	int val = (unsigned long long)tmp % 10;	//소숫점 아래 rdx+1번째 값

	tmp /= 10;			//소숫점 아래 rdx번째까지만 소숫점 위로 표현

	tmp = (long double)((unsigned long long )tmp);	//rdx 밑의 자릿수 값 버리기

	if(val >= 5)		//반올림 o
		tmp += 1;

	for(int i = 0; i < rdx; i++)	//원상 복구
		tmp /= 10;

	return tmp;
}

//Kib 단위를 Kb로 변환시키는 함수
unsigned long kib_to_kb(unsigned long kib)
{
	unsigned long kb = kib * 1024 / 1000;
	return kb;
}

// /proc/uptime에서 OS 부팅 후 지난 시간 얻는 함수
unsigned long get_uptime(void)
{
    FILE *fp;
    char buf[BUFFER_SIZE];
    long double time;

	memset(buf, '\0', BUFFER_SIZE);

    if ((fp = fopen("/proc/uptime", "r")) == NULL){	
		fprintf(stderr, "fopen error for %s\n", "/proc/uptime");
        exit(1);
    }
    fgets(buf, BUFFER_SIZE, fp);
    sscanf(buf, "%Lf", &time);	// /proc/uptime의 첫번째 double 읽기
    fclose(fp);

    return (unsigned long)time;
}

// /proc/meminfo에서 전체 물리 메모리 크기 얻는 함수
unsigned long get_mem_total(void)
{
    FILE *fp;
    char buf[BUFFER_SIZE];
	unsigned long memTotal;

    if ((fp = fopen("/proc/meminfo", "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", "/proc/meminfo");
        exit(1);
    }
	int i = 0;
	while(i < MEMINFO_MEM_TOTAL_ROW){	// memTotal read
		memset(buf, '\0', BUFFER_SIZE);
		fgets(buf, BUFFER_SIZE, fp);
		i++;
	}
	char *ptr = buf;
	while(!isdigit(*ptr)) ptr++;
    sscanf(ptr, "%lu", &memTotal);	// /proc/meminfo의 1행에서 memTotal read
    fclose(fp);

	memTotal = kib_to_kb(memTotal);	//Kib 단위를 Kb로 변환

    return memTotal;
}

// pid 디렉터리 내의 파일들을 이용해 myProc 완성하는 함수
void add_proc_list(char path[PATH_LEN], unsigned long cpuTimeTable[PID_MAX]){

	if(access(path, R_OK) < 0) {
		fprintf(stderr, "access error for %s|n", path);
		return;
	}

	//myProc 구조체 변수 선언
	myProc proc;
	erase_proc(&proc);

	char statPath[PATH_LEN];
	strcpy(statPath, path);	// /proc/pid
	strcat(statPath, STAT); // /proc/pid/stat

	if(access(statPath, R_OK) < 0) {
		fprintf(stderr, "aceess error for %s\n", statPath);
		return;
	}

	//stat파일 읽기
	FILE *statFp;
	if((statFp = fopen(statPath, "r")) == NULL) {
		fprintf(stderr, "fopen error %s %s\n", strerror(errno), statPath);
		return;	
	}

	//MAX_TOKEN:22 -> /proc/pid/stat에서 읽어들일 token수
	//TOKEN_LEN:32
	char statToken[MAX_TOKEN][TOKEN_LEN];
	memset(statToken, '\0', MAX_TOKEN * TOKEN_LEN);
	for(int i = 0; i < MAX_TOKEN; i++)
		fscanf(statFp, "%s", statToken[i]);
	fclose(statFp);

	//pid 획득
	proc.pid = (long)atoi(statToken[STAT_PID_IDX]); //#define STAT_PID_IDX 0

	//user명 획득
	struct stat statbuf;
	if(stat(statPath, &statbuf) < 0){
		fprintf(stderr, "stat error for %s\n", statPath);
		return;
	}
	proc.uid = statbuf.st_uid;
	struct passwd *upasswd = getpwuid(statbuf.st_uid);

	char tmp[UNAME_LEN];
	strcpy(tmp, upasswd->pw_name);
	tmp[strlen(SYSTEMD)] = '\0';
	if(!strcmp(tmp, SYSTEMD)){				//user명이 systemd-로 시작할 경우
		tmp[strlen(SYSTEMD)-1] = '+';		//systemd+를 user명으로 저장
		strcpy(proc.user, tmp);
	}
	else									//user명이 systemd-로 시작하지 않을 경우 그대로 저장
		strcpy(proc.user, upasswd->pw_name);
	
	//%CPU 계산
	unsigned long utime = (unsigned long)atoi(statToken[STAT_UTIME_IDX]); //#define STAT_UTIME_IDX 13
	unsigned long stime = (unsigned long)atoi(statToken[STAT_UTIME_IDX]);
	unsigned long startTime = (unsigned long)atoi(statToken[STAT_START_TIME_IDX]);
	unsigned long long totalTime = utime + stime;
	long double cpu = 0;
	bool update = false;
	if(cpuTimeTable != NULL){			//ttop일 경우
		if(cpuTimeTable[proc.pid] != 0){		//이전에 실행된 내역이 있을 경우 process 시작 시각 이후의 값이 아닌 이전 실행 이후의 값 불러옴
			update = true;
			cpu = (totalTime-cpuTimeTable[proc.pid]) / (long double)(now - before) / hertz * 100;	//%cpu 갱신
		}
		cpuTimeTable[proc.pid] = totalTime;		//현재까지의 process 사용 내역 저장
	}
	if(!update)
		cpu = ((totalTime) / hertz) / (long double)(uptime-(startTime/hertz)) * 100;	//%cpu 계산
	if(isnan(cpu) || isinf(cpu) || cpu > 100 || cpu < 0)	// error 처리
		proc.cpu = 0.0;
	else
		proc.cpu = round_double(cpu, 2);


	char statusPath[PATH_LEN];				
	memset(statusPath, '\0', PATH_LEN);
	strcpy(statusPath, path); // /proc/pid
	strcat(statusPath, STATUS); // /proc/pid/status

	FILE *statusFp;
	unsigned long vmLck = 0;

	//status 파일 읽기
	if((statusFp = fopen(statusPath, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", statusPath);
		return;
	}

	char buf[BUFFER_SIZE];
	int cnt = 0;
	while(cnt < STATUS_VSZ_ROW) { 					//18 line까지 read
		memset(buf, '\0', BUFFER_SIZE);
		fgets(buf, BUFFER_SIZE, statusFp);
		cnt++;
	}
	
	//숫자정보 slicing
	char *ptr = buf;
	for(int i = 0; i < strlen(buf); i++) {
		if(isdigit(buf[i])){
			ptr = buf+i;
			break;
		}
	}

	buf[strlen(VMSIZE)] = '\0';
	// /proc/pid/status에 메모리 정보는 없는 경우
	if(strcmp(buf, VMSIZE)) {
		proc.vsz = 0;
		proc.rss = 0;
		proc.shr = 0;
		proc.mem = 0.0;
	}
	else { // /proc/pid/status에 메모리 정보가 있는 경우
				
		sscanf(ptr, "%lu", &proc.vsz);				//18행에서 vsz 획득
		
		
		while(cnt < STATUS_VMLCK_ROW){				//19 line까지 read
			memset(buf, '\0', BUFFER_SIZE);
			fgets(buf, BUFFER_SIZE, statusFp);
			cnt++;
		}

		ptr = buf;
		for(int i = 0; i < strlen(buf); i++) {
			if(isdigit(buf[i])){
				ptr = buf+i;
				break;
			}
		}
		//state에서 사용할 vmLck read
		vmLck = 0;									
		sscanf(ptr, "%lu", &vmLck);					//19행에서 vmLck 획득

		while(cnt < STATUS_RSS_ROW){				//22행까지 read
			memset(buf, '\0', BUFFER_SIZE);
			fgets(buf, BUFFER_SIZE, statusFp);
			cnt++;
		}
		ptr = buf;
		for(int i = 0; i < strlen(buf); i++)
			if(isdigit(buf[i])){
				ptr = buf+i;
				break;
			}
		sscanf(ptr, "%lu", &proc.rss);				//22행에서 rss 획득

		while(cnt < STATUS_SHR_ROW){				//22행까지 read
			memset(buf, '\0', BUFFER_SIZE);
			fgets(buf, BUFFER_SIZE, statusFp);
			cnt++;
		}
		ptr = buf;
		for(int i = 0; i < strlen(buf); i++)
			if(isdigit(buf[i])){
				ptr = buf+i;
				break;
			}
		sscanf(ptr, "%lu", &proc.shr);				//24행에서 shr 획득
		
		//%MEM(RAM에서 RES가 차지하는 비율) 계산
		long double mem = (long double)proc.rss / memTotal * 100;
		if(isnan(mem) || isinf(mem) || mem > 100 || mem < 0)
			proc.mem = 0.0;
		else
			proc.mem = round_double(mem, 2);
	}
	fclose(statusFp);

	
	proc.priority = atoi(statToken[STAT_PRIORITY_IDX]);	//priority 획득
	proc.nice = atoi(statToken[STAT_NICE_IDX]);			//nice 획득
	strcpy(proc.stat, statToken[STAT_STATE_IDX]);		//STATE 획득

	//START 획득
	unsigned long start = time(NULL) - uptime + (startTime/hertz);
	struct tm *tmStart= localtime(&start);
	if(time(NULL) - start < 24 * 60 * 60){
		strftime(proc.start, TIME_LEN, "%H:%M", tmStart);
	}
	else if(time(NULL) - start < 7 * 24 * 60 * 60){
		strftime(proc.start, TIME_LEN, "%b %d", tmStart);
	}
	else{
		strftime(proc.start, TIME_LEN, "%y", tmStart);
	}
	
	//TIME 획득
	unsigned long cpuTime = totalTime / hertz;
	struct tm *tmCpuTime= localtime(&cpuTime);
	sprintf(proc.time, "%02d:%02d:%02d", tmCpuTime->tm_hour, tmCpuTime->tm_min, tmCpuTime->tm_sec);

	//command 획득
	sscanf(statToken[STAT_CMD_IDX], "(%s", proc.cmd);
	proc.cmd[strlen(proc.cmd)-1] = '\0';	//마지막 ')' 제거

	//procList에 저장
	procList[procCnt].pid = proc.pid;
	procList[procCnt].uid = proc.uid;
	strcpy(procList[procCnt].user, proc.user);
	procList[procCnt].cpu = proc.cpu;
	procList[procCnt].mem = proc.mem;
	procList[procCnt].vsz = proc.vsz;
	procList[procCnt].rss = proc.rss;
	procList[procCnt].shr = proc.shr;
	procList[procCnt].priority = proc.priority;
	procList[procCnt].nice = proc.nice;
	strcpy(procList[procCnt].tty, proc.tty);
	strcpy(procList[procCnt].stat, proc.stat);
	strcpy(procList[procCnt].start, proc.start);
	strcpy(procList[procCnt].time, proc.time);
	strcpy(procList[procCnt].cmd, proc.cmd);
	procCnt++;

	return;
}

//	/proc 디렉터리 탐색하는 함수
void search_proc(unsigned long cpuTimeTable[PID_MAX])
{
	uptime = get_uptime();
	DIR *dirp;
	if((dirp = opendir(PROC)) == NULL){	// /proc 디렉터리 open
		fprintf(stderr, "dirp error for %s\n", PROC);
		exit(1);
	}
	struct dirent *dentry; //디렉터리 구조체
	while((dentry = readdir(dirp)) != NULL){	// /proc 디렉터리 내 하위 파일들 탐색 시작

		char path[PATH_LEN];			//디렉터리의 절대 경로 저장
		memset(path, '\0', PATH_LEN);
		strcpy(path, PROC);
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
		bool isPid = true;
		for(int i = 0; i < len; i++){	//디렉터리가 PID인지 찾기
			if(!isdigit(dentry->d_name[i])){	//디렉터리명 중 숫자 아닌 문자가 있을 경우
				isPid = false;
				break;
			}
		}
		if(!isPid)				//PID 디렉터리가 아닌 경우 skip
			continue;
		add_proc_list(path, cpuTimeTable);	//PID 디렉터리인 경우 procList에 추가
	}
	closedir(dirp);
	return;
}

//proc의 내용을 지우는 함수
void erase_proc(myProc *proc)
{
	proc->pid = 0;
	proc->uid = 0;
	memset(proc->user, '\0', UNAME_LEN);
	proc->cpu = 0.0;
	proc->mem = 0.0;
	proc->vsz = 0;
	proc->rss = 0;
	proc->shr = 0;
	proc->priority = 0;
	proc->nice = 0;
	memset(proc->tty, '\0', TTY_LEN);
	memset(proc->stat, '\0', STAT_LEN);
	memset(proc->start, '\0', TIME_LEN);
	memset(proc->time, '\0', TIME_LEN);
	memset(proc->cmd, '\0', CMD_LEN);
	return;
}

// procList 내용 지우는 함수
void erase_proc_list(void)
{
	for(int i = 0; i < procCnt; i++)
		erase_proc(procList + i);
	procCnt = 0;
	return;
}
