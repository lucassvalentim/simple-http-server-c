#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 3000
#define BUFFER_SIZE 8192

void send_response(int client_fd, const char *status, const char *content_type, const char *body) {
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             status, content_type, strlen(body));

    send(client_fd, header, strlen(header), 0);
    send(client_fd, body, strlen(body), 0);
}

void send_file(int client_fd, const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        send_response(client_fd, "404 Not Found", "text/html", "<h1>404 - Arquivo nao encontrado</h1>");
        return;
    }

    char header[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    // Envia cabeçalho HTTP
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Connection: close\r\n\r\n");
    send(client_fd, header, strlen(header), 0);

    // Envia o conteúdo do arquivo
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_fd, buffer, bytes_read, 0);
    }

    fclose(file);
}

void send_directory_listing(int client_fd, const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        send_response(client_fd, "500 Internal Server Error", "text/html", "<h1>Erro ao abrir diretorio</h1>");
        return;
    }

    char body[BUFFER_SIZE * 2];
    strcpy(body, "<html><body><h1>Listagem de arquivos</h1><ul>");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignora diretórios ocultos como . e ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        strcat(body, "<li><a href=\"");
        strcat(body, entry->d_name);
        strcat(body, "\">");
        strcat(body, entry->d_name);
        strcat(body, "</a></li>");
    }

    closedir(dir);
    strcat(body, "</ul></body></html>");

    send_response(client_fd, "200 OK", "text/html", body);
}

void handle_client(int client_fd, const char *base_dir) {
    char buffer[BUFFER_SIZE];
    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        close(client_fd);
        return;
    }

    buffer[bytes] = '\0';
    printf("\nRequisição recebida:\n%s\n", buffer);

    // Lê a linha do método HTTP
    char method[8], path[1024];
    sscanf(buffer, "%s %s", method, path);

    if (strcmp(method, "GET") != 0) {
        send_response(client_fd, "405 Method Not Allowed", "text/html", "<h1>Metodo nao permitido</h1>");
        close(client_fd);
        return;
    }

    // Monta o caminho completo do arquivo solicitado
    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s%s", base_dir, path);

    printf("Full path: %s\n", full_path);
    struct stat path_stat;
    if (stat(full_path, &path_stat) != 0) {
        send_response(client_fd, "404 Not Found", "text/html", "<h1>Arquivo nao encontrado</h1>");
    } else if (S_ISDIR(path_stat.st_mode)) {
        // Se for diretório, tenta servir index.html
        char index_path[2060];
        snprintf(index_path, sizeof(index_path), "%s/index.html", full_path);
        if (access(index_path, F_OK) == 0) {
            send_file(client_fd, index_path);
        } else {
            send_directory_listing(client_fd, full_path);
        }
    } else {
        send_file(client_fd, full_path);
    }

    close(client_fd);
}

int main(int argc, char *argv[]) {

    if(argc != 2){
        printf("Uso: %s <diretorio a ser servido>\n", argv[0]);
        return 1;
    }

    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_size = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erro ao criar socket");
        return 1;
    }

    // Permite reuso da porta
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("Erro no bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Erro ao ouvir");
        close(server_fd);
        return 1;
    }

    printf("Servidor HTTP ouvindo na porta %d...\n", PORT);
    printf("Servindo diretório: %s\n", argv[1]);

    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_size);
        if (client_fd < 0) {
            perror("Erro no accept");
            continue;
        }
        handle_client(client_fd, argv[1]);
    }

    close(server_fd);
    return 0;
}
