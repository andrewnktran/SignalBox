#include "sender.h"
#include <iostream>
#include <cstring>

#if defined(_WIN32)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

void sendPacketUDP(const TelemetryPacket& pkt) {
    // Serialize packet into buffer
    char buffer[sizeof(pkt.timestamp) + sizeof(pkt.sensor_id) + sizeof(pkt.value)];
    std::memcpy(buffer, &pkt.timestamp, sizeof(pkt.timestamp));
    std::memcpy(buffer + sizeof(pkt.timestamp), &pkt.sensor_id, sizeof(pkt.sensor_id));
    std::memcpy(buffer + sizeof(pkt.timestamp) + sizeof(pkt.sensor_id), &pkt.value, sizeof(pkt.value));

#if defined(_WIN32)
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(sock, buffer, sizeof(buffer), 0, (sockaddr*)&addr, sizeof(addr));

#if defined(_WIN32)
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
}
