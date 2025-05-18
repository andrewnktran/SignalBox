#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <map>
#include <algorithm>
#include "parser.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
}

int main(int argc, char* argv[]) {
    std::cout << "SignalBox Starting...\n";

    int sensorFilter = -1;
    std::string decodeFile;
    std::string format = "csv";
    bool liveMode = false;

    // Step 1: Early pass for --decode and --sensor
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.rfind("--decode=", 0) == 0) {
            decodeFile = arg.substr(9);
        }
        else if (arg.rfind("--sensor=", 0) == 0) {
            sensorFilter = std::stoi(arg.substr(9));
        }
        else if (arg.rfind("--format=", 0) == 0) {
            format = arg.substr(9);
        }
        else if (arg == "--live") {
            liveMode = true;
        }
    }

    // Step 2: Decode mode
    if (!decodeFile.empty()) {
        std::ifstream input(decodeFile, std::ios::binary);
        if (!input) {
            std::cerr << "Failed to open " << decodeFile << " for reading.\n";
            return 1;
        }

        std::string outputExt = (format == "json") ? "_decoded.json" : "_decoded.csv";
        std::string outputFile = decodeFile.substr(0, decodeFile.find_last_of('.')) + outputExt;
        std::ofstream output(outputFile);

        if (format == "json") {
            output << "[\n";
        }
        else {
            output << "timestamp,sensor_id,value\n";
        }

        bool first = true;
        while (input.peek() != EOF) {
            TelemetryPacket pkt;
            input.read(reinterpret_cast<char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            input.read(reinterpret_cast<char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            input.read(reinterpret_cast<char*>(&pkt.value), sizeof(pkt.value));

            if (input.gcount() < sizeof(pkt.value)) break;

            if (sensorFilter == -1 || pkt.sensor_id == sensorFilter) {
                if (format == "json") {
                    if (!first) output << ",\n";
                    output << "  { \"timestamp\": " << pkt.timestamp << ", \"sensor_id\": " << pkt.sensor_id << ", \"value\": " << pkt.value << " }";
                    first = false;
                }
                else {
                    output << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
                }
            }
        }

        if (format == "json") {
            output << "\n]\n";
        }

        std::cout << "Decoded " << decodeFile << " to " << outputFile;
        if (sensorFilter != -1) {
            std::cout << " (filtered by sensor " << sensorFilter << ")";
        }
        std::cout << "\n";
        return 0;
    }

    // Step 3: Packet generation mode
    int count = 10;
    std::string logFileName;
    bool appendMode = false;
    bool userProvidedLog = false;
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
        else if (arg == "--binary") {
            binaryMode = true;
        }
    }

    if (!userProvidedLog) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << (binaryMode ? "telemetry_" : "log_")
            << std::put_time(std::localtime(&t), "%Y%m%d_%H%M%S");
        ss << ((format == "json") ? ".json" : (binaryMode ? ".bin" : ".csv"));
        logFileName = ss.str();
    }

    std::ofstream logfile;
    if (binaryMode) {
        logfile.open(logFileName, std::ios::binary | (appendMode ? std::ios::app : std::ios::trunc));
    }
    else {
        logfile.open(logFileName, appendMode ? std::ios::app : std::ios::trunc);
        if (!appendMode && format == "csv") {
            logfile << "timestamp,sensor_id,value\n";
        }
        else if (!appendMode && format == "json") {
            logfile << "[\n";
        }
    }

    std::map<int, TelemetryPacket> latestBySensor;
    bool firstJson = true;

    for (int i = 0; i < count; ++i) {
        TelemetryPacket pkt = parseRawPacket(generateRawPacket());
        if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) continue;

        if (liveMode) {
            latestBySensor[pkt.sensor_id] = pkt;
            clearScreen();
            std::cout << "Live Telemetry:\n";
            for (const auto& [id, p] : latestBySensor) {
                std::cout << "Sensor " << id << ": Value = " << p.value << " at " << p.timestamp << "\n";
            }
        }
        else {
            std::cout << "Timestamp: " << pkt.timestamp << ", Sensor ID: " << pkt.sensor_id << ", Value: " << pkt.value << "\n";
        }

        if (binaryMode) {
            logfile.write(reinterpret_cast<const char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            logfile.write(reinterpret_cast<const char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            logfile.write(reinterpret_cast<const char*>(&pkt.value), sizeof(pkt.value));
        }
        else if (format == "json") {
            if (!firstJson) logfile << ",\n";
            logfile << "  { \"timestamp\": " << pkt.timestamp << ", \"sensor_id\": " << pkt.sensor_id << ", \"value\": " << pkt.value << " }";
            firstJson = false;
        }
        else {
            logfile << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (format == "json" && !binaryMode) {
        logfile << "\n]\n";
    }

    logfile.close();
    return 0;
}
