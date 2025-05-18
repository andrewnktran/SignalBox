#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include "parser.h"

int main(int argc, char* argv[]) {
    std::cout << "SignalBox Starting...\n";

    int count = 10;
    std::string logFileName;
    bool appendMode = false;
    bool userProvidedLog = false;
    int sensorFilter = -1;  // -1 means no filter

    // Parse CLI arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.rfind("--count=", 0) == 0) {
            count = std::stoi(arg.substr(8));
        }
        else if (arg.rfind("--log=", 0) == 0) {
            logFileName = arg.substr(6);
            userProvidedLog = true;
        }
        else if (arg == "--append") {
            appendMode = true;
        }
        else if (arg.rfind("--sensor=", 0) == 0) {
            sensorFilter = std::stoi(arg.substr(9));
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        }
    }

    // Generate default log filename if not provided
    if (!userProvidedLog) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "log_" << std::put_time(std::localtime(&t), "%Y%m%d_%H%M%S") << ".csv";
        logFileName = ss.str();
    }

    std::ofstream logfile;
    if (appendMode) {
        logfile.open(logFileName, std::ios::app);
    }
    else {
        logfile.open(logFileName);
        logfile << "timestamp,sensor_id,value\n";
    }

    int written = 0;
    while (written < count) {
        std::vector<uint8_t> raw = generateRawPacket();
        TelemetryPacket pkt = parseRawPacket(raw);

        if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) {
            continue; // Skip non-matching packets
        }

        std::cout << "Timestamp: " << pkt.timestamp
            << ", Sensor ID: " << pkt.sensor_id
            << ", Value: " << pkt.value << std::endl;

        logfile << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
        ++written;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    logfile.close();
    return 0;
}
