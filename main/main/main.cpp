#include "app/bitmaps.hpp"
#include "app/ui.hpp"
#include "display/screen.hpp"
#include "drivers/gpio_pwm.hpp"
#include "drivers/spi_bus.hpp"
#include "utility/io.hpp"
using namespace evms;

/*
*   Connection to the 4.0" TFT display:
*   Screen      ESP32
*   VCC         3.3
*   GND         GND
*   CS           15
*   RESET         0
*   DC            2
*   MOSI         23
*   SCK          18
*   LED          22
*   MISO         19
*   T_CS         21
*/

extern "C" void app_main() {
    App::Ui::Config config = {
        .txPin    = GPIO_NUM_5,
        .rxPin    = GPIO_NUM_4,
        .csPin    = GPIO_NUM_15,
        .resetPin = GPIO_NUM_0,
        .dcPin    = GPIO_NUM_2,
        .mosiPin  = GPIO_NUM_23,
        .sckPin   = GPIO_NUM_18,
        .ledPin   = GPIO_NUM_22,
        .misoPin  = GPIO_NUM_19,
        .tcsPin   = GPIO_NUM_21
    };

    Drivers::GpioPwm backlight("Backlight", LEDC_CHANNEL_0, config.ledPin);
    Drivers::SpiBus spiBus("Main", SPI2_HOST, config.sckPin, config.mosiPin, config.misoPin);
    Display::Screen screen(spiBus, config.csPin, config.resetPin, config.dcPin);
    
    backlight.setDutyPercent(100.0f);
    while (true) {
        int x = Utility::RequestInteger("X: ");
        int y = Utility::RequestInteger("Y: ");

        screen.clear();
        screen.render(x, y, App::CrossBitmap);
    }
}
