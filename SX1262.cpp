#include "SX1262.h"
#include "pico/stdlib.h"



SX1262::SX1262(spi_inst_t *spi, uint8_t cs_pin, uint8_t busy_pin, uint8_t reset_pin)
{
    this->spi = spi;
    this->cs_pin = cs_pin;
    this->busy_pin = busy_pin;
    this->reset_pin = reset_pin;
}



void SX1262::wait_until_ready()
{
    while (gpio_get(this->busy_pin))
    {
        sleep_us(10);
    }
}



void SX1262::reset()
{
    gpio_put(this->reset_pin, 0);
    sleep_ms(50);
    gpio_put(this->reset_pin, 1);
    sleep_ms(50);
    this->wait_until_ready();
}



void SX1262::send(uint8_t command, const uint8_t *data, size_t length)
{
    this->wait_until_ready();
    gpio_put(this->cs_pin, 0);

    spi_write_blocking(this->spi, &command, 1);

    if (length > 0)
    {
        spi_write_blocking(this->spi, data, length);
    }

    gpio_put(this->cs_pin, 1);
    this->wait_until_ready();
}



void SX1262::set_TX()
{
    uint8_t timeout[3] = {0, 0, 0};
    this->send(SX1262::SET_TX, timeout, 3);
}



void SX1262::send_packet(uint8_t *packet, uint8_t packet_length)
{
    uint8_t buff[packet_length + 1];
    // Module buffer offset.
    buff[0] = 0x00;
    for (int i = 0; i < packet_length; i++)
    {
        buff[1+i] = packet[i];
    }

    this->send(SX1262::WRITE_BUFFER, buff, packet_length + 1);
    this->set_TX();
}



void SX1262::set_frequency(uint32_t frequency)
{
    // See the documentation for this calculation.
    uint64_t target = frequency;
    target *= 33554432;
    target /= 32000000;
    uint32_t short_target = target;

    uint8_t buffer[4];

    buffer[0] = short_target >> 24;
    buffer[1] = short_target >> 16;
    buffer[2] = short_target >> 8;
    buffer[3] = short_target;

    this->send(SX1262::SET_RF_FREQUENCY, buffer, 4);
}
