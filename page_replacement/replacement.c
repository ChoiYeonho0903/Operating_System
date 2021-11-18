#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FRAME_SIZE 4
#define MAX_REFER_STR_SIZE 30

char file_name[1024];
FILE *fp;
char *method[] = {"OPT", "FIFO", "LRU", "Second-Chance"};
int refer_str[MAX_REFER_STR_SIZE]; //page reference string
int frame_num; //page frame 개수
int *frame_arr; //page frame 배열
int *order; //page frame victim 판단 배열
int *refer_bit; //second chance에서 사용하는 reference bit
char tmp;
int refer_str_size = 0;
int page_fault_num = 0;


int pageFault(int item);
int pageSearch();
void print(int time, int page_fault);
int opt(int item, int index);
int fifo(int item);
int lru(int item);
int secondChance(int item);

int main(int argc, char *argv[]) {

	printf("input 파일의 이름을 입력하세요: ");
	scanf("%s", file_name);
	if((fp = fopen(file_name, "r"))== NULL) {
		printf("ERROR - cannot open file\n");
	}
	
	//input 파일 읽기
	fseek(fp, 0, SEEK_SET);
	fread(&tmp, sizeof(char), 1, fp);
	frame_num = tmp - '0';
	while(!feof(fp)) {
		tmp = getc(fp);
		if(isdigit(tmp)) {
			refer_str[refer_str_size] = tmp - '0';
			refer_str_size++;
		}
	}
	fclose(fp);	

	frame_arr = (int *)malloc(sizeof(int)*frame_num);
	order = (int *)malloc(sizeof(int)*frame_num);

	for(int i=0; i<4; i++) {
		memset(frame_arr, -1, frame_num * sizeof(int));
		memset(order, 0, frame_num * sizeof(int));
		int time = 1;
		page_fault_num = 0;

		printf("Used method : %s\n", method[i]);
		printf("page reference string : ");
		for(int j=0; j<refer_str_size; j++) {
			printf("%d ", refer_str[j]);
		}
		printf("\n\n");
		printf("        frame   1         2         3         page fault\n");
		printf("time\n");

		if(i==0) {
			for(int j=0; j<refer_str_size; j++) {
				int page_fault = opt(refer_str[j], j);
				print(time, page_fault);
				time++;
			}
		}
		else if(i==1) {
			for(int j=0; j<refer_str_size; j++) {
				int page_fault = fifo(refer_str[j]);
				print(time, page_fault);
				time++;
			}
		}
		else if(i==2) {
			for(int j=0; j<refer_str_size; j++) {
				int page_fault = lru(refer_str[j]);
				print(time, page_fault);
				time++;
			}
		}
		else {
			refer_bit = (int*)malloc(sizeof(int)*frame_num);
			memset(refer_bit, 0, frame_num * sizeof(int));
			for(int j=0; j<refer_str_size; j++) {
				int page_fault = secondChance(refer_str[j]);
				print(time, page_fault);
				time++;
			}
		}
	}
}

int pageFault(int item) {
	int flag = -1; //page fault 발생시 -1 return
	for(int i=0; i<frame_num; i++) {
		if(frame_arr[i]==item){
			flag = i; //hit 발생시 frame 번호 return
		}
	}
	return flag;
}

int pageSearch() {
	int flag = -1; //비어있는 page가 없다면 -1 return
	for(int i=0; i<frame_num; i++) {
		if(frame_arr[i]==-1) {
			flag = i; //비어있는 page가 있다면 frame 번호 return
			break;
		}
	}
	return flag;
}

void print(int time, int page_fault) {
	char str;
	if(page_fault < 0) {
		str = 'F';
	}
	else {
		str = ' ';
	}
	printf("%-16d", time);
	for(int i=0; i<frame_num; i++) {
		if(frame_arr[i]<0) {
			printf("%-10c", ' ');
		}
		else {
			printf("%-10d", frame_arr[i]);
		}
	}
	printf("%c\n", str);

	if(time == refer_str_size) {
		printf("Number of page faults : %d times\n", page_fault_num);
		printf("\n\n");
	}
}

