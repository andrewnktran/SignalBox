# Use a C++ build image
FROM debian:bookworm AS builder

RUN apt-get update && \
    apt-get install -y g++ cmake make

WORKDIR /app
COPY . .

RUN mkdir build && cd build && cmake .. && cmake --build .

# Final image (slim)
FROM debian:bookworm

WORKDIR /app
COPY --from=builder /app/build/SignalBox /app/SignalBox

ENTRYPOINT ["./SignalBox"]
