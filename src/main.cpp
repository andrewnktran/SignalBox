#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>
#include "parser.h"

int main(int argc, char* argv[]) {
    std::cout << "SignalBox Starting...\n";

    int count = 10;
    std::string logFileName = "telemetry_log.csv";
    bool appendMode = false;

    // Parse CLI args
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.rfind("--count=", 0) == 0) {
            count = std::stoi(arg.substr(8));
        }
        else if (arg.rfind("--log=", 0) == 0) {
            logFileName = arg.substr(6);
        }
        else if (arg == "--append") {
            appendMode = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        }
    }

    std::ofstream logfile;
    if (appendMode) {
        logfile.open(logFileName, std::ios::app);
    }
    else {
        logfile.open(logFileName);
        logfile << "timestamp,sensor_id,value\n";
    }

    for (int i = 0; i < count; ++i) {
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
