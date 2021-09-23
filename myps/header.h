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
#include <ncurses.h>

#define BUFFER_SIZE 1024
#define PATH_LEN 1024
#define TOKEN_LEN 32
#define FNAME_LEN 128

#define MAX_TOKEN 22				// /proc/pid/stat에서 읽어들일 token 갯수

#define UNAME_LEN 32
#define TTY_LEN 32
#define STAT_LEN 8
#define TIME_LEN 16
#define CMD_LEN 1024

#define PID_MAX 32768				//pid 최대 갯수

#define PROCESS_MAX 4096

#define SYSTEMD "systemd-"			// system USER명의 앞부분
#define DEVNULL "/dev/null"			// 터미널 없을 시 가리키는 /dev/null 절대 경로
#define PTS "pts/"					// 터미널 문자열의 앞부분

#define PROC "/proc"				// /proc 절대 경로
#define CPUSTAT "/proc/stat"		// /proc/stat 절대 경로
#define DEV "/dev"					// /dev 절대 경로

#define FD__ZERO "/fd/0"			// /proc/pid 에서의 0번째 fd 경로
#define STAT "/stat"				// /proc/pid에서의 stat 경로

// /proc/pid/stat에서의 idx
#define STAT_PID_IDX 0
#define STAT_CMD_IDX 1
#define STAT_STATE_IDX 2
#define STAT_SID_IDX 5
#define STAT_TTY_NR_IDX 6
#define STAT_TPGID_IDX 7
#define STAT_UTIME_IDX 13
#define STAT_STIME_IDX 14
#define STAT_PRIORITY_IDX 17
#define STAT_NICE_IDX 18
#define STAT_N_THREAD_IDX 19
#define STAT_START_TIME_IDX 21

// column에 출력할 문자열
#define PID_STR "PID"
#define TTY_STR "TTY"
#define TIME_STR "TIME"
#define CMD_STR "CMD"

#define TAB_WIDTH 8					//tab 길이

//process를 추상화 한 myProc 구조체
typedef struct{
	unsigned long pid;			//process id
	char tty[TTY_LEN];			//terminal
	char time[TIME_LEN];		//총 cpu 사용 시간
	char cmd[CMD_LEN];			//process가 실행중인 명령	
		
}myProc;


//path에 대한 tty 얻는 함수
void getTTY(char path[PATH_LEN], char tty[TTY_LEN]);

// pid 디렉터리 내의 파일들을 이용해 myProc 완성하는 함수
void add_proc_list(char path[PATH_LEN], unsigned long cpuTimeTable[]);

//	/proc 디렉터리 탐색하는 함수
void search_proc(unsigned long cpuTimeTable[]);

//proc의 내용을 지우는 함수
void erase_proc(myProc *proc);
