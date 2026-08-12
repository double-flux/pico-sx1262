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
