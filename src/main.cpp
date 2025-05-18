#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "parser.h"

int main() {
    std::cout << "SignalBox Starting...\n";

    std::ofstream logfile("telemetry_log.csv");
    logfile << "timestamp,sensor_id,value\n";

    for (int i = 0; i < 10; ++i) {
        TelemetryPacket pkt = generatePacket();

        std::cout << "Timestamp: " << pkt.timestamp
            << ", Sensor ID: " << pkt.sensor_id
            << ", Value: " << pkt.value << std::endl;

        logfile << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    logfile.close();
    return 0;
}
