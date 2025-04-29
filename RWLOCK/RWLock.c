#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_OF_READER 5
#define NUM_OF_WRITER 2

pthread_rwlock_t rwlock;
int inventory = 100;

void* reader(void* arg) {
	int id = *(int*)arg;
	free(arg);
	while(1) {
		pthread_rwlock_rdlock(&rwlock);
		printf("Reader %d : Inverntory %d\n", id, inventory);
		pthread_rwlock_unlock(&rwlock);
		sleep(rand() % 3);
	}
	return NULL;
}

void* writer(void* arg) {
	int id = *(int*)arg;
	free(arg);
	while(1) {
		pthread_rwlock_wrlock(&rwlock);
		inventory -= 10;
		printf("Writer %d : Inventory %d\n", id, inventory);
		pthread_rwlock_unlock(&rwlock);
		sleep(rand() % 5);
	}
	return NULL;
}

int main() {
	pthread_t reader[NUM_OF_READER], writer[NUM_OF_WRITER];

	pthread_rwlock_init(&rwlock, NULL);

	void* (*_start_routine)(void*) = NULL;

	_start_routine = (void* (*)(void*))&reader;
	for (int i = 0; i<NUM_OF_READER; i++) {
		int *id = malloc(sizeof(int));
		if ( id == NULL ) {
			perror("Failed to allocate memory");
			exit(EXIT_FAILURE);
		}
		*id = i + 1;

		pthread_create(&reader[i], NULL, _start_routine, id);
	}


	_start_routine = (void* (*)(void*))&writer;
	for ( int i = 0; i<NUM_OF_WRITER; i++) {
		int *id = malloc(sizeof(int));
		if ( id == NULL) {
			perror("Failed to allocate memory");
			exit(EXIT_FAILURE);
		}
		*id = i + 1;

		pthread_create(&writer[i], NULL, _start_routine, id);
	}

	for ( int i = 0; i < NUM_OF_READER; i++) {
		pthread_join(reader[i], NULL);
	}

	for ( int i = 0; i < NUM_OF_WRITER; i++) {
		pthread_join(writer[i], NULL);
	}

	pthread_rwlock_destroy(&rwlock);
	return 0;
}
