/*
 * tcp_ipv6_server.c – serwer TCP/IPv6
 * Kompilacja: gcc tcp_ipv6_server.c -o tcp_ipv6_server
 * Użycie:     ./tcp_ipv6_server [port]   (domyślnie 9001)
 *
 * Uwaga: IPv6 z opcją IPV6_V6ONLY=0 akceptuje też połączenia IPv4
 *        w postaci ::ffff:127.0.0.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 9001
#define BUF_SIZE     1024
#define BACKLOG      5

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : DEFAULT_PORT;

    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    /* SO_REUSEADDR – szybki restart serwera */
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* IPV6_V6ONLY = 0 → akceptuj też IPv4-mapped */
    int v6only = 0;
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

    struct sockaddr_in6 srv = {
        .sin6_family = AF_INET6,
        .sin6_port   = htons(port),
        .sin6_addr   = in6addr_any
    };

    if (bind(sockfd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(sockfd, BACKLOG) < 0) { perror("listen"); exit(1); }

    printf("[TCP/IPv6 serwer] nasłuchuje na porcie %d ...\n", port);

    while (1) {
        struct sockaddr_in6 cli;
        socklen_t cli_len = sizeof(cli);
        int connfd = accept(sockfd, (struct sockaddr *)&cli, &cli_len);
        if (connfd < 0) { perror("accept"); continue; }

        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &cli.sin6_addr, ip, sizeof(ip));
        printf("[połączenie] %s:%d\n", ip, ntohs(cli.sin6_port));

        /* prosta obsługa – echo w pętli aż do zamknięcia połączenia */
        char buf[BUF_SIZE];
        ssize_t n;
        while ((n = recv(connfd, buf, BUF_SIZE - 1, 0)) > 0) {
            buf[n] = '\0';
            /* usuń ewentualny newline do wydruku */
            char tmp[BUF_SIZE];
            strncpy(tmp, buf, sizeof(tmp));
            tmp[strcspn(tmp, "\r\n")] = '\0';
            printf("  [%s] %s\n", ip, tmp);

            char reply[BUF_SIZE + 8];
            snprintf(reply, sizeof(reply), "ECHO: %s", buf);
            send(connfd, reply, strlen(reply), 0);
        }

        printf("[rozłączono] %s\n", ip);
        close(connfd);
    }

    close(sockfd);
    return 0;
}