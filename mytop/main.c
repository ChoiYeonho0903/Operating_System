#include "header.h"

unsigned long cpuTimeTable[PID_MAX];	//cpu의 이전 시각 저장할 hash table

myProc procList[PROCESS_MAX];	//완성한 myProc의 포인터 저장 배열

myProc *sorted[PROCESS_MAX];	//procList를 cpu 순으로 sorting한 myProc 포인터 배열

int procCnt = 0;				//현재까지 완성한 myProc 갯수

time_t before;
time_t now;

unsigned long uptime;			//os 부팅 후 지난 시간
unsigned long beforeUptime = 0;	//이전 실행 시의 os 부팅 후 지난 시각
unsigned long memTotal;			//전체 물리 메모리 크기
unsigned int hertz;	 			//os의 hertz값 얻기(초당 context switching 횟수)

long double beforeTicks[TICK_CNT] = {0, };	//이전의 cpu ticks 저장하기 위한 배열

pid_t myPid;					//자기 자신의 pid
uid_t myUid;					//자기 자신의 uid
char myPath[PATH_LEN];			//자기 자신의 path
char myTTY[TTY_LEN];			//자기 자신의 tty

int ch;
int row, col;

int main(int argc, char *argv[]) {

	memTotal = get_mem_total();					//전체 물리 메모리 크기
	hertz = (unsigned int)sysconf(_SC_CLK_TCK);	//os의 hertz값 얻기(초당 context switching 횟수)
	now = time(NULL);

	memset(cpuTimeTable, (unsigned long)0, PID_MAX);

	initscr();				//출력 윈도우 초기화
	halfdelay(10);			//0.1초마다 입력 새로 갱신
	noecho();				//echo 제거
	keypad(stdscr, TRUE);	//특수 키 입력 허용 (방향키 작동과 q입력시 종료)
	curs_set(0);			//curser invisible

	search_proc(cpuTimeTable);

	row = 0;
	col = 0;

	ch = 0;

	bool print = false;
	pid_t pid;

	before = time(NULL);

	sort_by_cpu();			//cpu 순으로 정렬
	print_ttop();			//초기 출력
	refresh();

	do{						//무한 반복
		now = time(NULL);	//현재 시각 갱신

		switch(ch){			//방향키 입력 좌표 처리
			case KEY_LEFT:
				col--;
				if(col < 0)
					col = 0;
				print = true;
				break;
			case KEY_RIGHT:
				col++;
				print = true;
				break;
			case KEY_UP:
				row--;
				if(row < 0)
					row = 0;
				print = true;
				break;
			case KEY_DOWN:
				row++;
				if(row > procCnt)
					row = procCnt;
				print = true;
				break;
		}

		if(print || now - before >= 3){	//3초 경과 시 화면 갱신
			erase();
			erase_proc_list();
			search_proc(cpuTimeTable);
			sort_by_cpu();			//cpu 순으로 정렬
			print_ttop();
			refresh();
			before = now;
			print = false;
		}

	}while((ch = getch()) != 'q');	//q 입력 시 종료

	endwin();

	return 0;
}	

