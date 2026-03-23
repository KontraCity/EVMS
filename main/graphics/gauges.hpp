#pragma once

#include "display/screen.hpp"

namespace evms {

namespace Graphics {
    void DrawLabels(Display::Screen& screen);

    void DrawManifoldAbsolutePressGauge(Display::Screen& screen, float pressure);

    void DrawFuelFlowGauge(Display::Screen& screen, float fuelFlow);

    void DrawOilPressGauge(Display::Screen& screen, float pressure);

    void DrawGearText(Display::Screen& screen, int gear);

    void DrawEngineLoadText(Display::Screen& screen, float engineLoad);

    void DrawIntakeAirTempText(Display::Screen& screen, float intakeAirTemp);

    void DrawBatteryVoltageText(Display::Screen& screen, float batteryVoltage);

    void DrawCoolantTempGauge(Display::Screen& screen, float temp);
    
    void DrawOilTempGauge(Display::Screen& screen, float temp);
    
    void DrawGearboxTempGauge(Display::Screen& screen, float temp);
}

} // namespace evms
