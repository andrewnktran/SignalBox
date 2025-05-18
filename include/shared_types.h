// include/shared_types.h
#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "parser.h"

struct PacketQueue {
    std::queue<TelemetryPacket> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
};
