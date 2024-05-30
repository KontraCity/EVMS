#include "utility.hpp"

namespace kc {

std::string Utility::RequestString(const char* comment)
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
        if (std::strlen(comment) != 0)
            std::printf("%s ", comment);
        std::string input;

        while (true)
        {
            char character = static_cast<char>(std::getchar());
            std::printf("%c", character);

            if (character == '\n')
                return input;
            input += character;
        }
    }
}

int Utility::RequestInteger(const char* comment)
{
    int result;
    while (true)
    {
        std::string input = RequestString(comment);
        if (std::sscanf(input.c_str(), "%i", &result) == 1)
            return result;
    }
}

float Utility::RequestFloat(const char* comment)
{
    float result;
    while (true)
    {
        std::string input = RequestString(comment);
        if (std::sscanf(input.c_str(), "%f", &result) == 1)
            return result;
    }
}

int Utility::RandomNumber(int min, int max)
{
    static std::mt19937 generator(esp_random());
    return std::uniform_int_distribution(min, max)(generator);
}

float Utility::Constraint(float value, float min, float max)
{
    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    return value;
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
