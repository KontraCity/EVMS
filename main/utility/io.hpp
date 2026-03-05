#pragma once

#include <cstdio>
#include <cstring>
#include <string>

#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"

#include "driver/uart.h"

namespace evms {

namespace Utility {
    inline void Utility::InitializeIO() {
        static bool initialized = false;
        if (!initialized) {
            setvbuf(stdin, NULL, _IONBF, 0);
            setvbuf(stdout, NULL, _IONBF, 0);
            ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
            esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
            esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
            esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);
            initialized = true;
        }
    }

    inline std::string Utility::RequestString(const char* comment) {
        Utility::InitializeIO();
        while (true) {
            if (std::strlen(comment) != 0)
                std::printf("%s ", comment);
            std::string input;

            while (true) {
                char character = static_cast<char>(std::getchar());
                std::printf("%c", character);

                if (character == '\n')
                    return input;
                input += character;
            }
        }
    }

    inline int Utility::RequestInteger(const char* comment) {
        int result;
        while (true) {
            std::string input = RequestString(comment);
            if (std::sscanf(input.c_str(), "%i", &result) == 1)
                return result;
        }
    }

    inline float Utility::RequestFloat(const char* comment) {
        float result;
        while (true) {
            std::string input = RequestString(comment);
            if (std::sscanf(input.c_str(), "%f", &result) == 1)
                return result;
        }
    }
}

} // namespace evms
