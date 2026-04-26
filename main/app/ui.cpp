#include "app/ui.hpp"

#include <stdio.h>

#include <cstdint>
#include <vector>
#include <algorithm>
#include <array>
#include <utility>

#include "app/bitmaps.hpp"
#include "drivers/nvs_storage.hpp"
#include "utility/io.hpp"
#include "utility/time.hpp"

#include <esp_log.h>

namespace evms {

static void PrintMessage(const char* comment, const Drivers::CanBus::Message& message) {
    printf("%s ID: %03X | ", comment, static_cast<unsigned int>(message.id));
    for (int index = 0; index < message.data.size(); index++)
        printf("%02X ", message.data[index]);
    printf("\n");
}

static Drivers::CanBus::Message ExpectMessage(const Drivers::CanBus& canBus, uint32_t id) {
    while (true) {
        Drivers::CanBus::Message message = canBus.receive();
        //if (message.id != 0x130)
            //PrintMessage("<-", message);
        if (message.id == id)
            return message;
    }
}

static std::vector<uint8_t> Service01Request(const Drivers::CanBus& canBus, const std::vector<uint8_t>& pids) {
    std::vector<uint8_t> result;
    int bytesWritten = 0;
    
    const int pidsCount = std::min(7, static_cast<int>(pids.size())); // 7 PIDs at max for now.
    Drivers::CanBus::Message request = {
        .id = 0x7DF,
        .data = { static_cast<uint8_t>(pidsCount + 1), 0x01 }
    };
    std::copy(pids.data(), pids.data() + pidsCount, request.data.data() + 2);

    canBus.send(request);
    //PrintMessage("->", request);
    while (true) {
        Drivers::CanBus::Message response = ExpectMessage(canBus, 0x7E8);

        // Response is a first frame of multi-frame response?
        if (response.data[0] == 0x10) {
            result.resize(response.data[1]);
            std::copy(
                response.data.data() + 2,
                response.data.data() + response.data.size(),
                result.data()
            );
            bytesWritten = response.data.size() - 2;
            
            // Tell the car we're ready for next frames
            request = { 0x7E0, { 0x30 } };
            canBus.send(request);
            //PrintMessage("->", request);
            continue;
        }

        // Response is a next frame of multi-frame response?
        if (response.data[0] >= 0x21) {
            int bytesToWrite = std::min(7, static_cast<int>(result.size()) - bytesWritten);
            std::copy(
                response.data.data() + 1,
                response.data.data() + 1 + bytesToWrite,
                result.data() + bytesWritten
            );
            bytesWritten += bytesToWrite;

            if (bytesWritten == result.size())
                return result;
            continue;
        }

        // Response is fully contained in current frame
        result.resize(response.data[0]);
        std::copy(
            response.data.data() + 1,
            response.data.data() + 1 + response.data[0],
            result.data()
        );
        return result;
    }
}

App::Ui::Ui(const Config& config)
    : m_logTag("App")
    , m_canBus("Car", config.txPin, config.rxPin)
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
    , m_canBus(std::move(other.m_canBus))
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
        m_canBus = std::move(other.m_canBus);
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
        auto GetTouchMessageBitmap = [this](int index) -> const TouchMessageBitmapType& {
            if (index == 1)
                return FirstTouchMessageBitmap;
            if (index == 2)
                return SecondTouchMessageBitmap;
            if (index == 3)
                return ThirdTouchMessageBitmap;
            return FourthTouchMessageBitmap;
        };

        m_screen.clear();
        m_screen.draw(77, 94, CalibrationMessageBitmap);
        m_screen.draw(77, 114, PleaseTouchAsInstructedMessageBitmap);
        m_screen.draw(123, 132, GetTouchMessageBitmap(index));
        m_screen.draw(x - CrossBitmap.width() / 2, y - CrossBitmap.height() / 2, CrossBitmap);
        m_screen.render();

        static float s_prevReleaseTime = -1.0f;
        if (index == 1)
            s_prevReleaseTime = -1.0f;

        Display::Position position = { -1, -1 };
        while (true) {
            position = m_touch.getTouchPosition();
            if (position && Utility::TimeSeconds() - s_prevReleaseTime >= 0.5f)
                break;
            Utility::Sleep(0.1f);
        }

        m_screen.clear(x - CrossBitmap.width() / 2, y - CrossBitmap.height() / 2, CrossBitmap.size());
        m_screen.clear(77, 114, PleaseTouchAsInstructedMessageBitmap.size());
        m_screen.draw(81, 114, RecordedPleaseReleaseMessageBitmap);
        m_screen.render();

        while (m_touch.getTouchPosition())
            Utility::Sleep(0.1f);
        s_prevReleaseTime = Utility::TimeSeconds();
        
        if (index == 4) {
            m_screen.clear(81, 114, RecordedPleaseReleaseMessageBitmap.size());
            m_screen.draw(127, 114, ThankYouMessageBitmap);
            m_screen.render();
            Utility::Sleep(2.0f);
        }
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

App::Ui::Packet App::Ui::readPacket() {
    auto data = Service01Request(m_canBus, { 0x0B, 0x04, 0x0F, 0x42, 0x05 });

    Packet packet = {};
    packet.manifoldAbsolutePress = data[2] / 100.0f;
    packet.engineLoad = data[4] / 2.55f;
    packet.intakeAirTemp = data[6] - 40.0f;
    packet.batteryVoltage = (256.0f * data[8] + data[9]) / 1000.0f;
    packet.coolantTemp = data[11] - 40.0f;
    return packet;
}

void App::Ui::mainloop() {
    m_backlight.setDutyPercent(100.0f);
    Utility::Sleep(9999.9f);
}

} // namespace evms
