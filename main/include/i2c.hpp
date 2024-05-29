#pragma once

// STL modules
#include <vector>

// ESP drivers
#include <driver/i2c.h>

namespace kc {

namespace I2C
{
    class Device
    {
    private:
        i2c_port_t m_port;
        uint8_t m_address;

    public:
        /// @brief Initialize communication with I2C device
        /// @param port I2C port to use
        /// @param address I2C device address
        /// @param sclPin Pin I2C bus clock wire is connected to
        /// @param sdaPin Pin I2C bus data wire is connected to
        Device(i2c_port_t port, uint8_t address, int sclPin, int sdaPin);

        ~Device();

        /// @brief Send data to I2C device
        /// @param data Data to send
        void send(const std::vector<uint8_t>& data);

        /// @brief Receive data from I2C device
        /// @param size Size of data to receive
        /// @return Received data
        std::vector<uint8_t> receive(int size);
    };
}

} // namespace kc
