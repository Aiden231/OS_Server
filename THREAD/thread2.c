#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#define ARRAY_SIZE 10000000 // 배열 크기 
#define NUM_THREAD 5 // 스레드의 수 

// 배열 두 개 생성 
int array[ARRAY_SIZE] = {0};
int array1[ARRAY_SIZE] = {0};

// 스레드 프로그램 
void *multiply(void *arg) {
	int thread_id = *(int *)arg;
	int start = thread_id * (ARRAY_SIZE / NUM_THREAD);
	int end = (thread_id + 1) * (ARRAY_SIZE / NUM_THREAD);

	for ( int i = start; i < end; i++) {
		array1[i] *= 2; // 모든 배열에 2 곱하기 
	}

	pthread_exit(NULL);
}

// 싱글 스레드 함수 
struct timeval do_single_thread() {
	struct timeval start, end;
	gettimeofday(&start, NULL);

	for(int i = 0; i < ARRAY_SIZE; i++) {
		array[i] *= 2; // 모든 배열에 2 곱하기 
	}

	gettimeofday(&end, NULL);
	struct timeval processing_time;
	timersub(&end, &start, &processing_time);

	printf("array[0] = %ld, array[100] = %ld, array[10000] = %ld\n", array[0], array[100], array[10000]);
	printf("array[100000] = %ld, array[9999999] = %ld\n", array[100000], array[9999999]);

	return processing_time;
}

// 멀티 스레드 함수 
struct timeval do_multi_thread() {
	pthread_t threads[NUM_THREAD];
	int thread_args[NUM_THREAD];
	
	struct timeval start, end;
	gettimeofday(&start, NULL);

	for ( int i = 0; i < NUM_THREAD; i++) {
		thread_args[i] = i;
		pthread_create(&threads[i], NULL, multiply, (void *)&thread_args[i]);
	}

	for ( int i = 0; i < NUM_THREAD; i++) {
		pthread_join(threads[i], NULL);
	}

	gettimeofday(&end, NULL);
	struct timeval processing_time;
	timersub(&end, &start, &processing_time);

	printf("array[0] = %ld, array[100] = %ld, array[10000] = %ld\n",array1[0], array1[100], array1[10000]);
	printf("array[100000] = %ld, array[9999999} = %ld\n", array1[100000], array1[9999999]);

	return processing_time;
}

// 메인 함수
int main(int argc, char* argv[]) {
	// 배열 전부 1로 설정 
	for(int i = 0; i < ARRAY_SIZE; i++) {
		array[i] = 1;
		array1[i] = 1;
	}

	// 결과 출력 
	printf(" --- Single Thread --- \n");
	struct timeval single_thread_processing_time = do_single_thread();
	printf("Single Thread Time : %ld seconds %ld microseconds\n\n",
			single_thread_processing_time.tv_sec, single_thread_processing_time.tv_usec);

	printf(" --- Multi Thread --- \n");
	struct timeval multi_thread_processing_time = do_multi_thread();
	printf("Multi Thread Time : %ld seconds %ld microseconds\n\n",
			multi_thread_processing_time.tv_sec, multi_thread_processing_time.tv_usec);

	return 0;
}
