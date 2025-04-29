#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	pid_t pid; // int 타입 변수 설정(process id) 

	pid = fork(); // fork 실행 

	if (pid < 0) { /* 오류 발생시 */
		fprintf(stderr,"Error Failed");
		return 1;
	}

	else if (pid ==0){ /* child process */
		printf("Hello\n");
		printf("Hello\n");
		printf("Hello\n");
	}

	else {
		wait(NULL); /* parent process */
		for (int i =0; i < 3; i++){
			printf("Hello\n");
		}
	}

	return 0;
}