int opt(int item, int index) {
	int hit_frame = pageFault(item);
	int empty_frame = pageSearch();
	if(hit_frame < 0) { //page fault 발생
		if(empty_frame < 0) { //빈 frame 없는 경우 (optimal 알고리즘에 의해 victim 결정)
			int victim_index = 0;
			int max_index = 0;
			for(int i=0; i<frame_num; i++) {
				int j;
				for(j=index+1; j<refer_str_size; j++) {
					if(frame_arr[i] == refer_str[j]) {
						if(max_index < j) {
							victim_index = i;
							max_index = j;
						}
						break;
					}
				}
				if(j==refer_str_size) {
					victim_index = i;
					max_index = j;
					break;
				}
			}
			frame_arr[victim_index] = item;
		}
		else { //빈 frame이 있는 경우
			frame_arr[empty_frame] = item;
		}
		page_fault_num++;
	}
	else { //page hit
	}
	return hit_frame;
}

int fifo(int item) {
	int hit_frame = pageFault(item);
	int empty_frame = pageSearch();
	if(hit_frame < 0) { //page fault 발생
		if(empty_frame < 0) { //빈 frame 없는 경우 (FIFO 알고리즘에 의해 victim 결정)
			int victim_index;
			int max = 0;
			for(int i=0; i<frame_num; i++) {
				if(max < order[i]) {
					victim_index = i;
					max = order[i];
				}
			}
			frame_arr[victim_index] = item;
			order[victim_index] = -1;
		}
		else { //빈 frame이 있는 경우
			frame_arr[empty_frame] = item;
			order[empty_frame] = -1;
		}
		page_fault_num++;
	}
	else { //page hit
	}

	for(int i=0; i<frame_num; i++) {
		if(frame_arr[i]!=-1) {
			order[i]++;
		}
	}
	return hit_frame;
}

int lru(int item) {
	int hit_frame = pageFault(item);
	int empty_frame = pageSearch();
	if(hit_frame < 0) { //page fault 발생
		if(empty_frame < 0) { //빈 frame 없는 경우 (LRU 알고리즘에 의해 victim 결정)
			int victim_index;
			int max = 0;
			for(int i=0; i<frame_num; i++) {
				if(max < order[i]) {
					victim_index = i;
					max = order[i];
				}
			}
			frame_arr[victim_index] = item;
			order[victim_index] = -1;
		}
		else { //빈 frame이 있는 경우
			frame_arr[empty_frame] = item;
			order[empty_frame] = -1;
		}
		page_fault_num++;
	}
	else { //page hit
		order[hit_frame] = -1;	
	}

	for(int i=0; i<frame_num; i++) {
		if(frame_arr[i]!=-1) {
			order[i]++;
		}
	}
	return hit_frame;
}

int secondChance(int item) {
	int hit_frame = pageFault(item);
	int empty_frame = pageSearch();
	if(hit_frame < 0) { //page fault 발생
		if(empty_frame < 0) { //빈 frame 없는 경우 (LRU 알고리즘에 의해 victim 결정)
			while(1) {
				int victim_index;
				int max = 0;
				for(int i=0; i<frame_num; i++) {
					if(max < order[i]) {
						victim_index = i;
						max = order[i];
					}
				}
				if(refer_bit[victim_index] == 1) {
					order[victim_index] = 0;
					refer_bit[victim_index] = 0;
					for(int i=0; i<frame_num; i++) {
						if(i!=victim_index) {
							order[i]++;
						}
					}
				}
				else {
					frame_arr[victim_index] = item;
					order[victim_index] = -1;
					break;
				}
			}
		}
		else { //빈 frame이 있는 경우
			frame_arr[empty_frame] = item;
			order[empty_frame] = -1;
			refer_bit[empty_frame] = 0;
		}
		page_fault_num++;
	}
	else { //page hit
		refer_bit[hit_frame] = 1; //hit 발생시 reference bit 1
	}

	for(int i=0; i<frame_num; i++) {
		if(frame_arr[i]!=-1) {
			order[i]++;
		}
	}
	return hit_frame;
}
