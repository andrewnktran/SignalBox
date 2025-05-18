#include "parser.h"
#include <random>
#include <ctime>

TelemetryPacket generatePacket() {
    static std::default_random_engine gen(std::random_device{}());
    static std::uniform_int_distribution<int16_t> value_dist(-100, 100);
    static std::uniform_int_distribution<uint16_t> sensor_dist(0, 10);

    TelemetryPacket pkt;
    pkt.timestamp = static_cast<uint32_t>(time(nullptr));
    pkt.sensor_id = sensor_dist(gen);
    pkt.value = value_dist(gen);
    return pkt;
}
