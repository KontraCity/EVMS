#pragma once

#include <cstdint>
#include <string>

#include <nvs.h>

namespace evms {

namespace Drivers {
    class NvsStorage {
    private:
        std::string m_logTag;
        nvs_handle_t m_handle;

    public:
        NvsStorage(const std::string& namespaceName);

        NvsStorage(const NvsStorage& other) = delete;

        NvsStorage(NvsStorage&& other) noexcept;

        ~NvsStorage();

    public:
        NvsStorage& operator=(const NvsStorage& other) = delete;

        NvsStorage& operator=(NvsStorage&& other) noexcept;

    public:
        void setField(const char* name, const uint8_t* data, size_t length);

        template <typename FieldType>
        void setField(const char* name, const FieldType& field) {
            setField(name, reinterpret_cast<const uint8_t*>(&field), sizeof(FieldType));
        }

        size_t getField(const char* name, uint8_t* buffer, size_t length);

        template <typename FieldType>
        size_t getField(const char* name, FieldType& field) {
            return getField(name, reinterpret_cast<uint8_t*>(&field), sizeof(FieldType));
        }

        void save();
    };
}

} // namespace evms
