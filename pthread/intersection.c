#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

int vehicle_num;          //전체 차량의 수
int *vehicle_arr;         //랜덤으로 생성한 차량의 배열
int *vehicle_progress;    //차량의 진행과정 (1초, 2초 진행여부)
int *visited;             //차량의 도착여부
int passed_num[4];        //각 출발점마다 총 진행된 차량의 수
int tick;                 //시간
int limit;              
int status;               
int passed_vehicle;       //현재시간에 도착한 차량
int current_vehicle;      //현재 진행되고 있는 차량
int current_index;        //vehicle_arr에서 현재 진행되고 있는 차량의 index
int total;                //현재 도착한 전체 차량

pthread_t tid[4];         //4개의 thread ID 주소공간

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;           //mutex lock 생성과 동시에 정적초기화

pthread_cond_t cond_main = PTHREAD_COND_INITIALIZER;         //main thread, 4개의 thread의 조건변수
pthread_cond_t cond[4] = {
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
};

void *p1_thread(void *arg);
void *p2_thread(void *arg);
void *p3_thread(void *arg);
void *p4_thread(void *arg);
void print();

int main(int argc, char *argv[]) {
	printf("차량수를 입력해주세요: ");
	scanf("%d", &vehicle_num);
	
	vehicle_arr=(int*)malloc(sizeof(int)*vehicle_num);
	vehicle_progress=(int*)malloc(sizeof(int)*vehicle_num);
	visited=(int*)malloc(sizeof(int)*vehicle_num);
	srand(time(NULL));

	printf("Total number of vehicles : %d\n", vehicle_num);
	printf("Start point : ");
	for(int i=0; i<vehicle_num; i++) {
		vehicle_arr[i] = rand()%4 + 1;
		printf("%d ", vehicle_arr[i]);
	}
	printf("\n");

	pthread_mutex_lock(&mutex);
	//thread 생성
	if(pthread_create(&tid[0], NULL, p1_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex);
	if(pthread_create(&tid[1], NULL, p2_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex);

	if(pthread_create(&tid[2], NULL, p3_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex);

	if(pthread_create(&tid[3], NULL, p4_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex);
	pthread_mutex_unlock(&mutex);

	while(1) {
		pthread_mutex_lock(&mutex);
		tick++;
		limit = tick < vehicle_num ? tick : vehicle_num;
		passed_vehicle = 0;
		if(current_vehicle==0) {
			for(int i=0; i<limit; i++) {
				if(visited[i]==0) {
					current_vehicle = vehicle_arr[i];
					current_index = i;
					break;
				}
			}
		}

		pthread_cond_signal(&cond[current_vehicle-1]);
		pthread_cond_wait(&cond_main, &mutex);
		
		if(vehicle_progress[current_index]==2) {
			int tf = 0;
			for(int i=0; i<limit; i++) {
				if(visited[i]==0 && abs(current_vehicle - vehicle_arr[i])==2) {
					current_vehicle = vehicle_arr[i];
					current_index = i;
					tf = 1;
					break;
				}
			}

			if(tf==1) {
				pthread_cond_signal(&cond[current_vehicle-1]);
				pthread_cond_wait(&cond_main, &mutex);
			}
			else {
				current_vehicle = 0;
				current_index = 0;
			}
		}
		
		print();
		
		if(total==vehicle_num) {
			tick++;
			passed_vehicle = 0;
			print();
			break;	
		}

		pthread_mutex_unlock(&mutex);
	}

	//4개의 thread 종료
	for(int i=0; i<4; i++) {
		pthread_cond_signal(&cond[i]);
	}

	//mutex와 조건변수 해제
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond_main);
	for(int i=0; i<4; i++) {
		pthread_cond_destroy(&cond[i]);
	}

	printf("Number of vehicles passed from each start point\n");
	for(int i=0; i<4; i++) {
		printf("P%d : %d times\n", i+1, passed_num[i]);
	}
	printf("Total time : %d\n", tick);
	exit(0);
}

void *p1_thread(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond_main);
		pthread_cond_wait(&cond[0], &mutex);

		if(total==vehicle_num) {
			pthread_mutex_unlock(&mutex);
			break;
		}

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 1;
			visited[current_index]=1;
			total++;
			passed_num[0]++;
		}
		
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *p2_thread(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond_main);
		pthread_cond_wait(&cond[1], &mutex);

		if(total==vehicle_num) {
			pthread_mutex_unlock(&mutex);
			break;
		}

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 2;
			visited[current_index]=1;
			total++;
			passed_num[1]++;
		}

		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *p3_thread(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond_main);
		pthread_cond_wait(&cond[2], &mutex);

		if(total==vehicle_num) {
			pthread_mutex_unlock(&mutex);
			break;
		}

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 3;
			visited[current_index]=1;
			total++;
			passed_num[2]++;
		}


		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *p4_thread(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond_main);
		pthread_cond_wait(&cond[3], &mutex);

		if(total==vehicle_num) {
			pthread_mutex_unlock(&mutex);
			break;
		}

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 4;
			visited[current_index]=1;
			total++;
			passed_num[3]++;
		}

		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void print() {
	printf("tick : %d\n", tick);
	printf("==============================\n");
	printf("Passed Vehicle\n");
	if(passed_vehicle==0) {
		printf("Car\n");
	}
	else {
		printf("Car %d\n", passed_vehicle);
		passed_vehicle = 0;
	}
	printf("Waiting Vehicle\n");
	printf("Car ");
	for(int i=0; i<limit; i++) {
		if(vehicle_progress[i]==0) {
			printf("%d ", vehicle_arr[i]);
		}
	}
	printf("\n");
	printf("==============================\n");
}
