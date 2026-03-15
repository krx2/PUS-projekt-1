# Makefile – Projekt PUS nr 1
# Użycie: make          → kompiluje wszystko
#         make clean    → usuwa pliki wynikowe
#         make setcap   → nadaje cap_net_raw snifferowi (bez sudo przy uruchomieniu)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g

TARGETS = udp_server udp_client tcp_ipv6_server tcp_ipv6_client raw_sniffer

.PHONY: all clean setcap

all: $(TARGETS)

udp_server:       udp_server.c
	$(CC) $(CFLAGS) $< -o $@

udp_client:       udp_client.c
	$(CC) $(CFLAGS) $< -o $@

tcp_ipv6_server:  tcp_ipv6_server.c
	$(CC) $(CFLAGS) $< -o $@

tcp_ipv6_client:  tcp_ipv6_client.c
	$(CC) $(CFLAGS) $< -o $@

raw_sniffer:      raw_sniffer.c
	$(CC) $(CFLAGS) $< -o $@

# ocena 4.5 – capability zamiast sudo
setcap: raw_sniffer
	sudo setcap cap_net_raw+ep ./raw_sniffer
	@echo "cap_net_raw nadane – możesz uruchamiać bez sudo"

clean:
	rm -f $(TARGETS)