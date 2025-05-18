#include "parser.h"
#include <random>
#include <ctime>
#include <vector>
#include <stdexcept>

std::vector<uint8_t> generateRawPacket() {
    TelemetryPacket pkt;
    pkt.timestamp = static_cast<uint32_t>(time(nullptr));
    pkt.sensor_id = rand() % 11;
    pkt.value = static_cast<int16_t>(rand() % 201 - 100);

    std::vector<uint8_t> data(8);
    data[0] = pkt.timestamp & 0xFF;
    data[1] = (pkt.timestamp >> 8) & 0xFF;
    data[2] = (pkt.timestamp >> 16) & 0xFF;
    data[3] = (pkt.timestamp >> 24) & 0xFF;
    data[4] = pkt.sensor_id & 0xFF;
    data[5] = (pkt.sensor_id >> 8) & 0xFF;
    data[6] = pkt.value & 0xFF;
    data[7] = (pkt.value >> 8) & 0xFF;

    return data;
}

TelemetryPacket parseRawPacket(const std::vector<uint8_t>& data) {
    if (data.size() != 8) {
        throw std::runtime_error("Invalid packet length");
    }

    TelemetryPacket pkt;
    pkt.timestamp = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    pkt.sensor_id = data[4] | (data[5] << 8);
    pkt.value = static_cast<int16_t>(data[6] | (data[7] << 8));

    return pkt;
}
