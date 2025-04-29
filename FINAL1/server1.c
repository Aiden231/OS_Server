#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8888
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 10

int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pool_cond = PTHREAD_COND_INITIALIZER;

volatile sig_atomic_t stop;
int task_queue[MAX_CLIENTS];
int task_count = 0;

void catch_signal(int sig) {
    stop = 1;
    pthread_cond_broadcast(&pool_cond);
}

void broadcast_message(char *message, int exclude_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i] != 0 && client_sockets[i] != exclude_socket) {
            if (send(client_sockets[i], message, strlen(message), 0) < 0) {
                perror("Send failed");
                close(client_sockets[i]);
                client_sockets[i] = 0;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_socket;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 32];
    int nbytes;

    while (1) {
        pthread_mutex_lock(&pool_mutex);
        while (task_count == 0 && !stop) {
            pthread_cond_wait(&pool_cond, &pool_mutex);
        }
        if (stop) {
            pthread_mutex_unlock(&pool_mutex);
            break;
        }
        client_socket = task_queue[0];
        for (int i = 0; i < task_count - 1; ++i) {
            task_queue[i] = task_queue[i + 1];
        }
        task_count--;
        pthread_mutex_unlock(&pool_mutex);

        while ((nbytes = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
            buffer[nbytes] = '\0';
            snprintf(message, sizeof(message), "Client %d: %s", client_socket, buffer);
            printf("%s", message);
            broadcast_message(message, client_socket);
        }

        close(client_socket);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (client_sockets[i] == client_socket) {
                client_sockets[i] = 0;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        snprintf(message, sizeof(message), "Client %d has disconnected.\n", client_socket);
        printf("%s", message);
        broadcast_message(message, -1);
    }

    return NULL;
}

void *thread_function(void *arg) {
    while (1) {
        pthread_mutex_lock(&pool_mutex);
        while (task_count == 0 && !stop) {
            pthread_cond_wait(&pool_cond, &pool_mutex);
        }
        if (stop) {
            pthread_mutex_unlock(&pool_mutex);
            break;
        }
        int client_socket = task_queue[--task_count];
        pthread_mutex_unlock(&pool_mutex);

        handle_client((void *)&client_socket);
    }

    return NULL;
}

int main() {
    int server_socket, client_socket, addr_len;
    struct sockaddr_in server_addr, client_addr;

    signal(SIGINT, catch_signal);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_sockets[i] = 0;
    }

    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 0) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", PORT);

    while (!stop) {
        addr_len = sizeof(client_addr);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len)) < 0) {
            if (stop) break;
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; ++i) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (i == MAX_CLIENTS) {
            char *message = "Server is full\n";
            send(client_socket, message, strlen(message), 0);
            close(client_socket);
        } else {
            pthread_mutex_lock(&pool_mutex);
            task_queue[task_count++] = client_socket;
            pthread_cond_signal(&pool_cond);
            pthread_mutex_unlock(&pool_mutex);

            char message[BUFFER_SIZE];
            snprintf(message, sizeof(message), "Client %d has connected.\n", client_socket);
            printf("%s", message);
            broadcast_message(message, -1);
        }
    }

    printf("Shutting down server...\n");
    close(server_socket);

    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        pthread_join(thread_pool[i], NULL);
    }

    return 0;
}

