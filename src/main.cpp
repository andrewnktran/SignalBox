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
#include "sender.h"

int main(int argc, char* argv[]) {
    std::cout << "SignalBox Starting...\n";

    int sensorFilter = -1;
    std::string decodeFile;
    std::string format = "csv";
    bool enableStats = false;
    std::string summaryLogFile;
    bool enableSend = false;

    bool decodeMode = false;
    bool hasGenerationFlag = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.rfind("--decode=", 0) == 0) {
            decodeMode = true;
        }
        else if (
            arg.rfind("--count=", 0) == 0 ||
            arg.rfind("--log=", 0) == 0 ||
            arg == "--binary" ||
            arg == "--append" ||
            arg == "--threads" ||
            arg == "--live" ||
            arg == "--send"
            ) {
            hasGenerationFlag = true;
        }
    }

    if (!decodeMode && !hasGenerationFlag) {
        std::cout << "Run mode not specified. Use --decode=<file> or see help.\n";
        return 1;
    }

    // Step 1: Decode-related flags
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
        else if (arg == "--stats") {
            enableStats = true;
        }
        else if (arg.rfind("--summary-log=", 0) == 0) {
            summaryLogFile = arg.substr(15);
        }
    }

    // Step 2: Decode Mode
    if (!decodeFile.empty()) {
        std::ifstream input(decodeFile, std::ios::binary);
        if (!input) {
            std::cerr << "Failed to open " << decodeFile << " for reading.\n";
            return 1;
        }

        std::string outFile = decodeFile.substr(0, decodeFile.find_last_of('.')) + "_decoded." + format;
        std::ofstream output(outFile);
        if (format == "csv") output << "timestamp,sensor_id,value\n";
        else if (format == "json") output << "[\n";

        std::map<int, std::vector<int>> sensorStats;
        bool firstJson = true;

        while (input.peek() != EOF) {
            TelemetryPacket pkt;
            input.read(reinterpret_cast<char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            input.read(reinterpret_cast<char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            input.read(reinterpret_cast<char*>(&pkt.value), sizeof(pkt.value));

            if (input.gcount() < sizeof(pkt.value)) break;
            if (sensorFilter != -1 && pkt.sensor_id != sensorFilter) continue;
            if (enableStats) sensorStats[pkt.sensor_id].push_back(pkt.value);

            if (format == "csv") {
                output << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
            }
            else {
                if (!firstJson) output << ",\n";
                output << "  {\"timestamp\":" << pkt.timestamp << ",\"sensor_id\":" << pkt.sensor_id << ",\"value\":" << pkt.value << "}";
                firstJson = false;
            }
        }

        if (format == "json") output << "\n]\n";

        std::cout << "Decoded " << decodeFile << " to " << outFile;
        if (sensorFilter != -1) std::cout << " (filtered by sensor " << sensorFilter << ")";
        std::cout << "\n";

        if (enableStats) {
            std::cout << "\nTelemetry Summary:\n";
            std::ofstream summaryFile;
            if (!summaryLogFile.empty()) {
                summaryFile.open(summaryLogFile);
                summaryFile << "sensor_id,count,average\n";
            }

            for (const auto& [id, values] : sensorStats) {
                int count = values.size();
                double avg = 0;
                for (int v : values) avg += v;
                avg /= count;
                std::cout << "Sensor " << id << " - Count: " << count << ", Avg: " << avg << "\n";
                if (summaryFile.is_open()) {
                    summaryFile << id << "," << count << "," << avg << "\n";
                }
            }

            if (summaryFile.is_open()) {
                std::cout << "Summary written to " << summaryLogFile << "\n";
            }
        }

        return 0;
    }

    // Step 3: Generation mode
    int count = 10;
    std::string logFileName;
    bool appendMode = false;
    bool binaryMode = false;

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
        else if (arg == "--binary") {
            binaryMode = true;
        }
        else if (arg == "--send") {
            enableSend = true;
        }
    }

    if (logFileName.empty()) {
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
        if (!appendMode) logfile << "timestamp,sensor_id,value\n";
    }

    for (int i = 0; i < count; ++i) {
        TelemetryPacket pkt = parseRawPacket(generateRawPacket());

        std::cout << "Timestamp: " << pkt.timestamp
            << ", Sensor ID: " << pkt.sensor_id
            << ", Value: " << pkt.value << "\n";

        if (enableSend) sendPacketUDP(pkt);

        if (binaryMode) {
            logfile.write(reinterpret_cast<const char*>(&pkt.timestamp), sizeof(pkt.timestamp));
            logfile.write(reinterpret_cast<const char*>(&pkt.sensor_id), sizeof(pkt.sensor_id));
            logfile.write(reinterpret_cast<const char*>(&pkt.value), sizeof(pkt.value));
        }
        else {
            logfile << pkt.timestamp << "," << pkt.sensor_id << "," << pkt.value << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    logfile.close();
    return 0;
}
