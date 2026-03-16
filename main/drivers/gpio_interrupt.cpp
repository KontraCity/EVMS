#include "gpio_interrupt.hpp"

#include <utility>

#include <esp_log.h>

#include "utility/time.hpp"

namespace evms {

static std::string MakeLogTag(const std::string& logName, gpio_num_t pin) {
    std::string pinStr = std::to_string(static_cast<int>(pin));
    return logName + " GpioInterrupt [" + pinStr + "]";
}

static void InitializeIsrService() {
    static bool s_initialized = false;
    if (!s_initialized) {
        ESP_ERROR_CHECK(gpio_install_isr_service(0));
        s_initialized = true;
    }
}

void IRAM_ATTR Drivers::GpioInterrupt::IsrHandler(void* argument) {
    GpioInterrupt* root = reinterpret_cast<GpioInterrupt*>(argument);
    Event event = gpio_get_level(root->m_pin) ? Event::Rising : Event::Falling;
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    xQueueOverwriteFromISR(root->m_queue, &event, &higherPriorityTaskWoken);
    if (higherPriorityTaskWoken)
        portYIELD_FROM_ISR();
}

void Drivers::GpioInterrupt::CallbackTask(void* argument) {
    GpioInterrupt* root = reinterpret_cast<GpioInterrupt*>(argument);
    while (true) {
        Event event;
        if (!xQueueReceive(root->m_queue, &event, portMAX_DELAY))
            continue;

        const std::vector<Callback>* callbacks = nullptr;
        if (event == Event::Rising)
            callbacks = &root->m_risingCallbacks;
        else
            callbacks = &root->m_fallingCallbacks;

        for (const Callback& callback : *callbacks)
            callback();
    }
}

Drivers::GpioInterrupt::GpioInterrupt(const char* logName, gpio_num_t pin)
    : m_logTag(MakeLogTag(logName, pin))
    , m_pin(pin)
    , m_queue(xQueueCreate(1, sizeof(Event))) 
    , m_task(nullptr) {
    gpio_config_t gpioConfig = {};
    gpioConfig.pin_bit_mask = 1ULL << m_pin;
    gpioConfig.mode = GPIO_MODE_INPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));

    InitializeIsrService();
    ESP_ERROR_CHECK(gpio_isr_handler_add(m_pin, IsrHandler, this));
    ESP_LOGI(m_logTag.c_str(), "Initialized");
}

Drivers::GpioInterrupt::GpioInterrupt(GpioInterrupt&& other) noexcept
    : m_logTag(std::move(other.m_logTag))
    , m_pin(std::exchange(other.m_pin, GPIO_NUM_MAX))
    , m_queue(std::exchange(other.m_queue, nullptr))
    , m_task(std::exchange(other.m_task, nullptr))
    , m_risingCallbacks(std::move(other.m_risingCallbacks))
    , m_fallingCallbacks(std::move(other.m_fallingCallbacks))
{}

Drivers::GpioInterrupt::~GpioInterrupt() {
    bool deinitialized = false;
    if (m_task) {
        vTaskDelete(m_task);
        deinitialized = true;
    }
    if (m_queue) {
        vQueueDelete(m_queue);
        deinitialized = true;
    }
    if (m_pin != GPIO_NUM_MAX) {
        gpio_isr_handler_remove(m_pin);
        gpio_reset_pin(m_pin);
        deinitialized = true;
    }

    if (deinitialized)
        ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

Drivers::GpioInterrupt& Drivers::GpioInterrupt::operator=(GpioInterrupt&& other) noexcept {
    if (&other != this) {
        m_logTag = std::move(other.m_logTag);
        m_pin = std::exchange(other.m_pin, GPIO_NUM_MAX);
        m_queue = std::exchange(other.m_queue, nullptr);
        m_task = std::exchange(other.m_task, nullptr);
        m_risingCallbacks = std::move(other.m_risingCallbacks);
        m_fallingCallbacks = std::move(other.m_fallingCallbacks);
    }
    return *this;
}

void Drivers::GpioInterrupt::startCallbackTask() {
    if (!m_task)
        xTaskCreate(CallbackTask, m_logTag.c_str(), 2048, this, 10, &m_task);
}

void Drivers::GpioInterrupt::stopCallbackTask() {
    if (m_task) {
        vTaskDelete(m_task);
        m_task = nullptr;
    }
}

void Drivers::GpioInterrupt::setCallbacks(const CallbackList& risingCallbacks, const CallbackList& fallingCallbacks) {
    stopCallbackTask();
    m_risingCallbacks = risingCallbacks;
    m_fallingCallbacks = fallingCallbacks;
    startCallbackTask();
}

} // namespace evms
