#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define sleep(x) Sleep(1000 * (x))
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define PORT 8080

typedef struct {
    SOCKET client_socket;
    int client_id;
} client_data;

void handle_client(SOCKET client_socket, int client_id) {
    char *message =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello Client!";

    printf("[Server] Handling client %d...\n", client_id);

    printf("[Server] Processing request for 5 seconds...\n");
    sleep(5);

    send(client_socket, message, (int)strlen(message), 0);
    printf("[Server] Response sent to client %d. Closing connection.\n", client_id);

#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

void* client_thread(void* arg) {
    client_data* data = (client_data*)arg;

    handle_client(data->client_socket, data->client_id);

    free(data);
    return NULL;
}
 
int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    int client_count = 0;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    printf("Server listening on port %d...\n", PORT);
    printf("NOTE: This server is MULTITHREADED. It can handle multiple clients at the same time!\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);

        if (client_socket != INVALID_SOCKET) {
            client_count++;

            client_data* data = malloc(sizeof(client_data));
            data->client_socket = client_socket;
            data->client_id = client_count;

            pthread_t tid;
            pthread_create(&tid, NULL, client_thread, data);
            pthread_detach(tid);
        }
    }

#ifdef _WIN32
    closesocket(server_socket);
    WSACleanup();
#else
    close(server_socket);
#endif

    return 0;
}