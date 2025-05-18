#include <iostream>
#include <thread>
#include "parser.h"
#include "producer.h"
#include "consumer.h"

int main() {
    std::cout << "SignalBox Starting...\n";

    PacketQueue queue;

    std::thread producer_thread(startProducer, std::ref(queue));
    std::thread consumer_thread(startConsumer, std::ref(queue));

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
