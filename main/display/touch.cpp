#include "touch.hpp"

#include <cmath>
#include <utility>

#include "display/screen.hpp"
#include "utility/math.hpp"

namespace evms {

Display::Touch::Touch(const Drivers::SpiBus& spiBus, gpio_num_t csPin)
    : SpiDevice(spiBus.newDevice("XPT2046", csPin, 2'000'000, true))
{}

Display::Touch::Touch(Touch&& other) noexcept
    : SpiDevice(std::move(other))
    , m_calibration(std::exchange(other.m_calibration, {}))
{}

Display::Touch& Display::Touch::operator=(Touch&& other) noexcept {
    if (&other != this) {
        SpiDevice::operator=(std::move(other));
        m_calibration = std::exchange(other.m_calibration, {});
    }
    return *this;
}

int Display::Touch::getValue(uint8_t valueCode) const {
    std::vector<uint8_t> response = transfer({ valueCode, 0x00, 0x00 }, 3);
    return ((response[1] << 8) | response[2]) >> 3;
}

void Display::Touch::setCalibration(const Calibration& calibration) {
    m_calibration = calibration;
}

Display::Position Display::Touch::getTouchPosition() const {
    constexpr int EdgeLimit = 50;
    constexpr int NoiseLimit = 20;

    for (int attempt = 0; attempt < 5; ++attempt) {
        int x1 = getValue(0b1'101'00'00);
        int y1 = getValue(0b1'001'00'00);
        if (x1 < EdgeLimit || x1 > 4095 - EdgeLimit || y1 < EdgeLimit || y1 > 4095 - EdgeLimit)
            continue; // floating, retry read

        int x2 = getValue(0b1'101'00'00);
        int y2 = getValue(0b1'001'00'00);
        if (std::abs(x1 - x2) > NoiseLimit || std::abs(y1 - y2) > NoiseLimit)
            continue; // noisy, retry read
            
        // Average the two readings
        Position position = { (x1 + x2) / 2, (y1 + y2) / 2 };
        if (!m_calibration.calibrated)
            return position;

        // Apply calibration
        position.x = Utility::Constraint(Utility::ConvertRange(
            position.x,
            m_calibration.touchLeftLineX, m_calibration.touchRightLineX,
            m_calibration.screenLeftLineX, m_calibration.screenRightLineX
        ), 0, Screen::ScreenWidth);
        position.y = Utility::Constraint(Utility::ConvertRange(
            position.y,
            m_calibration.touchTopLineY, m_calibration.touchBottomLineY,
            m_calibration.screenTopLineY, m_calibration.screenBottomLineY
        ), 0, Screen::ScreenHeight);
        return position;
    }
    
    // All attempts are noisy, not touched?
    return { -1, -1 };
}

int Display::Touch::getTouchPressure() const {
    int sum = 0;
    for (int sample = 0; sample < 5; ++sample) {
        int z1 = getValue(0b1'011'00'00);
        int z2 = getValue(0b1'100'00'00);
        if (z1 == 0 || z2 <= z1)
            return 0;

        int x = getValue(0b1'101'00'00);
        sum += x * (z2 - z1) / z1;
    }
    return sum / 5;
}

float Display::Touch::getControllerTemp() const {
    int sum = 0;
    for (int sample = 0; sample < 5; ++sample) {
        int t0 = getValue(0b1'000'01'00);
        int t1 = getValue(0b1'111'01'00);
        sum += (t1 - t0);
    }
    return (sum / 5) * 0.125f;
}

} // namespace evms
