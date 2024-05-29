#include "i2c.hpp"

namespace kc {

I2C::Device::Device(i2c_port_t port, uint8_t address, int sclPin, int sdaPin)
    : m_port(port)
    , m_address(address)
{
    i2c_config_t config = {};
    config.mode = I2C_MODE_MASTER;
    config.scl_io_num = sclPin;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.sda_io_num = sdaPin;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = 400000;
    i2c_param_config(m_port, &config);
    ESP_ERROR_CHECK(i2c_driver_install(m_port, I2C_MODE_MASTER, 0, 0, 0));
}

I2C::Device::~Device()
{
    i2c_driver_delete(m_port);
}

void I2C::Device::send(const std::vector<uint8_t>& data)
{
    i2c_master_write_to_device(m_port, m_address, data.data(), data.size(), 100);
}

std::vector<uint8_t> I2C::Device::receive(int size)
{
    std::vector<uint8_t> buffer(size);
    i2c_master_read_from_device(m_port, m_address, buffer.data(), size, 100);
    return buffer;
}

} // namespace kc
