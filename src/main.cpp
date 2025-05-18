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
#include "parser.h"

int main(int argc, char* argv[]) {
    std::cout << "SignalBox Starting...\n";

    int sensorFilter = -1;
    std::string decodeFile;
    bool showStats = false;

    // Step 1: Early pass for --decode, --sensor, --stats
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.rfind("--decode=", 0) == 0) {
            decodeFile = arg.substr(9);
        }
        else if (arg.rfind("--sensor=", 0) == 0) {
            sensorFilter = std::stoi(arg.substr(9));
        }
        else if (arg == "--stats") {
            showStats = true;
        }
    }

    // Step 2: Decode mode
    if (!decodeFile.empty()) {
        std::ifstream input(decodeFile, std::ios::binary);
        if (!input) {
            std::cerr << "Failed to open " << decodeFile << " for reading.\n";
            return 1;
        }

        std::string csvFile = decodeFile.substr(0, decodeFile.find_last_of('.')) + "_decoded.csv";
        std::ofstream output(csvFile);
        output << "timestamp,sensor_id,value\n";

        while (input.peek() != EOF) {
            TelemetryPacket pkt;
            input.read(reinterpret_cast<char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            input.read(reinterpret_cast<char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            input.read(reinterpret_cast<char*>(&pkt.value), sizeof(pkt.value));

            if (input.gcount() < sizeof(pkt.value)) break;

            if (sensorFilter == -1 || pkt.sensor_id == sensorFilter) {
                output << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
            }
        }

        std::cout << "Decoded " << decodeFile << " to " << csvFile;
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
    bool liveMode = false;
    std::string outputFormat = "csv";

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
        else if (arg == "--live") {
            liveMode = true;
        }
        else if (arg.rfind("--format=", 0) == 0) {
            outputFormat = arg.substr(9);
        }
        else if (arg == "--stats") {
            showStats = true;
        }
        else if (arg.rfind("--sensor=", 0) == 0 || arg.rfind("--decode=", 0) == 0) {
            // already handled
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
            << (outputFormat == "json" ? ".json" : (binaryMode ? ".bin" : ".csv"));
        logFileName = ss.str();
    }

    std::ofstream logfile;
    if (binaryMode) {
        logfile.open(logFileName, std::ios::binary | (appendMode ? std::ios::app : std::ios::trunc));
    }
    else {
        logfile.open(logFileName, appendMode ? std::ios::app : std::ios::trunc);
        if (!appendMode && outputFormat == "csv") {
            logfile << "timestamp,sensor_id,value\n";
        }
    }

    std::map<int, std::vector<int>> sensorData;

    int written = 0;
    while (written < count) {
        std::vector<uint8_t> raw = generateRawPacket();
        TelemetryPacket pkt = parseRawPacket(raw);

        if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) {
            continue;
        }

        if (showStats) {
            sensorData[pkt.sensor_id].push_back(pkt.value);
        }

        if (liveMode) {
            std::cout << "\rTimestamp: " << pkt.timestamp << ", Sensor ID: " << pkt.sensor_id << ", Value: " << pkt.value << std::flush;
        }
        else {
            std::cout << "Timestamp: " << pkt.timestamp << ", Sensor ID: " << pkt.sensor_id << ", Value: " << pkt.value << "\n";
        }

        if (binaryMode) {
            logfile.write(reinterpret_cast<const char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            logfile.write(reinterpret_cast<const char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            logfile.write(reinterpret_cast<const char*>(&pkt.value), sizeof(pkt.value));
        }
        else if (outputFormat == "json") {
            logfile << "{\"timestamp\":" << pkt.timestamp
                << ",\"sensor_id\":" << pkt.sensor_id
                << ",\"value\":" << pkt.value << "},\n";
        }
        else {
            logfile << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
        }

        ++written;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    logfile.close();

    if (showStats) {
        std::cout << "\n\nTelemetry Summary:\n";
        for (const auto& [sensor_id, values] : sensorData) {
            int count = values.size();
            double avg = 0.0;
            for (int v : values) avg += v;
            avg /= count;
            std::cout << "Sensor " << sensor_id << " — Count: " << count << ", Avg: " << avg << "\n";
        }
    }

    return 0;
}
