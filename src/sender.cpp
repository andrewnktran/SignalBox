#include "sender.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

namespace {
    bool initialized = false;
    SOCKET sock;
    sockaddr_in serverAddr;
}

void sendPacketUDP(const TelemetryPacket& pkt) {
    if (!initialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "[Send] WSAStartup failed.\n";
            return;
        }

        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "[Send] Failed to create socket.\n";
            WSACleanup();
            return;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9000);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        initialized = true;
    }

    sendto(sock, reinterpret_cast<const char*>(&pkt), sizeof(pkt), 0,
        reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
}

void cleanupSender() {
    if (initialized) {
        closesocket(sock);
        WSACleanup();
        initialized = false;
    }
}
