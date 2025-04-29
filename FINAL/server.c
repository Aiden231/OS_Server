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

int client_sockets[MAX_CLIENTS];
int client_ids[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_pool[MAX_CLIENTS];
volatile sig_atomic_t stop;

void catch_signal(int sig) {
    stop = 1;
}

void broadcast_message(char *message, int exclude_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i] != 0 && client_sockets[i] != exclude_socket) {
            if (send(client_sockets[i], message, strlen(message), 0) < 0) {
                perror("Send failed");
                close(client_sockets[i]);
                client_sockets[i] = 0;
                client_ids[i] = 0;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    int client_id;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 32];
    int nbytes;

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i] == client_socket) {
            client_id = client_ids[i];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    while ((nbytes = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[nbytes] = '\0';
        snprintf(message, sizeof(message), "Client %d: %s", client_id, buffer);
        printf("%s", message);
        broadcast_message(message, client_socket);
    }

    close(client_socket);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = 0;
            client_ids[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    snprintf(message, sizeof(message), "Client %d has disconnected.\n", client_id);
    printf("%s", message);
    broadcast_message(message, -1);

    return NULL;
}

int main() {
    int server_socket, client_socket, addr_len;
    struct sockaddr_in server_addr, client_addr;

    signal(SIGINT, catch_signal);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_sockets[i] = 0;
        client_ids[i] = 0;
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

    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started...\n");
    printf("Waiting for client..\n");

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
                client_ids[i] = i + 1;  // Assign unique ID starting from 1
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (i == MAX_CLIENTS) {
            char *message = "Server is full\n";
            send(client_socket, message, strlen(message), 0);
            close(client_socket);
        } else {
            pthread_create(&thread_pool[i], NULL, handle_client, &client_socket);
            char message[BUFFER_SIZE];
            snprintf(message, sizeof(message), "Client %d has connected.\n", i + 1);
            printf("%s", message);
            broadcast_message(message, -1);
        }
    }

    printf("Shutting down server...\n");
    close(server_socket);
    return 0;
}

