#include "utility.hpp"

namespace kc {

int Utility::RequestNumber(const char* comment)
{
    static bool initialized = false;
    if (!initialized)
    {
        setvbuf(stdin, NULL, _IONBF, 0);
        setvbuf(stdout, NULL, _IONBF, 0);
        ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
        esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
        esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
        esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);
        initialized = true;
    }

    while (true)
    {
        printf("%s ", comment);
        std::string input;

        do
        {
            char character = static_cast<char>(getchar());
            input += character;
            printf("%c", character);
        }
        while (input[input.size() - 1] != '\n');

        int result, converted = std::sscanf(input.c_str(), "%i", &result);
        if (converted == 1)
            return result;
    }
}

int Utility::RandomNumber(int min, int max)
{
    static std::mt19937 generator(esp_random());
    return std::uniform_int_distribution(min, max)(generator);
}

void Utility::Sleep(double seconds)
{
    if (seconds <= 0)
        return;

    /*
    *   One FreeRTOS tick is 10ms, so 1 second is 100 ticks.
    *   Delays shorter than 1 tick should just burn CPU cycles.
    */
    if (seconds < 0.01)
        ets_delay_us(seconds * 1'000'000);
    vTaskDelay(seconds * 100);
}

} // namespace kc
