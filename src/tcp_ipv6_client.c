/*
 * tcp_ipv6_client.c – klient TCP/IPv6
 * Kompilacja: gcc tcp_ipv6_client.c -o tcp_ipv6_client
 * Użycie:     ./tcp_ipv6_client [host] [port]
 *             domyślnie: ::1  9001
 *
 * Przykłady hostów: ::1   (localhost IPv6)
 *                   ::ffff:127.0.0.1  (IPv4-mapped)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_HOST "::1"
#define DEFAULT_PORT  9001
#define BUF_SIZE      1024

int main(int argc, char *argv[]) {
    const char *host = (argc > 1) ? argv[1] : DEFAULT_HOST;
    int         port = (argc > 2) ? atoi(argv[2]) : DEFAULT_PORT;

    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in6 srv = {
        .sin6_family = AF_INET6,
        .sin6_port   = htons(port)
    };
    if (inet_pton(AF_INET6, host, &srv.sin6_addr) <= 0) {
        fprintf(stderr, "Nieprawidłowy adres IPv6: %s\n", host); exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("connect"); exit(1);
    }

    printf("[TCP/IPv6 klient] połączono z [%s]:%d\n", host, port);
    printf("Wpisz wiadomość (Ctrl+D aby wyjść)\n");

    char buf[BUF_SIZE];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) break;

        if (send(sockfd, buf, strlen(buf), 0) < 0) {
            perror("send"); break;
        }

        char reply[BUF_SIZE];
        ssize_t n = recv(sockfd, reply, BUF_SIZE - 1, 0);
        if (n <= 0) { printf("[serwer zamknął połączenie]\n"); break; }
        reply[n] = '\0';
        printf("< %s", reply);
        if (reply[n-1] != '\n') printf("\n");
    }

    close(sockfd);
    return 0;
}