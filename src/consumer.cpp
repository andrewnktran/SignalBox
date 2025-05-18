// src/consumer.cpp
#include "consumer.h"
#include <iostream>

void startConsumer(PacketQueue& queue) {
    while (!queue.empty()) {
        TelemetryPacket pkt = queue.front();
        queue.pop();

        std::cout << "[Consumer] Processed packet: "
            << pkt.timestamp << ", Sensor: " << (int)pkt.sensor_id
            << ", Value: " << pkt.value << "\n";
    }
}
