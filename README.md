# Projekt PUS nr 1 – Konrad Żebro

## Struktura

```
src/
├── udp_server.c          # ocena 3.0
├── udp_client.c          # ocena 3.0
├── tcp_ipv6_server.c     # ocena 3.5
├── tcp_ipv6_client.c     # ocena 3.5
├── raw_sniffer.c         # ocena 4.0 / 4.5
└── Makefile
```

---

## Kompilacja

```bash
cd src
make          # kompiluje wszystkie programy
make clean    # usuwa pliki wynikowe
```

---

## Ocena 3.0 – UDP/IPv4

### Serwer
```bash
./udp_server          # port domyślny 9000
./udp_server 8888     # własny port
```

### Klient
```bash
./udp_client                   # 127.0.0.1:9000
./udp_client 127.0.0.1 8888
```

Klient czyta linie ze standardowego wejścia i wysyła je do serwera.
Serwer odpowiada `ECHO: <wiadomość>`.

---

## Ocena 3.5 – TCP/IPv6

### Serwer
```bash
./tcp_ipv6_server         # port domyślny 9001
./tcp_ipv6_server 9999
```

### Klient
```bash
./tcp_ipv6_client              # ::1:9001
./tcp_ipv6_client ::1 9999
```

Serwer z `IPV6_V6ONLY=0` akceptuje zarówno IPv6 jak i IPv4-mapped
(`::ffff:127.0.0.1`).

---

## Ocena 4.0 – Raw sniffer (z sudo)

```bash
sudo ./raw_sniffer
```

Przechwytuje wszystkie pakiety IPv4 na interfejsach Ethernet.
Wyświetla:
- adresy src → dst, protokół, TTL, długość
- porty i flagi (TCP) lub długość (UDP)
- podgląd ładunku (hex, pierwsze 32 bajty)

---

## Ocena 4.5 – Raw sniffer bez sudo (setcap)

```bash
make setcap           # jednorazowo – nadaje cap_net_raw
./raw_sniffer         # już bez sudo
```

`setcap cap_net_raw+ep` przypisuje tylko niezbędną capability do pliku
binarnego, bez dawania pełnych uprawnień roota.

---

## Ocena 5.0 – Firewall przez iptables

### Blokowanie ruchu przychodzącego na port UDP 9000

```bash
# Dodaj regułę
sudo iptables -A INPUT -p udp --dport 9000 -j DROP

# Sprawdź reguły
sudo iptables -L INPUT -v -n

# Usuń regułę (po testach)
sudo iptables -D INPUT -p udp --dport 9000 -j DROP
```

### Blokowanie TCP/IPv6 na porcie 9001

```bash
sudo ip6tables -A INPUT -p tcp --dport 9001 -j DROP
sudo ip6tables -L INPUT -v -n
sudo ip6tables -D INPUT -p tcp --dport 9001 -j DROP
```

### Logowanie zamiast cichego DROP

```bash
# Najpierw loguj, potem drop
sudo iptables -A INPUT -p udp --dport 9000 -j LOG --log-prefix "PUS-DROP: "
sudo iptables -A INPUT -p udp --dport 9000 -j DROP

# Sprawdź logi (WSL → zwykle /var/log/kern.log lub dmesg)
sudo dmesg | grep PUS-DROP
```

### Zapis reguł (trwałość po restarcie)

```bash
sudo apt install iptables-persistent
sudo netfilter-persistent save
```

> **Uwaga WSL:** W WSL2 jądro nie obsługuje pełnego `iptables`
> w trybie legacy. Jeśli polecenie nie działa, sprawdź:
> ```bash
> sudo update-alternatives --set iptables /usr/sbin/iptables-legacy
> sudo update-alternatives --set ip6tables /usr/sbin/ip6tables-legacy
> ```

---

## Szybki test end-to-end

```bash
# Terminal 1
./udp_server

# Terminal 2
./udp_client
> hello
< ECHO: hello
```