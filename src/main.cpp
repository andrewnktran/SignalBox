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

    int sensorFilter = -1;
    std::string decodeFile;
    std::string format = "csv"; // default

    // Early pass: --decode, --sensor, --format
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
    }

    // --- DECODE MODE ---
    if (!decodeFile.empty()) {
        std::ifstream input(decodeFile, std::ios::binary);
        if (!input) {
            std::cerr << "Failed to open " << decodeFile << " for reading.\n";
            return 1;
        }

        std::string base = decodeFile.substr(0, decodeFile.find_last_of('.'));
        std::string outputFile = base + "_decoded." + (format == "json" ? "json" : "csv");
        std::ofstream output(outputFile);
        if (format == "json") output << "[\n";

        bool first = true;
        while (input.peek() != EOF) {
            TelemetryPacket pkt;
            input.read(reinterpret_cast<char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            input.read(reinterpret_cast<char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            input.read(reinterpret_cast<char*>(&pkt.value), sizeof(pkt.value));
            if (input.gcount() < sizeof(pkt.value)) break;

            if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) continue;

            if (format == "json") {
                if (!first) output << ",\n";
                output << "  {\"timestamp\": " << pkt.timestamp
                    << ", \"sensor_id\": " << pkt.sensor_id
                    << ", \"value\": " << pkt.value << "}";
                first = false;
            }
            else {
                if (first) output << "timestamp,sensor_id,value\n";
                output << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
                first = false;
            }
        }

        if (format == "json") output << "\n]\n";
        output.close();

        std::cout << "Decoded " << decodeFile << " to " << outputFile;
        if (sensorFilter != -1) std::cout << " (filtered by sensor " << sensorFilter << ")";
        std::cout << "\n";
        return 0;
    }

    // --- GENERATE MODE ---
    int count = 10;
    std::string logFileName;
    bool appendMode = false;
    bool userProvidedLog = false;
    bool binaryMode = false;
    bool multithreaded = false;

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
        else if (arg == "--threads") {
            multithreaded = true;
        }
    }

    if (!userProvidedLog) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << (binaryMode ? "telemetry_" : "log_")
            << std::put_time(std::localtime(&t), "%Y%m%d_%H%M%S")
            << (format == "json" ? ".json" : binaryMode ? ".bin" : ".csv");
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

    int written = 0;
    bool firstJson = true;
    while (written < count) {
        TelemetryPacket pkt = parseRawPacket(generateRawPacket());

        if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) continue;

        std::cout << "Timestamp: " << pkt.timestamp
            << ", Sensor ID: " << pkt.sensor_id
            << ", Value: " << pkt.value << std::endl;

        if (binaryMode) {
            logfile.write(reinterpret_cast<const char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            logfile.write(reinterpret_cast<const char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            logfile.write(reinterpret_cast<const char*>(&pkt.value), sizeof(pkt.value));
        }
        else if (format == "json") {
            if (!firstJson) logfile << ",\n";
            logfile << "  {\"timestamp\": " << pkt.timestamp
                << ", \"sensor_id\": " << pkt.sensor_id
                << ", \"value\": " << pkt.value << "}";
            firstJson = false;
        }
        else {
            logfile << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
        }

        ++written;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (format == "json") logfile << "\n]\n";
    logfile.close();
    return 0;
}
