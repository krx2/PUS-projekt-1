/*
 * raw_sniffer.c – pasywny sniffer pakietów (gniazda surowe)
 * Kompilacja: gcc raw_sniffer.c -o raw_sniffer
 *
 * Uruchomienie (dwie opcje):
 *   a) sudo ./raw_sniffer          ← tymczasowe uprawnienia root
 *   b) sudo setcap cap_net_raw+ep ./raw_sniffer
 *      ./raw_sniffer               ← bez sudo (ocena 4.5)
 *
 * Przechwytuje pakiety IP na wszystkich interfejsach i wyświetla
 * nagłówki IPv4 (src→dst, protokół) + pierwsze bajty ładunku.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/tcp.h>        /* struct tcphdr w środowisku Linux/WSL */
#include <linux/if_ether.h>   /* ETH_P_ALL, ETH_HLEN, struct ethhdr */

#define BUF_SIZE 65536
#define PAYLOAD_PREVIEW 32    /* ile bajtów ładunku pokazujemy */

static volatile int running = 1;
static void handle_sig(int s) { (void)s; running = 0; }

/* wypisz bajty jako hex */
static void print_hex(const unsigned char *data, int len) {
    for (int i = 0; i < len; i++) printf("%02x ", data[i]);
    printf("\n");
}

/* nazwa protokołu IP */
static const char *proto_name(uint8_t proto) {
    switch (proto) {
        case IPPROTO_TCP:  return "TCP";
        case IPPROTO_UDP:  return "UDP";
        case IPPROTO_ICMP: return "ICMP";
        default:           return "OTHER";
    }
}

int main(void) {
    signal(SIGINT, handle_sig);

    /* AF_PACKET + ETH_P_ALL → wszystkie ramki Ethernet */
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket (potrzebne cap_net_raw lub sudo)");
        exit(1);
    }

    printf("[raw_sniffer] nasłuchuję... (Ctrl+C aby zatrzymać)\n\n");

    unsigned char buf[BUF_SIZE];
    long pkt_count = 0;

    while (running) {
        ssize_t n = recvfrom(sockfd, buf, BUF_SIZE, 0, NULL, NULL);
        if (n < 0) break;

        /* pomiń ramkę jeśli krótsza niż nagłówek Ethernet + IP */
        if (n < (ssize_t)(ETH_HLEN + sizeof(struct iphdr))) continue;

        /* pomiń ramki nie-IP (ethertype != 0x0800) */
        struct ethhdr *eth = (struct ethhdr *)buf;
        if (ntohs(eth->h_proto) != ETH_P_IP) continue;

        struct iphdr *ip = (struct iphdr *)(buf + ETH_HLEN);
        int ip_hdr_len   = ip->ihl * 4;

        char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip->saddr, src, sizeof(src));
        inet_ntop(AF_INET, &ip->daddr, dst, sizeof(dst));

        printf("--- Pakiet #%ld ---\n", ++pkt_count);
        printf("  IPv4  %s → %s  proto=%s  ttl=%d  len=%d\n",
               src, dst, proto_name(ip->protocol),
               ip->ttl, ntohs(ip->tot_len));

        /* port src/dst dla TCP i UDP */
        unsigned char *transport = buf + ETH_HLEN + ip_hdr_len;
        int transport_len = n - ETH_HLEN - ip_hdr_len;

        if (ip->protocol == IPPROTO_TCP && transport_len >= (int)sizeof(struct tcphdr)) {
            struct tcphdr *tcp = (struct tcphdr *)transport;
            printf("  TCP   sport=%d  dport=%d  flags=%s%s%s\n",
                   ntohs(tcp->source), ntohs(tcp->dest),
                   tcp->syn ? "SYN " : "",
                   tcp->ack ? "ACK " : "",
                   tcp->fin ? "FIN " : "");
        } else if (ip->protocol == IPPROTO_UDP && transport_len >= (int)sizeof(struct udphdr)) {
            struct udphdr *udp = (struct udphdr *)transport;
            printf("  UDP   sport=%d  dport=%d  len=%d\n",
                   ntohs(udp->source), ntohs(udp->dest), ntohs(udp->len));
        }

        /* podgląd ładunku */
        int payload_off = ETH_HLEN + ip_hdr_len;
        if (ip->protocol == IPPROTO_TCP)  payload_off += ((struct tcphdr *)transport)->doff * 4;
        if (ip->protocol == IPPROTO_UDP)  payload_off += sizeof(struct udphdr);

        int payload_len = n - payload_off;
        if (payload_len > 0) {
            int show = payload_len < PAYLOAD_PREVIEW ? payload_len : PAYLOAD_PREVIEW;
            printf("  dane  [%d B] ", payload_len);
            print_hex(buf + payload_off, show);
        }
        printf("\n");
    }

    printf("\n[raw_sniffer] zatrzymano. Przechwycono %ld pakietów.\n", pkt_count);
    close(sockfd);
    return 0;
}