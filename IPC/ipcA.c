#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main()
{
	char buffer[BUFFER_SIZE];
	int fd[2];

	// 파이프 생성
	if(pipe(fd) == -1) {
		perror("Pipe Failed");
		exit(EXIT_FAILURE);
	}

	// 자식 프로세스 생성
	pid_t pid = fork();

	if(pid < 0) {
		perror("Fork Failed");
		exit(EXIT_FAILURE);
	}

	if(pid > 0) { // 부모 프로세스
		close(fd[0]); // 읽기 모드 파이프 닫기

		while(1) {
			printf("[프로세스 A 화면] 입력대기 : ");
			fgets(buffer, BUFFER_SIZE, stdin);
			buffer[strcspn(buffer, "\n")] = '\0'; // 개행 문자 제거
			
			// 파이프를 통해 메시지 전송 
			write(fd[1], buffer, strlen(buffer) + 1);
			
			// exit 입력시 종료
			if(strcmp(buffer, "exit") == 0) {
				break;
			}
			
			// 수신 대기로 전환
			printf("수신대기...\n");
			sleep(1);
		}

		close(fd[1]); // 쓰기 모드 파이프 닫기
	}

	else { // 자식 프로세스
		close(fd[1]); // 쓰기 모드 파이프 닫기 
		
		while (1) {
		
			printf("[프로세스 B 화면] 수신대기...\n");

			// 파이프를 통해 메시지 수신
			read(fd[0], buffer, BUFFER_SIZE);
			printf("A) %s\n", buffer);

			if (strcmp(buffer, "exit") == 0) {
				break;
			}

			// 입력 대기 상태로 전환
			printf("입력대기 : ");
			fgets(buffer, BUFFER_SIZE, stdin);
			buffer[strcspn(buffer, "\n")] = '\0';
			
			// 파이프를 통해 메시지 전송 
			write(fd[1], buffer, strlen(buffer) + 1);
		}

		close(fd[0]);
	}

	return 0;
}
