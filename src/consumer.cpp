// src/consumer.cpp
#include "consumer.h"
#include <fstream>
#include <iostream>

void startConsumer(PacketQueue& queue, const std::string& logFile, bool binary) {
    std::ofstream out;
    if (binary) {
        out.open(logFile, std::ios::binary);
    }
    else {
        out.open(logFile);
        out << "timestamp,sensor_id,value\n";
    }

    while (true) {
        TelemetryPacket pkt;

        {
            std::unique_lock<std::mutex> lock(queue.mtx);
            queue.cv.wait(lock, [&] { return !queue.queue.empty() || queue.done; });

            if (queue.queue.empty() && queue.done) break;

            pkt = queue.queue.front();
            queue.queue.pop();
        }

        std::cout << "[Consumer] " << pkt.timestamp << ", Sensor: "
            << (int)pkt.sensor_id << ", Value: " << pkt.value << "\n";

        if (binary) {
            out.write(reinterpret_cast<const char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            out.write(reinterpret_cast<const char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            out.write(reinterpret_cast<const char*>(&pkt.value), sizeof(pkt.value));
        }
        else {
            out << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
        }
    }

    out.close();
}
