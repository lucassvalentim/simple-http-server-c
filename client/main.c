#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 4096

// parse da url http://<host>[:porta]/<arquivo>
int parse_url(const char *url, char *host, int *port, char *path){
    if(strncmp(url, "http://", 7) != 0){
        fprintf(stderr, "URL invalida (deve comecar com http://)\n");
        return -1;
    }

    const char *host_start = url + 7;
    const char *path_start = strchr(host_start, '/');
    const char *port_start = strchr(host_start, ':');

    if(path_start == NULL){
        strcpy(path, "/");
    }else{
        strcpy(path, path_start);
    }

    if(port_start && port_start < path_start){
        *port = atoi(port_start + 1);
        strncpy(host, host_start, port_start - host_start);
        host[port_start - host_start] = '\0';
    }else{
        *port = 80;

        if(path_start){
            strncpy(host, host_start, path_start - host_start);
        }else{
            strcpy(host, host_start);
        }

        host[(path_start ? path_start - host_start : strlen(host_start))] = '\0';
    }

    return 0;
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Uso: %s http://<host>[:porta]/<arquivo>\n", argv[0]);
        return 1;
    }

    char host[256], path[512];
    int port;

    if(parse_url(argv[1], host, &port, path) != 0){
        return 1;
    }

    printf("Host: %s\n", host);
    printf("Port: %d\n", port);
    printf("Path: %s\n", path);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("Erro ao criar o socket\n");
        return 1;
    }

    struct hostent *server = gethostbyname(host);
    if(server == NULL){
        fprintf(stderr, "Host nao encontrado\n");
        close(sockfd);
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memset(&server_addr.sin_addr.s_addr, atoi(server->h_addr_list[0]), server->h_length);

    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof server_addr) < 0){
        perror("Erro ao conectar");
        close(sockfd);
        return 1;
    }

    char request[1024];
    snprintf(request, sizeof request, 
            "GET %s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Connection: close \r\n\r\n",
            path, host
    );

    send(sockfd, request, strlen(request), 0);

    const char * filename = strrchr(path, '/');
    if(filename){
        filename = filename + 1;
        if(strlen(filename) == 0){
            filename = "downloaded_file.html";
        }else{
            const char *ext = strrchr(filename, '.');
            if(ext == NULL){
                filename = "directory_listing.html";
            }
        }
    }else{
        filename = "downloaded_file.html";
    }

    FILE *fp = fopen(filename, "wb");
    if(!fp){
        perror("Erro ao criar arquivo local");
        close(sockfd);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int bytes, header_done = 0;
    char *body_start;

    while((bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0){
        buffer[bytes] = '\0';

        if(!header_done) {
            body_start = strstr(buffer, "\r\n\r\n");

            if(body_start){
                header_done = 1;
                *body_start = '\0';

                printf("Header recebido:\n %s\n", buffer);

                if(strstr(buffer, "404")){
                    printf("Erro 404: arquivo nao encontrado.\n");
                    fclose(fp);
                    close(sockfd);
                    remove(filename);
                    return 1;
                }

                body_start += 4;
                fwrite(body_start, 1, bytes - (body_start - buffer), fp);
            }
            
        } else {
            fwrite(buffer, 1, bytes, fp);
        }
    }

    printf("Arquivo salvo como '%s'\n", filename);

    fclose(fp);
    close(sockfd);

    return 0;
}