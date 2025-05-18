#include "producer.h"
#include <random>
#include <chrono>

void startProducer(PacketQueue& queue) {
    std::default_random_engine gen(std::random_device{}());
    std::uniform_int_distribution<int16_t> value_dist(-100, 100);
    std::uniform_int_distribution<uint16_t> sensor_dist(0, 10);

    for (int i = 0; i < 10; ++i) {
        TelemetryPacket packet;
        packet.timestamp = static_cast<uint32_t>(time(nullptr));
        packet.sensor_id = sensor_dist(gen);
        packet.value = value_dist(gen);

        queue.push(packet);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
