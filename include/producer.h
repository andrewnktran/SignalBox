// include/producer.h
#pragma once
#include "shared_types.h"

void startProducer(PacketQueue& queue, int count, int sensorFilter);
