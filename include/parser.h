#pragma once
#include <cstdint>
#include <queue>
#include <mutex>
#include <condition_variable>

struct TelemetryPacket {
    uint32_t timestamp;
    uint16_t sensor_id;
    int16_t value;
};

class PacketQueue {
public:
    void push(const TelemetryPacket& packet) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(packet);
        cv.notify_one();
    }

    TelemetryPacket pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !q.empty(); });
        TelemetryPacket packet = q.front();
        q.pop();
        return packet;
    }

private:
    std::queue<TelemetryPacket> q;
    std::mutex mtx;
    std::condition_variable cv;
};
