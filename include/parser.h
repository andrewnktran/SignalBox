#pragma once
#include <cstdint>
#include <vector>

struct TelemetryPacket {
    uint32_t timestamp;
    uint16_t sensor_id;
    int16_t value;
};

// Generate a random binary packet
std::vector<uint8_t> generateRawPacket();

// Parse 8-byte binary data into a structured packet
TelemetryPacket parseRawPacket(const std::vector<uint8_t>& data);
