#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM 10

int g_val = 10;

long long single_thread_result = 1;
long long multi_thread_result = 1;
long long multi_thread_results[NUM] = {0};
pthread_mutex_t mutex =  PTHREAD_MUTEX_INITIALIZER;

void *calculate(void *thread_id) {
	long tid = (long)thread_id;
	for(int i = 1; i<=10; ++i) {
		multi_thread_result = g_val * i;
	}

	pthread_mutex_lock(&mutex);
	multi_thread_results[tid] = multi_thread_result;
	pthread_mutex_unlock(&mutex);
	
	pthread_exit(NULL);
}

struct timeval do_single_thread() {
	struct timeval start, end;

	gettimeofday(&start, NULL);

	printf("\n---Single Thread---\n");

	for (int i = 1; i<=10; ++i) {
		single_thread_result = g_val * i;
		printf("g_val = %ld\n",single_thread_result);
	}

	gettimeofday(&end, NULL);

	struct timeval processing_time;
	timersub(&end, &start, &processing_time);
	return processing_time;
}

struct timeval do_multi_thread() {
	pthread_t threads[NUM];
	struct timeval start, end;

	gettimeofday(&start, NULL);
	
	printf("\n---Multi Thread---\n");

	for (long t = 0; t < NUM; ++t) {
		pthread_create(&threads[t], NULL, calculate, (void*)t);
	}

	for (long t = 0; t< NUM; ++t) {
		pthread_join(threads[t], NULL);
	}

	gettimeofday(&end, NULL);

	struct timeval processing_time;
	timersub(&end, &start, &processing_time);
	return processing_time;
}

int main(int argc, char* argv[]) {
	struct timeval single_thread_processing_time = do_single_thread();
	printf("Single thread processing time : %ld seconds %ld microseconds\n",
			single_thread_processing_time.tv_sec, single_thread_processing_time.tv_usec);

	struct timeval multi_thread_processing_time = do_multi_thread();
	printf("Multi thread processing time : %ld seconds %ld mircroseconds\n",
			multi_thread_processing_time.tv_sec, multi_thread_processing_time.tv_usec);

	for(int i = 0; i < NUM; i++) {
		printf("g_val = %ld\n", multi_thread_results[i]);
	}

	return 0;
}
