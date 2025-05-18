# 📡 SignalBox

**SignalBox** is a lightweight C++ telemetry simulator that generates, logs, decodes, and optionally sends structured sensor data packets over UDP. It is designed for practicing modern C++ features and mimicking simplified avionics or embedded data processing pipelines.

---

## 🚀 Features

- ✅ Generate fake telemetry packets with timestamp, sensor ID, and value
- ✅ Output in `CSV`, `JSON`, or raw binary `.bin` format
- ✅ Decode `.bin` files back into readable formats
- ✅ Filter packets by `--sensor=<id>`
- ✅ Append to existing logs using `--append`
- ✅ Display live summaries (`--live`, `--stats`)
- ✅ Send packets over UDP with `--send`
- ✅ Multi-threaded packet production/consumption with `--threads`
- ✅ Dockerized build and execution

---

## 🛠️ Technologies Used

- **C++17** – Modern C++ language features
- **CMake** – Build system
- **Docker** – Containerization
- **GoogleTest** – Unit testing
- **MSVC / GCC** – Cross-platform compilation
- **Sockets (UDP)** – Lightweight packet transmission

---

## 🏁 Getting Started

### 🔧 Build (Locally)
```bash
mkdir build && cd build
cmake ..
cmake --build .

### 🔧 Build (Locally)
# Generate 10 packets and log to CSV
./SignalBox --count=10 --log=telemetry.csv

# Generate and log to binary
./SignalBox --count=10 --binary --log=telemetry.bin

# Decode binary file
./SignalBox --decode=telemetry.bin

# Decode and show sensor 3 only
./SignalBox --decode=telemetry.bin --sensor=3

# With stats + output summary
./SignalBox --decode=telemetry.bin --stats --summary-log=summary.csv

# Live summary view
./SignalBox --live --count=10

# Send packets via UDP
./SignalBox --count=5 --send

### 🔧 Build (Locally)

🔨 Build Docker Image
docker build -t signalbox .

🚀 Run from Container
docker run --rm signalbox --count=5 --send