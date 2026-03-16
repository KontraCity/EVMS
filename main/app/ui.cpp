#include "app/ui.hpp"

#include <utility>

#include "app/bitmaps.hpp"
#include "drivers/nvs_storage.hpp"
#include "utility/math.hpp"
#include "utility/time.hpp"

#include <esp_log.h>

namespace evms {

static std::string MakeLogTag() {
    return "App";
}

App::Ui::Ui(const Config& config)
    : m_logTag(MakeLogTag())
    , m_spiBus("Main", SPI2_HOST, config.sckPin, config.mosiPin, config.misoPin)
    , m_backlight("Backlight", LEDC_CHANNEL_0, config.ledPin)
    , m_screen(m_spiBus, config.csPin, config.resetPin, config.dcPin)
    , m_touch(m_spiBus, config.tcsPin) {
    if (!applyTouchCalibration(false))
        calibrateTouch();
    ESP_LOGI(m_logTag.c_str(), "Initialized");
}

App::Ui::Ui(Ui&& other) noexcept
    : m_logTag(std::move(other.m_logTag))
    , m_spiBus(std::move(other.m_spiBus))
    , m_backlight(std::move(other.m_backlight))
    , m_screen(std::move(other.m_screen))
    , m_touch(std::move(other.m_touch))
{}

App::Ui::~Ui() {
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

App::Ui& App::Ui::operator=(Ui&& other) noexcept {
    if (&other != this) {
        m_logTag = std::move(other.m_logTag);
        m_spiBus = std::move(other.m_spiBus);
        m_backlight = std::move(other.m_backlight);
        m_screen = std::move(other.m_screen);
        m_touch = std::move(other.m_touch);
    }
    return *this;
}

void App::Ui::calibrateTouch() {
    constexpr int Offset = 25; // for touch positions from the edges for ease of touch
    constexpr Display::Position TopLeftPos = { Offset, Offset };
    constexpr Display::Position TopRightPos = { Screen::ScreenWidth - Offset, Offset };
    constexpr Display::Position BottomRightPos = { Screen::ScreenWidth - Offset, Screen::ScreenHeight - Offset };
    constexpr Display::Position BottomLeftPos = { Offset, Screen::ScreenHeight - Offset };

    auto ShowCrossAndGetPos = [this](int index, int x, int y) -> Display::Position {
        m_screen.clear();
        std::vector<std::string> messages = {
            "CALIBRATION",
            "TOUCH PRECISELY",
            std::to_string(index) + "/4 PRESSES"
        };
        printCenteredText(messages);

        const auto& CrossBitmap = evms::App::CrossBitmap;
        m_screen.draw(
            x - CrossBitmap.MapWidth / 2,
            y - CrossBitmap.MapHeight / 2,
            CrossBitmap
        );
        m_screen.render();

        Display::Position position = { -1, -1 };
        while (!(position = m_touch.getTouchPosition()))
            Utility::Sleep(0.1f);

        m_screen.clear();
        messages[1] = "RELEASE";
        printCenteredText(messages);
        m_screen.render();

        while (m_touch.getTouchPosition())
            Utility::Sleep(0.1f);
        return position;
    };

    m_screen.clear();
    m_screen.render();
    m_backlight.setDutyPercent(100.0f);
    ESP_LOGI(m_logTag.c_str(), "Calibrating touch");

    Display::Position topLeftTouchPos = ShowCrossAndGetPos(1, TopLeftPos.x, TopLeftPos.y);
    Display::Position topRightTouchPos = ShowCrossAndGetPos(2, TopRightPos.x, TopRightPos.y);
    Display::Position bottomRightTouchPos = ShowCrossAndGetPos(3, BottomRightPos.x, BottomRightPos.y);
    Display::Position bottomLeftTouchPos = ShowCrossAndGetPos(4, BottomLeftPos.x, BottomLeftPos.y);

    m_screen.clear();
    m_screen.render();
    m_backlight.setDutyPercent(0.0f);

    applyTouchCalibration(true, {
        .calibrated = true,
        .touchLeftLineX = (topLeftTouchPos.x + bottomLeftTouchPos.x) / 2,
        .touchRightLineX = (topRightTouchPos.x + bottomRightTouchPos.x) / 2,
        .touchTopLineY = (topLeftTouchPos.y + topRightTouchPos.y) / 2,
        .touchBottomLineY = (bottomLeftTouchPos.y + bottomRightTouchPos.y) / 2,
        .screenLeftLineX = TopLeftPos.x,
        .screenRightLineX = TopRightPos.x,
        .screenTopLineY = TopLeftPos.y,
        .screenBottomLineY = BottomLeftPos.y
    });
}

bool App::Ui::applyTouchCalibration(bool overwrite, Display::Touch::Calibration calibration) {
    constexpr const char* StorageNamespace = "Touch";
    constexpr const char* CalibrationFieldName = "calibration";

    Drivers::NvsStorage storage(StorageNamespace);
    bool success = false;
    if (overwrite) {
        storage.setField(CalibrationFieldName, calibration);
        success = true;
    }
    else {
        size_t bytesRead = storage.getField(CalibrationFieldName, calibration);
        success = bytesRead != 0;
    }

    if (success) {
        m_touch.setCalibration(calibration);
        ESP_LOGI(m_logTag.c_str(), "Touch calibration applied");
    }
    return success;
}

Display::Size App::Ui::getTextSize(const std::string& text) {
    Display::Size size = Glyph::MapSize;
    size.width *= text.length();
    return size;
}

void App::Ui::printText(int x, int y, const std::string& text) {
    for (char character : text) {
        m_screen.draw(x, y, Fonts::GetEcamFontGlyph(character));
        x += Glyph::MapWidth;
    }
}

void App::Ui::printCenteredText(const std::vector<std::string>& lines) {
    constexpr int Spacing = 5;
    int y = (Screen::ScreenHeight - (Glyph::MapHeight + Spacing) * lines.size()) / 2;
    for (const std::string& line : lines) {
        int x = (Screen::ScreenWidth - getTextSize(line).width) / 2;
        printText(x, y, line);
        y += Glyph::MapHeight + Spacing;
    }
}

void App::Ui::mainloop() {
    m_backlight.setDutyPercent(100.0f);
    while (true) {
        Display::Position position = m_touch.getTouchPosition();
        if (!position) {
            Utility::Sleep(0.01f);
            continue;
        }

        m_screen.draw(position.x - 1, position.y - 1, DotBitmap);
        m_screen.render();
        Utility::Sleep(0.01f);
    }
}

} // namespace evms
