#include "app/ui.hpp"
using namespace evms;

/*
*   Connection to the 2.4" TFT display:
*   Screen      ESP32
*   VCC         3.3
*   GND         GND
*   CS           15
*   RESET         4
*   DC            2
*   MOSI         23
*   SCK          18
*   LED          22
*   MISO         19
*   T_CS         21
*/

extern "C" void app_main() {
    App::Ui ui({
        .csPin    = GPIO_NUM_15,
        .resetPin = GPIO_NUM_4,
        .dcPin    = GPIO_NUM_2,
        .mosiPin  = GPIO_NUM_23,
        .sckPin   = GPIO_NUM_18,
        .ledPin   = GPIO_NUM_22,
        .misoPin  = GPIO_NUM_19,
        .tcsPin   = GPIO_NUM_21
    });
    ui.mainloop();
}
