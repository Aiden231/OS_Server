#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8888
#define BUFFER_SIZE 1024

void sigint_handler(int sig) {
    char c;

    signal(sig, SIG_IGN); // 시그널을 일시적으로 무시
    printf("정말 종료하시겠습니까? [y/n]: ");
    c = getchar();
    if (c == 'y' || c == 'Y') {
        exit(0);
    } else {
        signal(SIGINT, sigint_handler); // 시그널 핸들러를 다시 설정
    }
    getchar(); // '\n' 문자를 읽어서 버림
}

void *receive_messages(void *socket_desc) {
    int sock = *((int *)socket_desc);
    char buffer[BUFFER_SIZE];
    int nbytes;

    while ((nbytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[nbytes] = '\0';
        printf("%s", buffer);
    }

    return NULL;
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    pthread_t receive_thread;

    signal(SIGINT, sigint_handler);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    pthread_create(&receive_thread, NULL, receive_messages, &sock);

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }

    close(sock);
    return 0;
}

