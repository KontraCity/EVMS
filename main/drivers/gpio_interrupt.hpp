#pragma once

#include <string>
#include <vector>
#include <functional>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <driver/gpio.h>

namespace evms {

namespace Drivers {
    class GpioInterrupt {
    public:
        enum class Event {
            Rising,
            Falling,
        };
        
        using Callback = std::function<void()>;
        using CallbackList = std::vector<Callback>;

    private:
        static void IRAM_ATTR IsrHandler(void* argument);

        static void CallbackTask(void* argument);

    private:
        std::string m_logTag;
        gpio_num_t m_pin;
        QueueHandle_t m_queue;
        TaskHandle_t m_task;

        std::vector<Callback> m_risingCallbacks;
        std::vector<Callback> m_fallingCallbacks;

    public:
        GpioInterrupt(const char* logName, gpio_num_t pin);

        GpioInterrupt(const GpioInterrupt& other) = delete;

        GpioInterrupt(GpioInterrupt&& other) noexcept;

        ~GpioInterrupt();

    public:
        GpioInterrupt& operator=(const GpioInterrupt& other) = delete;

        GpioInterrupt& operator=(GpioInterrupt&& other) noexcept;

    private:
        void startCallbackTask();

        void stopCallbackTask();

    public:
        void setCallbacks(const CallbackList& risingCallbacks, const CallbackList& fallingCallbacks);
    };
}

} // namespace evms
