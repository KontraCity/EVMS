#include "drivers/pwm_led.hpp"
#include "utility/time.hpp"
using namespace evms;

extern "C" void app_main() {
    Drivers::PwmLed led(22);
    const uint8_t step = 3;

    for (int index = 0; index < 5; ++index) {
        for (uint8_t brightness = 0; brightness < 255; brightness += step) {
            led.setBrightness(brightness);
            Utility::Sleep(0.01);
        }

        for (uint8_t brightness = 255; brightness > 0; brightness -= step) {
            led.setBrightness(brightness);
            Utility::Sleep(0.01);
        }
    }  
}
