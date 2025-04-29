#include <pthread.h>
#include <stdio.h>

// 읽기와 쓰기를 위한 LOCK을 정의 
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

int inventory = 100; // 재고 

// 재고를 읽는 함수
void *read_inventory(void* arg) {
	pthread_rwlock_rdlock(&rwlock); // 읽기 LOCK 획득
	printf("Current inventory : %d\n", inventory);
	pthread_rwlock_unlock(&rwlock); // 읽기 LOCK 해제 

	return NULL;
}

// 재고를 쓰는 함수
void *write_inventory(void* arg) {
	pthread_rwlock_wrlock(&rwlock); // 쓰기 LOCK 획득
	inventory -= 10; // 재고를 10 감소 
	printf("Inventory update, Current inventory %d\n", inventory);
	pthread_rwlock_unlock(&rwlock); // 쓰기 LOck 해제 
	
	return NULL;
}

int main() {
	pthread_t reader[5], writer;
	int i;

	// 5개의 읽기 스레드 생성 
	for ( i = 0; i < 5; i++) {
		pthread_create(&reader[i], NULL, read_inventory, NULL);
	}

	// 쓰기 스레드 생성
	pthread_create(&writer, NULL, write_inventory, NULL);
	
	// 스레드가 실행을 마칠 떄까지 대기 
	for ( i = 0; i < 5; ++i) {
		pthread_join(reader[i], NULL);
	}
	pthread_join(writer, NULL);

	return 0;
}
