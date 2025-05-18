#pragma once

#include "parser.h"

void sendPacketUDP(const TelemetryPacket& pkt);
void cleanupSender();
