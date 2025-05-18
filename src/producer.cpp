#include "producer.h"
#include <random>
#include <iostream>
#include <thread>

void startProducer(PacketQueue& queue) {
    for (int i = 0; i < 10; ++i) {
        auto raw = generateRawPacket();
        TelemetryPacket pkt = parseRawPacket(raw);
        queue.push(pkt);

        std::cout << "[Producer] Queued packet: "
            << pkt.timestamp << ", Sensor: " << (int)pkt.sensor_id
            << ", Value: " << pkt.value << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
