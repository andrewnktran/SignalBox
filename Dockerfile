FROM gcc:12

LABEL name="SignalBox"

RUN apt-get update && \
    apt-get install -y cmake libgtest-dev && \
    cd /usr/src/gtest && \
    cmake CMakeLists.txt && \
    make && \
    cp *.a /usr/lib

WORKDIR /app
COPY . /app

RUN cmake -S . -B build && cmake --build build

CMD ["./build/SignalBox"]
