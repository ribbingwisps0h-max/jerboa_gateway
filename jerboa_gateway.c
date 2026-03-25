#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#define LISTEN_PORT 5000
#define JTP_ETH_TYPE 0x88B5

int main() {
    int udp_sock, raw_sock;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buffer[2048];

    // 1. Создаем UDP сокет для приема из WAN (eth0)
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(LISTEN_PORT);
    bind(udp_sock, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    // 2. Создаем Raw сокет для отправки в LAN (eth4)
    raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_ifindex = if_nametoindex("eth4");
    socket_address.sll_halen = ETH_ALEN;
    uint8_t mac_macbook[] = {0x26, 0xA6, 0x26, 0x24, 0xAC, 0x76};
    memcpy(socket_address.sll_addr, mac_macbook, ETH_ALEN);

    printf("🚀 Jerboa Gateway запущен: eth0(UDP:5000) -> eth4(L2:0x88B5)\n");

    while (1) {
        int n = recvfrom(udp_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) continue;

        // Формируем Ethernet заголовок для локалки
        unsigned char frame[2048];
        struct ethhdr *eth = (struct ethhdr *)frame;

        memcpy(eth->h_dest, mac_macbook, ETH_ALEN);
        // MAC-адрес eth4 роутера (подставится автоматически или впиши сам)
        memset(eth->h_source, 0, ETH_ALEN);
        eth->h_proto = htons(JTP_ETH_TYPE);

        // Копируем данные JTP из UDP-пакета сразу после Ethernet заголовка
        memcpy(frame + sizeof(struct ethhdr), buffer, n);

        // "Выстреливаем" пакет в eth4
        sendto(raw_sock, frame, n + sizeof(struct ethhdr), 0,
               (struct sockaddr*)&socket_address, sizeof(socket_address));

        printf("📡 Транслирован пакет: %d байт\n", n);
    }

    return 0;
}