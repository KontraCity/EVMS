#include <stdio.h>

#include "drivers/can_bus.hpp"
using namespace evms;

extern "C" void app_main() {
    Drivers::CanBus canBus("Car", GPIO_NUM_5, GPIO_NUM_4);
    while (true) {
        Drivers::CanBus::Message message = canBus.receive();
        printf("ID: %03X | ", static_cast<unsigned int>(message.id));
        for (int index = 0; index < message.length; index++)
            printf("%02X ", message.data[index]);
        for (int index = message.length; index < 8; index++)
            printf(".. ");
        printf("\n");
    }
}
