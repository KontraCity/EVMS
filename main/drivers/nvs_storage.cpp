#include "nvs_storage.hpp"

#include <utility>

#include <nvs_flash.h>

#include <esp_err.h>
#include <esp_log.h>

namespace evms {

static void InitializeNvsFlash() {
    static bool s_initialized = false;
    if (!s_initialized) {
        esp_err_t result = nvs_flash_init();
        if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            result = nvs_flash_init();
        }
        ESP_ERROR_CHECK(result);
        s_initialized = true;
    }
}

Drivers::NvsStorage::NvsStorage(const std::string& namespaceName)
    : m_logTag(namespaceName + " NVSS")
    , m_handle(0) {
    InitializeNvsFlash();
    ESP_ERROR_CHECK(nvs_open(namespaceName.c_str(), NVS_READWRITE, &m_handle));
    ESP_LOGI(m_logTag.c_str(), "Initialized");
}

Drivers::NvsStorage::NvsStorage(NvsStorage&& other) noexcept
    : m_logTag(std::move(other.m_logTag))
    , m_handle(std::exchange(other.m_handle, 0))
{}

Drivers::NvsStorage::~NvsStorage() {
    if (m_handle != 0) {
        save();
        nvs_close(m_handle);
        ESP_LOGI(m_logTag.c_str(), "Deinitialized");
    }
}

Drivers::NvsStorage& Drivers::NvsStorage::operator=(NvsStorage&& other) noexcept {
    if (&other != this) {
        m_logTag = std::move(other.m_logTag);
        m_handle = std::exchange(other.m_handle, 0);
    }
    return *this;
}

void Drivers::NvsStorage::setField(const char* name, const uint8_t* data, size_t length) {
    ESP_ERROR_CHECK(nvs_set_blob(m_handle, name, data, length));
    ESP_LOGI(m_logTag.c_str(), "Field \"%s\" set (%d byte%s)", name, length, length == 1 ? "" : "s");
}

size_t Drivers::NvsStorage::getField(const char* name, uint8_t* buffer, size_t length) {
    esp_err_t result = nvs_get_blob(m_handle, name, buffer, &length);
    if (result == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(m_logTag.c_str(), "Field \"%s\" couldn't be retrieved: not found", name);
        return 0;
    }
    if (result == ESP_ERR_NVS_INVALID_LENGTH) {
        ESP_LOGI(m_logTag.c_str(), "Field \"%s\" couldn't be retrieved: invalid length", name);
        return length;
    }

    ESP_ERROR_CHECK(result);
    ESP_LOGI(m_logTag.c_str(), "Field \"%s\" retrieved (%d byte%s)", name, length, length == 1 ? "" : "s");
    return length;
}

void Drivers::NvsStorage::save() {
    ESP_ERROR_CHECK(nvs_commit(m_handle));
    ESP_LOGI(m_logTag.c_str(), "Saved");
}

} // namespace evms
