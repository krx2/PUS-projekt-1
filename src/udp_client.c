/*
 * udp_client.c – klient UDP/IPv4
 * Kompilacja: gcc udp_client.c -o udp_client
 * Użycie:     ./udp_client [host] [port]   (domyślnie 127.0.0.1 9000)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT  9000
#define BUF_SIZE      1024

int main(int argc, char *argv[]) {
    const char *host = (argc > 1) ? argv[1] : DEFAULT_HOST;
    int         port = (argc > 2) ? atoi(argv[2]) : DEFAULT_PORT;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in srv = {
        .sin_family = AF_INET,
        .sin_port   = htons(port)
    };
    if (inet_pton(AF_INET, host, &srv.sin_addr) <= 0) {
        fprintf(stderr, "Nieprawidłowy adres: %s\n", host); exit(1);
    }

    printf("[UDP klient] cel: %s:%d  (wpisz wiadomość, Ctrl+D aby wyjść)\n",
           host, port);

    char buf[BUF_SIZE];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) break;

        /* usuń newline */
        buf[strcspn(buf, "\n")] = '\0';
        if (!buf[0]) continue;

        sendto(sockfd, buf, strlen(buf), 0,
               (struct sockaddr *)&srv, sizeof(srv));

        /* odbierz odpowiedź */
        char reply[BUF_SIZE];
        socklen_t srv_len = sizeof(srv);
        ssize_t n = recvfrom(sockfd, reply, BUF_SIZE - 1, 0,
                             (struct sockaddr *)&srv, &srv_len);
        if (n < 0) { perror("recvfrom"); continue; }
        reply[n] = '\0';
        printf("< %s\n", reply);
    }

    close(sockfd);
    return 0;
}