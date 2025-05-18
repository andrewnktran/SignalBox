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

    // DECODE MODE CHECK
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.rfind("--decode=", 0) == 0) {
            std::string binFile = arg.substr(9);
            std::ifstream input(binFile, std::ios::binary);
            if (!input) {
                std::cerr << "Failed to open " << binFile << " for reading.\n";
                return 1;
            }

            std::string csvFile = binFile.substr(0, binFile.find_last_of('.')) + "_decoded.csv";
            std::ofstream output(csvFile);
            output << "timestamp,sensor_id,value\n";

            while (input.peek() != EOF) {
                TelemetryPacket pkt;
                input.read(reinterpret_cast<char*>(&pkt.timestamp), sizeof(pkt.timestamp));
                input.read(reinterpret_cast<char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
                input.read(reinterpret_cast<char*>(&pkt.value), sizeof(pkt.value));

                if (input.gcount() < sizeof(pkt.value)) break;

                output << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
            }

            std::cout << "Decoded " << binFile << " to " << csvFile << "\n";
            return 0;
        }
    }

    // REGULAR PACKET GENERATION MODE
    int count = 10;
    std::string logFileName;
    bool appendMode = false;
    bool userProvidedLog = false;
    int sensorFilter = -1;
    bool binaryMode = false;

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
        else if (arg == "--binary") {
            binaryMode = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        }
    }

    if (!userProvidedLog) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << (binaryMode ? "telemetry_" : "log_")
            << std::put_time(std::localtime(&t), "%Y%m%d_%H%M%S")
            << (binaryMode ? ".bin" : ".csv");
        logFileName = ss.str();
    }

    std::ofstream logfile;
    if (binaryMode) {
        logfile.open(logFileName, std::ios::binary | (appendMode ? std::ios::app : std::ios::trunc));
    }
    else {
        logfile.open(logFileName, appendMode ? std::ios::app : std::ios::trunc);
        if (!appendMode) {
            logfile << "timestamp,sensor_id,value\n";
        }
    }

    int written = 0;
    while (written < count) {
        std::vector<uint8_t> raw = generateRawPacket();
        TelemetryPacket pkt = parseRawPacket(raw);

        if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) {
            continue;
        }

        std::cout << "Timestamp: " << pkt.timestamp
            << ", Sensor ID: " << pkt.sensor_id
            << ", Value: " << pkt.value << std::endl;

        if (binaryMode) {
            logfile.write(reinterpret_cast<const char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            logfile.write(reinterpret_cast<const char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            logfile.write(reinterpret_cast<const char*>(&pkt.value), sizeof(pkt.value));
        }
        else {
            logfile << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
        }

        ++written;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    logfile.close();
    return 0;
}
