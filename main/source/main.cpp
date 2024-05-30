// Custom modules
#include "display/ui.hpp"
using namespace kc;

namespace Const
{
    constexpr int ClockPin = 27;
    constexpr int DataPin = 26;
}

extern "C" void app_main()
{
    Display::Ui ui(Const::ClockPin, Const::DataPin);
    ui.start();
}
