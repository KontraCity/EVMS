#include "display/ui.hpp"

namespace kc {

Display::BitMap Display::Ui::SlidingValueWidget(float value)
{
    int wholeValue = static_cast<int>(value);
    int digitCount = (wholeValue > 0 ? std::log10(std::ceil(value)) + 1: 1);
    int offset = (value - wholeValue) * Font::ValueCharacter::Height;
    BitMap map(Font::ValueCharacter::Height, std::vector<bool>(digitCount * Font::ValueCharacter::Width));
    for (int digitIndex = digitCount - 1; digitIndex >= 0; --digitIndex, wholeValue /= 10)
    {
        int digit = wholeValue % 10;
        Font::ValueCharacter digitMap(static_cast<int>(value) != 0 && digitIndex == 0 && digit == 0 ? ' ' : '0' + digit);
        if (!offset)
        {
            for (int y = 0, height = Font::ValueCharacter::Height; y < height; ++y)
                for (int x = 0, width = Font::ValueCharacter::Width; x < width; ++x)
                    map[y][x + Font::ValueCharacter::Width * digitIndex] = digitMap[y][width - x];
            continue;
        }

        Font::ValueCharacter nextDigitMap('0' + (digit + 1) % 10);
        for (int y = 0, height = Font::ValueCharacter::Height; y < height; ++y)
        {
            for (int x = 0, width = Font::ValueCharacter::Width; x < width; ++x)
            {
                if (y >= offset)
                    map[y][x + Font::ValueCharacter::Width * digitIndex] = digitMap[y - offset][width - x];
                else
                    map[y][x + Font::ValueCharacter::Width * digitIndex] = nextDigitMap[Font::ValueCharacter::Height - 1 - (offset - y)][width - x];
            }
        }

        if (digit != 9)
            offset = 0;
    }
    return map;
}

Display::Ui::Ui(int sclPin, int sdaPin)
    : Master(sclPin, sdaPin)
{
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);
}

void Display::Ui::start()
{
    print<Font::TextCharacter>(0, 0, "Oil temp     1/1");
    print<Font::TextCharacter>(0, 16, "\370C");

    float value = 0;
    while (true)
    {
        clear(24, 16, 128 - 24, 64 - 16);
        draw(127, 22, SlidingValueWidget(value), true);
        render();
        
        int32_t buffer = 0;
        for (int offset = 8 * (sizeof(buffer) - 1); offset >= 0; offset -= 8)
            buffer |= std::getchar() << offset;
        value = *reinterpret_cast<float*>(&buffer);
    }
}

} // namespace kc
