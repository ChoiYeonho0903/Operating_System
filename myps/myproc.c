#include "header.h"

extern myProc procList[PROCESS_MAX];
extern int procCnt;
extern int hertz;

extern pid_t myPid;					//자기 자신의 pid
extern uid_t myUid;					//자기 자신의 uid
extern char myPath[PATH_LEN];		//자기 자신의 path
extern char myTTY[TTY_LEN];			//자기 자신의 tty

//path에 대한 tty 얻는 함수
void getTTY(char path[PATH_LEN], char tty[TTY_LEN])
{
	char fdZeroPath[PATH_LEN];			//0번 fd에 대한 절대 경로
	memset(tty, '\0', TTY_LEN);
	memset(fdZeroPath, '\0', TTY_LEN);
	strcpy(fdZeroPath, path);			// /proc/pid
	strcat(fdZeroPath, FD__ZERO);       //#define FD__ZERO "/fd/0"

	if(access(fdZeroPath, F_OK) < 0){	//fd 0이 없을 경우

		char statPath[PATH_LEN];		// /proc/pid/stat에 대한 절대 경로
		memset(statPath, '\0', PATH_LEN);
		strcpy(statPath, path);
		strcat(statPath, STAT);

		FILE *statFp;
		if((statFp = fopen(statPath, "r")) == NULL){	// /proc/pid/stat open
			fprintf(stderr, "fopen error %s %s\n", strerror(errno), statPath);
			sleep(1);
			return;
		}

		char buf[BUFFER_SIZE];
		for(int i = 0; i <= STAT_TTY_NR_IDX; i++){		// 7행까지 read해 TTY_NR 획득
			memset(buf, '\0', BUFFER_SIZE);
			fscanf(statFp, "%s", buf);
		}
		fclose(statFp);

		int ttyNr = atoi(buf);		//ttyNr 정수 값으로 저장

		DIR *dp;
		struct dirent *dentry;
		if((dp = opendir(DEV)) == NULL){		// 터미널 찾기 위해 /dev 디렉터리 open
			fprintf(stderr, "opendir error for %s\n", DEV);
			exit(1);
		}
		char nowPath[PATH_LEN];

		while((dentry = readdir(dp)) != NULL){	// /dev 디렉터리 탐색
			memset(nowPath, '\0', PATH_LEN);	// 현재 탐색 중인 파일 절대 경로
			strcpy(nowPath, DEV);
			strcat(nowPath, "/");
			strcat(nowPath, dentry->d_name);

			struct stat statbuf;
			if(stat(nowPath, &statbuf) < 0){	// stat 획득
				fprintf(stderr, "stat error for %s\n", nowPath);
				exit(1);
			}
			if(!S_ISCHR(statbuf.st_mode))		//문자 디바이스 파일이 아닌 경우 skip
				continue;
			else if(statbuf.st_rdev == ttyNr){	//문자 디바이스 파일의 디바이스 ID가 ttyNr과 같은 경우
				strcpy(tty, dentry->d_name);	//tty에 현재 파일명 복사
				break;
			}
		}
		closedir(dp);

		if(!strlen(tty))					// /dev에서도 찾지 못한 경우
			strcpy(tty, "?");				//nonTerminal
	}
	else{                                   //fd 0이 있는 경우
		char symLinkName[FNAME_LEN];
		memset(symLinkName, '\0', FNAME_LEN);
		if(readlink(fdZeroPath, symLinkName, FNAME_LEN) < 0){
			fprintf(stderr, "readlink error for %s\n", fdZeroPath);
			exit(1);
		}
		if(!strcmp(symLinkName, DEVNULL))		//symbolic link로 가리키는 파일이 /dev/null일 경우
			strcpy(tty, "?");					//nonTerminal
		else
			sscanf(symLinkName, "/dev/%s", tty);	//그 외의 경우 tty 획득
	}
	return;
}

// pid 디렉터리 내의 파일들을 이용해 myProc 완성하는 함수
void add_proc_list(char path[PATH_LEN], unsigned long cpuTimeTable[PID_MAX]){
	
	if(access(path, R_OK) < 0){
		fprintf(stderr, "aceess error for %s\n", path);
		return;
	}
	myProc proc;
	erase_proc(&proc);

	char statPath[PATH_LEN];
	strcpy(statPath, path);
	strcat(statPath, STAT);

	// /proc/pid/stat 파일 읽기
	if(access(statPath, R_OK) < 0){
		fprintf(stderr, "aceess error for %s\n", statPath);
		return;
	}
	FILE *statFp;
	if((statFp = fopen(statPath, "r")) == NULL){
		fprintf(stderr, "fopen error %s %s\n", strerror(errno), statPath);
		sleep(1);
		return;
	}

	char statToken[MAX_TOKEN][TOKEN_LEN];
	memset(statToken, '\0', MAX_TOKEN * TOKEN_LEN);
	for(int i = 0; i < MAX_TOKEN; i++)
		fscanf(statFp, "%s", statToken[i]);
	fclose(statFp);

	//pid 획득
	proc.pid = (long)atoi(statToken[STAT_PID_IDX]);

	//TTY 획득
	getTTY(path, proc.tty);	

	//TIME 획득 (해당 process가 사용한 CPU의 양)
	unsigned long utime = (unsigned long)atoi(statToken[STAT_UTIME_IDX]);
	unsigned long stime = (unsigned long)atoi(statToken[STAT_STIME_IDX]);
	unsigned long long totalTime = utime + stime;
	unsigned long cpuTime = totalTime / hertz;
	struct tm *tmCpuTime= localtime(&cpuTime);
	sprintf(proc.time, "%02d:%02d:%02d", tmCpuTime->tm_hour, tmCpuTime->tm_min, tmCpuTime->tm_sec);
	
	//cmd 획득
	sscanf(statToken[STAT_CMD_IDX], "(%s", proc.cmd);
	proc.cmd[strlen(proc.cmd)-1] = '\0';	//마지막 ')' 제거

	//구조체 변수
	procList[procCnt].pid = proc.pid;
	strcpy(procList[procCnt].tty, proc.tty);
	strcpy(procList[procCnt].time, proc.time);
	strcpy(procList[procCnt].cmd, proc.cmd);
	procCnt++;

	return;
}

//	/proc 디렉터리 탐색하는 함수
void search_proc(unsigned long cpuTimeTable[PID_MAX])
{	
	DIR *dirp;
	if((dirp = opendir(PROC)) == NULL){	// /proc 디렉터리 open
		fprintf(stderr, "dirp error for %s\n", PROC);
		exit(1);
	}
	struct dirent *dentry;
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

		//PID 디렉터리가 아닌 경우 skip
		if(!isPid)				
			continue;
		
		//uid가 자기 자신과 다를 경우 skip
		if(statbuf.st_uid != myUid)		
			continue;
		
		//nonTerminal일 경우 skip
		char tty[TTY_LEN];
		memset(tty, '\0', TTY_LEN);
		getTTY(path, tty);		//TTY 획득
		if(!strlen(tty) || !strcmp(tty, "?"))		
			continue;
		
		//자기 자신과 tty 다를 경우 skip
		memset(tty, '\0', TTY_LEN);
		getTTY(path, tty);		//TTY 획득
		if(strcmp(tty, myTTY))	
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
	memset(proc->tty, '\0', TTY_LEN);
	memset(proc->time, '\0', TIME_LEN);
	memset(proc->cmd, '\0', CMD_LEN);
	return;
}
