/*
 * udp_server.c – serwer UDP/IPv4
 * Kompilacja: gcc udp_server.c -o udp_server
 * Użycie:     ./udp_server [port]   (domyślnie 9000)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 9000
#define BUF_SIZE     1024

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : DEFAULT_PORT;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in srv = {
        .sin_family      = AF_INET,
        .sin_port        = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(sockfd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("bind"); exit(1);
    }

    printf("[UDP serwer] nasłuchuje na porcie %d ...\n", port);

    char buf[BUF_SIZE];
    struct sockaddr_in cli;
    socklen_t cli_len = sizeof(cli);

    while (1) {
        ssize_t n = recvfrom(sockfd, buf, BUF_SIZE - 1, 0,
                             (struct sockaddr *)&cli, &cli_len);
        if (n < 0) { perror("recvfrom"); continue; }
        buf[n] = '\0';

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
        printf("[%s:%d] %s\n", ip, ntohs(cli.sin_port), buf);

        /* odsyłamy echo z prefiksem */
        char reply[BUF_SIZE + 8];
        snprintf(reply, sizeof(reply), "ECHO: %s", buf);
        sendto(sockfd, reply, strlen(reply), 0,
               (struct sockaddr *)&cli, cli_len);
    }

    close(sockfd);
    return 0;
}