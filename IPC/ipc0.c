#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1

int main()
{
	char write_msg[BUFFER_SIZE] = "1234";
	char read_msg[BUFFER_SIZE];

	int fd[2];
	pid_t pid;

	// 파이프 생성
	if( pipe(fd) == -1){
		fprintf(stderr, "Pipe Failed"); // 오류 발생시 
		return 1;
	}

	pid = fork();

	if(pid < 0) { // Fork 오류 발생시 
		fprintf(stderr, "Fork Failed");
		return 1;
	}

	if(pid > 0) { // 부모 프로세스
		close(fd[READ_END]);
		write(fd[WRITE_END], write_msg, BUFFER_SIZE);
		close(fd[WRITE_END]);
	}

	else { // 자식 프로세스 
		close(fd[WRITE_END]);
		read(fd[READ_END], read_msg, BUFFER_SIZE);
		printf("\nSend %s to Child...\n", write_msg);
		printf("Received %s from Parent\n", read_msg);
		close(fd[READ_END]);
	}

	return 0;
}
