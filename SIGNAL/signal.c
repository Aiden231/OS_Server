#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

// SIGINT 시그널 핸들러 
void sigint_handler(int signum) {
	char answer[5];
	printf("\n종료를 원하면 yes를 입력해 주세요 : "); // 종료할 것인지 물어봄 
	scanf("%s", answer); // 답변 받기 

	// yes 입력 시 종료 
	if(strcmp(answer, "yes") == 0) {
		printf("EXIT\n");
		exit(0);
	}
	// 아니면 계속 시행 
	else {
		signal(SIGINT, sigint_handler);
	}
}

// SIGSTP 시그널 핸들러  
void sigstp_handler(int signum) {
	printf("\n 프로그램이 SIGSTP 시그널을 받았습니다.일시정지합니다.\n");
}

// SIGAUIT 시그널 핸들러
void sigquit_handler(int signum) {
	printf("\n 프로그램이 SIGQUIT 시그널을 받았습니다.종료합니다.\n");
	exit(0);
}

int main() {
	//시그널 핸들러 설정
	signal(SIGINT, sigint_handler);
	signal(SIGTSTP, sigstp_handler);
	signal(SIGQUIT, sigquit_handler);

	// 시그널 핸들러 사용법 
	printf("Ctrl + c = SIGINT\n");
	printf("Ctrl + z = SIGSTP\n");
	printf("Ctrl + | = SIGQUIT\n");

	// 무한 루프 실행
	while (1) {
		// 그저 대기 하는 중
	}

	return 0;
}
