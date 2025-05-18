#include "consumer.h"
#include <iostream>

void startConsumer(PacketQueue& queue) {
    for (int i = 0; i < 10; ++i) {
        TelemetryPacket packet = queue.pop();
        std::cout << "Timestamp: " << packet.timestamp
                  << ", Sensor ID: " << packet.sensor_id
                  << ", Value: " << packet.value << std::endl;
    }
}
