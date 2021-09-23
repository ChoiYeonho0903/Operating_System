#include "header.h"

#define COLUMN_CNT 4	//출력할 column 최대 갯수

//출력 시 사용할 columnWidth배열에서의 index

#define PID_IDX 0
#define TTY_IDX 1
#define TIME_IDX 2
#define CMD_IDX 3

myProc procList[PROCESS_MAX];	//완성한 myProc의 포인터 저장 배열
int procCnt = 0;				//현재까지 완성한 myProc 갯수
unsigned int hertz;	 			//os의 hertz값 얻기(초당 context switching 횟수)

pid_t myPid;					//자기 자신의 pid
uid_t myUid;					//자기 자신의 uid
char myPath[PATH_LEN];			//자기 자신의 path
char myTTY[TTY_LEN];			//자기 자신의 tty

int termWidth;					//현재 터미널 너비

//실제 화면에 출력하는 함수
void print_pps(void) {

	int columnWidth[COLUMN_CNT] = {					//column의 x축 길이 저장하는 배열
		strlen(PID_STR), strlen(TTY_STR), strlen(TIME_STR), strlen(CMD_STR)};

	char buf[BUFFER_SIZE];

	for(int i = 0; i < procCnt; i++){			//PID 최대 길이 저장
		sprintf(buf, "%lu", procList[i].pid);
		if(columnWidth[PID_IDX] < strlen(buf))
			columnWidth[PID_IDX] = strlen(buf);
	}
	for(int i = 0; i < procCnt; i++){			//TTY 최대 길이 저장
		if(columnWidth[TTY_IDX] < strlen(procList[i].tty))
			columnWidth[TTY_IDX] = strlen(procList[i].tty);
	}
	for(int i = 0; i < procCnt; i++){			//TIME 최대 길이 저장
		if(columnWidth[TIME_IDX] < strlen(procList[i].time))
			columnWidth[TIME_IDX] = strlen(procList[i].time);
	}
	for(int i = 0; i < procCnt; i++){			//CMD 최대 길이 저장
		if(columnWidth[CMD_IDX] < strlen(procList[i].cmd))
			columnWidth[CMD_IDX] = strlen(procList[i].cmd);
	}
	
	/*****		column 출력 시작	*****/
	int gap = 0;
	memset(buf, '\0', BUFFER_SIZE);	
	strcat(buf, " ");
	strcat(buf, " ");

	//PID 출력
	gap = columnWidth[PID_IDX] - strlen(PID_STR);	//PID의 길이 차 구함
	for(int i = 0; i < gap; i++)					//PID 우측 정렬
		strcat(buf, " ");
	strcat(buf, PID_STR);

	strcat(buf, " ");

	//TTY 출력
	gap = columnWidth[TTY_IDX] - strlen(TTY_STR);	//TTY의 길이 차 구함
	strcat(buf, TTY_STR);
	for(int i = 0; i < gap; i++)				//TTY 좌측 정렬
		strcat(buf, " ");

	strcat(buf, " ");

	strcat(buf, " ");
	strcat(buf, " ");
	strcat(buf, " ");

	//TIME 출력
	gap = columnWidth[TIME_IDX] - strlen(TIME_STR);	//TIME의 길이 차 구함
	for(int i = 0; i < gap; i++)					//TIME 우측 정렬
		strcat(buf, " ");
	strcat(buf, TIME_STR);

	strcat(buf, " ");

	//COMMAND 또는 CMD 출력
	strcat(buf, CMD_STR);				

	buf[COLS] = '\0';							//터미널 너비만큼 잘라 출력
	printf("%s\n", buf);

	/*****		column 출력 종료	*****/

	/*****		process 출력 시작	*****/
	char token[TOKEN_LEN];
	memset(token, '\0', TOKEN_LEN);

	for(int i = 0; i < procCnt; i++){
		memset(buf, '\0', BUFFER_SIZE);
		strcat(buf, " ");
		strcat(buf, " ");

		//PID 출력
		memset(token, '\0', TOKEN_LEN);
		sprintf(token, "%lu", procList[i].pid);
		gap = columnWidth[PID_IDX] - strlen(token);		//PID의 길이 차 구함
		for(int i = 0; i < gap; i++)				//PID 우측 정렬
			strcat(buf, " ");
		strcat(buf, token);

		strcat(buf, " ");

		//TTY 출력
		gap = columnWidth[TTY_IDX] - strlen(procList[i].tty);	//TTY의 길이 차 구함
		strcat(buf, procList[i].tty);
		for(int i = 0; i < gap; i++)						//TTY 좌측 정렬
			strcat(buf, " ");

		strcat(buf, " ");

		strcat(buf, " ");
		strcat(buf, " ");
		strcat(buf, " ");

		//TIME 출력
		gap = columnWidth[TIME_IDX] - strlen(procList[i].time);	//TIME의 길이 차 구함
		for(int i = 0; i < gap; i++)				//TIME 우측 정렬
			strcat(buf, " ");
		strcat(buf, procList[i].time);

		strcat(buf, " ");

		//CMD 출력
		strcat(buf, procList[i].cmd);

		buf[COLS] = '\0';							//터미널 너비만큼만 출력
		printf("%s\n", buf);
	
	}

	/*****		process 출력 종료	*****/

	return;	
}

int main(int argc, char *argv[]) {

	hertz = (unsigned int)sysconf(_SC_CLK_TCK);	//초당 context switching 횟수
	myPid = getpid();			//자기 자신의 pid
	char pidPath[FNAME_LEN];
	memset(pidPath, '\0', FNAME_LEN);
	sprintf(pidPath, "/%d", myPid);

	strcpy(myPath, PROC);			//자기 자신의 /proc 경로 획득
	strcat(myPath, pidPath);

	getTTY(myPath, myTTY);			//자기 자신의 tty 획득
	for(int i = strlen(PTS); i < strlen(myTTY); i++)
		if(!isdigit(myTTY[i])){
			myTTY[i] = '\0';
			break;
		}

	myUid = getuid();			//자기 자신의 uid

	initscr();				//출력 윈도우 초기화
	termWidth = COLS;		//term 너비 획득
	endwin();				//출력 윈도우 종료

	search_proc(NULL);

	print_pps();

	return 0;
}
