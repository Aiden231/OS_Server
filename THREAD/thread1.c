#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_THREAD 10
#define ARRAY_SIZE 1000

int a[ARRAY_SIZE] = {0};

typedef struct {
	int* array;
	int start;
	int end;
	int result;
} ThreadData;

void* thread_sum(void* arg) {
	ThreadData* data = (ThreadData*)arg;
	int sum = 0;
	for (int i = data->start; i < data->end; i++) {
		sum += data->array[i];
	}
	data->result = sum;
	return NULL;
}

struct timeval do_single_thread() {
	struct timeval start, end;
	gettimeofday(&start, NULL);

	int sum = 0;
	
	for(int i = 0; i<ARRAY_SIZE; i++) {
		sum +=  a[i];
	}

	printf("Single thread result : %d\n", sum);
	
	gettimeofday(&end, NULL);
	struct timeval processing_time;
	timersub(&end, &start, &processing_time);

	return processing_time;
}

struct timeval do_multi_thread() {
	pthread_t threads[NUM_THREAD];
	ThreadData thread_data[NUM_THREAD];

	int chunk_size = ARRAY_SIZE / NUM_THREAD;
	for (int i = 0; i < NUM_THREAD; i++) {
		thread_data[i].array = a;
		thread_data[i].start = i * chunk_size;
		thread_data[i].end = (i+1) * chunk_size;
		if (i == NUM_THREAD - 1) {
			thread_data[i].end = ARRAY_SIZE;
		}
		pthread_create(&threads[i], NULL, thread_sum, (void*)&thread_data[i]);
	}

	struct timeval start, end;
	gettimeofday(&start, NULL);

	int total_sum = 0;
	for (int i =0; i < NUM_THREAD; i++) {
		pthread_join(threads[i], NULL);
		total_sum += thread_data[i].result;
	}

	gettimeofday(&end, NULL);
	struct timeval processing_time;
	timersub(&end, &start, &processing_time);

	printf("Multi thread result : %d\n", total_sum);

	return processing_time;
}

int main(int argc, char* argv[]){
	for (int i = 0; i < ARRAY_SIZE; i++) {
		a[i] = i + 1;
	}

	printf(" ---Single Thread--- \n");
	struct timeval single_thread_processing_time = do_single_thread();
	printf("Single thread procssing time : %ld seconds %ld microseconds\n\n",
			single_thread_processing_time.tv_sec, single_thread_processing_time.tv_usec);

	printf(" ---Multi Thread--- \n");
	struct timeval multi_thread_processing_time = do_multi_thread();
	printf("Multi thread procssing time : %ld seconds %ld microseconds\n\n",
			multi_thread_processing_time.tv_sec, multi_thread_processing_time.tv_usec);

	return 0;
}
