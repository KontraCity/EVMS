#include "display/ui.hpp"

namespace kc {

Display::Ui::Ui(int sclPin, int sdaPin)
    : Master(sclPin, sdaPin)
{}

void Display::Ui::start()
{
    print<Font::TextCharacter>(0, 0, "Engine temp. 1/1");
    render();

    while (true)
    {

        float value = Utility::Constraint(Utility::RequestFloat(), -99.9, 999.9);
        char buffer[256];
        sprintf(buffer, "%.1f", value);

        clear(0, 16, 128, 64 - 16);
        if (value < 100)
            print<Font::TextCharacter>(0, 16, "DEG");
        print<Font::ValueCharacter>(127, 22, buffer, true);
        render();
    }
}

} // namespace kc
