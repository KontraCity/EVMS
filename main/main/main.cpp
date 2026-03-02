#include "drivers/gpio_pin.hpp"
#include "drivers/pwm_led.hpp"
#include "drivers/spi_bus.hpp"
#include "utility/time.hpp"
using namespace evms;

/*
*   Connection to the 4" TFT display:
*   Screen      ESP32     Wire
*   VCC         3.3       (Red)
*   GND         GND       (Dark)
*   CS           15       (Green)
*   RESET         4       (Gray)
*   DC            2       (Green)
*   MOSI         23       (Orange)
*   SCK          18       (Orange)
*   LED          22       (Blue)
*   MISO         19       (Orange)
*/

static std::vector<uint8_t> Command(Drivers::GpioPin& dc, Drivers::SpiDevice& controller, uint8_t command, const std::vector<uint8_t>& data = {}, float delay = 0.0f, int responseSize = 0) {
    dc.write(false);
    controller.send({ command });

    if (!data.empty()) {
        dc.write(true);
        controller.send(data);
    }

    Utility::Sleep(delay);
    if (!responseSize)
        return {};

    dc.write(true);
    return controller.receive(responseSize);
}

extern "C" void app_main() {
    Drivers::GpioPin dc(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    Drivers::SpiBus spi(SPI2_HOST, GPIO_NUM_18, GPIO_NUM_23, GPIO_NUM_19);
    Drivers::SpiDevice controller = spi.newDevice(GPIO_NUM_15, 80'000'000, true);

    Drivers::GpioPin reset(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    reset.write(false);
    Utility::Sleep(0.05);
    reset.write(true);
    Utility::Sleep(0.12);

    dc.write(false);
    Command(dc, controller, 0x11, {}, 0.12f);           // Sleep out
    Command(dc, controller, 0x29);                      // Display on
    Command(dc, controller, 0x3A, { 0b0'101'0'101 });   // Interface pixel format
    Command(dc, controller, 0x36, { 0b000'010'00 });    // Memory data access control

    Drivers::PwmLed backlight(LEDC_CHANNEL_0, GPIO_NUM_22);
    for (float percentage = 0.0f; percentage <= 100.0f; percentage += 5.0f) {
        backlight.setDutyPercent(percentage);
        Utility::Sleep(0.01);
    }

    // Column address set
    int columnStart = 0, columnEnd = 239;
    Command(dc, controller, 0x2A, { (uint8_t)(columnStart >> 8), (uint8_t)(columnStart), (uint8_t)(columnEnd >> 8), (uint8_t)(columnEnd) });

    // Row address set
    int rowStart = 0, rowEnd = 319;
    Command(dc, controller, 0x2B, { (uint8_t)(rowStart >> 8), (uint8_t)(rowStart), (uint8_t)(rowEnd >> 8), (uint8_t)(rowEnd) });

    // Some data
    Command(dc, controller, 0x2C);
    std::vector<uint8_t> line(240 * 2);
    for (int index = 0; true; ++index) {
        constexpr uint8_t FullColor = 0b11111;
        uint8_t red = 0, green = 0, blue = 0;
        switch (index % 3) { 
            case 0: red   = FullColor; break;
            case 1: green = FullColor; break;
            case 2: blue  = FullColor; break;
        }

        uint16_t pixel = (red << 11) | (green << 5) | blue;
        for (int index = 0; index < line.size(); index += 2) {
            line[index] = static_cast<uint8_t>(pixel >> 8);
            line[index + 1] = static_cast<uint8_t>(pixel);
        }

        dc.write(true);
        for (int index = 0; index < 320; ++index)
            controller.send(line);
        Utility::Sleep(1);
    }
}
