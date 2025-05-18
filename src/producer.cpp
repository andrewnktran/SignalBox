// src/producer.cpp
#include "producer.h"
#include <thread>
#include <chrono>

void startProducer(PacketQueue& queue, int count, int sensorFilter) {
    for (int i = 0; i < count; ++i) {
        auto raw = generateRawPacket();
        TelemetryPacket pkt = parseRawPacket(raw);

        if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(queue.mtx);
            queue.queue.push(pkt);
        }

        queue.cv.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    {
        std::lock_guard<std::mutex> lock(queue.mtx);
        queue.done = true;
    }
    queue.cv.notify_all();
}
