// include/consumer.h
#pragma once
#include "shared_types.h"
#include <string>

void startConsumer(PacketQueue& queue, const std::string& logFile, bool binary);
