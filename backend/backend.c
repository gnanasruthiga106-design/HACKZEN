/*
 * PixelClear Studio C web server (Windows / MinGW GCC)
 * - Serves ../index.html at http://127.0.0.1:8000/
 * - Provides GET /api/status for the dashboard's backend connection badge
 * - Provides POST /api/ping for a simple backend test
 *
 * This server does not perform AI background removal. The existing dashboard
 * performs background removal in the browser using its configured JS model.
 */
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8000
#define REQUEST_CAP 8192

static void send_response(SOCKET client, int status, const char *status_text,
                          const char *content_type, const char *body) {
    char header[1024];
    size_t body_len = strlen(body);
    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n\r\n",
        status, status_text, content_type, body_len);
    send(client, header, hlen, 0);
    if (body_len) send(client, body, (int)body_len, 0);
}

static void serve_dashboard(SOCKET client) {
    FILE *file = fopen("index.html", "rb");
    if (!file) {
        send_response(client, 500, "Internal Server Error", "text/plain; charset=utf-8",
                      "Could not open index.html. Run backend.exe from the project root folder.");
        return;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        send_response(client, 500, "Internal Server Error", "text/plain; charset=utf-8", "Could not read index.html.");
        return;
    }
    long file_size = ftell(file);
    if (file_size < 0 || file_size > 10 * 1024 * 1024) {
        fclose(file);
        send_response(client, 500, "Internal Server Error", "text/plain; charset=utf-8", "Dashboard file size is invalid.");
        return;
    }
    rewind(file);
    char *content = (char *)malloc((size_t)file_size + 1);
    if (!content) {
        fclose(file);
        send_response(client, 500, "Internal Server Error", "text/plain; charset=utf-8", "Out of memory.");
        return;
    }
    size_t read_count = fread(content, 1, (size_t)file_size, file);
    fclose(file);
    if (read_count != (size_t)file_size) {
        free(content);
        send_response(client, 500, "Internal Server Error", "text/plain; charset=utf-8", "Could not read dashboard.");
        return;
    }
    content[read_count] = '\0';
    char header[1024];
    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\nCache-Control: no-store\r\n"
        "Connection: close\r\n\r\n", read_count);
    send(client, header, hlen, 0);
    size_t sent = 0;
    while (sent < read_count) {
        int chunk = (int)((read_count - sent) > 1024 * 1024 ? 1024 * 1024 : read_count - sent);
        int n = send(client, content + sent, chunk, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
    free(content);
}

static void handle_client(SOCKET client) {
    char request[REQUEST_CAP];
    int used = 0;
    memset(request, 0, sizeof(request));
    while (used < REQUEST_CAP - 1) {
        int n = recv(client, request + used, REQUEST_CAP - 1 - used, 0);
        if (n <= 0) break;
        used += n;
        request[used] = '\0';
        if (strstr(request, "\r\n\r\n")) break;
    }
    if (used == 0) return;

    if (strncmp(request, "OPTIONS ", 8) == 0) {
        const char *response =
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\nConnection: close\r\n\r\n";
        send(client, response, (int)strlen(response), 0);
    } else if (strncmp(request, "GET /api/status ", 16) == 0) {
        send_response(client, 200, "OK", "application/json; charset=utf-8",
                      "{\"status\":\"ok\",\"service\":\"PixelClear Studio C backend\"}");
    } else if (strncmp(request, "POST /api/ping ", 15) == 0) {
        send_response(client, 200, "OK", "application/json; charset=utf-8",
                      "{\"status\":\"ok\",\"message\":\"C backend received the request\"}");
    } else if (strncmp(request, "GET / ", 6) == 0 || strncmp(request, "GET /index.html ", 16) == 0) {
        serve_dashboard(client);
    } else {
        send_response(client, 404, "Not Found", "application/json; charset=utf-8",
                      "{\"error\":\"Route not found\"}");
    }
}

int main(void) {
    WSADATA wsa;
    SOCKET server;
    struct sockaddr_in address;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Winsock initialization failed.\n");
        return 1;
    }
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        WSACleanup();
        return 1;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        fprintf(stderr, "Port %d could not be opened. Close another app using this port and try again.\n", PORT);
        closesocket(server);
        WSACleanup();
        return 1;
    }
    if (listen(server, 8) == SOCKET_ERROR) {
        fprintf(stderr, "Server listen failed.\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    printf("PixelClear Studio C backend is running.\n");
    printf("Open dashboard: http://127.0.0.1:%d\n", PORT);
    printf("Status API:     http://127.0.0.1:%d/api/status\n", PORT);
    printf("Press Ctrl+C to stop.\n\n");
    for (;;) {
        SOCKET client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) continue;
        handle_client(client);
        shutdown(client, SD_SEND);
        closesocket(client);
    }
    closesocket(server);
    WSACleanup();
    return 0;
}
