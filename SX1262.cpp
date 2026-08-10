#include "SX1262.h"
#include "pico/stdlib.h"



SX1262::SX1262(spi_inst_t *spi, uint8_t cs_pin, uint8_t busy_pin, uint8_t reset_pin)
{
    this->spi = spi;
    this->cs_pin = cs_pin;
    this->busy_pin = busy_pin;
    this->reset_pin = reset_pin;
}
