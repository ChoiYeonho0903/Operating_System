#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

int vehicle_num = 10;
int vehicle_arr[10] = {4, 4, 3, 4, 1, 2, 2, 3, 3, 3};
int vehicle_progress[10]; //malloc이후 초기화 해줘야한다.
int visited[10];
pthread_t tid[4];
int passed_num[4];
int tick;
int limit;
int status;
int passed_vehicle;
int current_vehicle;
int current_index;

pthread_mutex_t mutex_main = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_main = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex[4] = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
};
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
int search_end();

int main(int argc, char *argv[]) {
//	printf("차량수를 입력해주세요: ");
//	scanf("%d", &vehicle_num);

/*	for(int i=0; i<vehicle_num; i++) {
		vehicle_arr[i] = rand()%4 + 1;
		printf("%d\n", vehicle_arr[i]);
	}

*/
	
	pthread_mutex_lock(&mutex_main);
	//thread 생성
	if(pthread_create(&tid[0], NULL, p1_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex_main);
	if(pthread_create(&tid[1], NULL, p2_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex_main);

	if(pthread_create(&tid[2], NULL, p3_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex_main);

	if(pthread_create(&tid[3], NULL, p4_thread, NULL) != 0) {
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	pthread_cond_wait(&cond_main, &mutex_main);
	pthread_mutex_unlock(&mutex_main);

	while(1) {
		pthread_mutex_lock(&mutex_main);
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
		pthread_cond_wait(&cond_main, &mutex_main);

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
				pthread_cond_wait(&cond_main, &mutex_main);
			}
			else {
				current_vehicle = 0;
				current_index = 0;
			}
		}
		
		print();
		
		if(search_end()==1 || tick>30) {
			break;	
		}

		pthread_mutex_unlock(&mutex_main);
	}

	for(int i=0; i<4; i++) {
		pthread_mutex_destroy(&mutex[i]);
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
		pthread_mutex_lock(&mutex[0]);
		if(tick==0) {
			pthread_cond_signal(&cond_main);
		}
		pthread_cond_wait(&cond[0], &mutex[0]);

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 1;
			visited[current_index]=1;
			passed_num[0]++;
		}
		
		if(search_end()==1) {
			pthread_cond_signal(&cond_main);
			break;
		}
		
		pthread_cond_signal(&cond_main);
		pthread_mutex_unlock(&mutex[0]);

	}
	return NULL;
}

void *p2_thread(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex[1]);
		if(tick==0) {
			pthread_cond_signal(&cond_main);
		}
		pthread_cond_wait(&cond[1], &mutex[1]);

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 2;
			visited[current_index]=1;
			passed_num[1]++;
		}

		if(search_end()==1) {
			pthread_cond_signal(&cond_main);
			break;
		}

		pthread_cond_signal(&cond_main);
		pthread_mutex_unlock(&mutex[1]);
	}
	return NULL;
}

void *p3_thread(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex[2]);
		if(tick==0) {
			pthread_cond_signal(&cond_main);
		}
		pthread_cond_wait(&cond[2], &mutex[2]);
		

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 3;
			visited[current_index]=1;
			passed_num[2]++;
		}

		if(search_end()==1) {
			pthread_cond_signal(&cond_main);
			break;
		}

		pthread_cond_signal(&cond_main);
		pthread_mutex_unlock(&mutex[2]);
	}
	return NULL;
}

void *p4_thread(void *arg) {
	while(1) {
		pthread_mutex_lock(&mutex[3]);
		if(tick==0) {
			pthread_cond_signal(&cond_main);
		}
		pthread_cond_wait(&cond[3], &mutex[3]);

		vehicle_progress[current_index]++;
		if(vehicle_progress[current_index]==2) {
			passed_vehicle = 4;
			visited[current_index]=1;
			passed_num[3]++;
		}

		if(search_end()==1) {
			pthread_cond_signal(&cond_main);
			break;
		}

		pthread_cond_signal(&cond_main);
		pthread_mutex_unlock(&mutex[3]);
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

int search_end() {
	int total = 0;
	for(int i=0; i<4; i++) {
		total += passed_num[i];
	}
	if(total == vehicle_num) {
		return 1;
	}
	else {
		return 0;
	}
}

