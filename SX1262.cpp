#include "SX1262.h"
#include "pico/stdlib.h"
#include <cstdio>



SX1262::SX1262(spi_inst_t *spi, uint8_t cs_pin, uint8_t busy_pin, uint8_t reset_pin)
{
    this->spi = spi;
    this->cs_pin = cs_pin;
    this->busy_pin = busy_pin;
    this->reset_pin = reset_pin;

    this->apply_default_config();
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



// This is the recommended startup sequence from the documentation: 14.2 Circuit Configuration for Basic Tx Operation
// I've made a few changes, based on trial and error.
void SX1262::apply_default_config()
{
    this->reset();

    // 1. Set to standby mode.
    uint8_t val = 0x00;
    this->send(SX1262::SET_STANDBY, &val, 1);
    sleep_ms(10);

    // 2. Define the protocol (01 = LoRa mode).
    val = 0x01;
    this->send(SX1262::SET_PACKET_TYPE, &val, 1);
    sleep_ms(10);

    // 3. Define the RF frequency.
    this->set_frequency(915000000);
    sleep_ms(10);

    // 4. Define the Power Amplifier configuration.
    // Table 13-21: PA Operating Modes with Optimal Settings
    // Output Power | paDutyCycle |  hpMax  | deviceSel | paLut
    // -------------|-------------|---------|-----------|------
    //     22db     |    0x04     |  0x07   |   0x00    | 0x01
    //     20db     |    0x03     |  0x05   |   0x00    | 0x01
    //     17db     |    0x02     |  0x03   |   0x00    | 0x01
    //     14db     |    0x02     |  0x02   |   0x00    | 0x01
    uint8_t pa_config[4] = {0x03, 0x05, 0x00, 0x01};
    this->send(SX1262::SET_PA_CONFIG, pa_config, 4);
    sleep_ms(10);

    // 5. Define output power and ramping time (20db, 200us ramp time).
    uint8_t tx_params[2] = {0x14, 0x04};
    this->send(SX1262::SET_TX_PARAMS, tx_params, 2);
    sleep_ms(10);

    // 6. Define where the data payload will be stored.
    uint8_t buffer_base[2] = {0x00, 0x00};
    this->send(SX1262::SET_BUFFER_BASE_ADDRESS, buffer_base, 2);
    sleep_ms(10);

    // 7. Send the payload to the data buffer
    // This is done each time in send_packet(), so no need here.

    // 8. Define the modulation parameter.
    // Spreading factor, bandwidth, coding rate, Low Data Rate Optimization,,,,
    // These are set to the default values that arduino LoRa uses.
    // sf7, 125kHz, LORA_CR_4_8, off
    uint8_t mod_params[8] = {0x07, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    this->send(SX1262::SET_MODULATION_PARAMS, mod_params, 8);
    sleep_ms(10);

    // 9. Define the frame format.
    // These are set to the arduino lora defaults
    // preamble length (2 bytes), header type, payload_length, CRC type, invert IQ, padding...
    // 8 bytes preamble, explicit header, 128 byte payload (no effect in explicit mode), CRC off, Standard IQ
    uint8_t params[9] =  {0x00, 0x08, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
    this->send(SX1262::SET_PACKET_PARAMS, params, 9);
    sleep_ms(10);

    // 10. Configure DIO and IRQ.
    // First two bytes are IRQ mask, next to are the DIO mask
    // 00000010 00000011 = 0x0203
    uint8_t irq_params[8] = {0x02, 0x03, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00};
    this->send(SX1262::SET_DIO_IRQ_PARAMS, irq_params, 8);
    sleep_ms(10);

    // 11. Define Sync Word value.
    // // Two byte register location, then the private network sync word.
    uint8_t sync_word[4] = {0x07, 0x40, 0x14, 0x24};
    // // Public
    // //uint8_t sync_word[4] = {0x07, 0x40, 0x34, 0x44};
    this->send(SX1262::WRITE_REGISTER, sync_word, 4);

    // Not in the application seqence from the documentation, but necessary to get the Waveshare LoRa hat working.
    // First byte is the voltage (1.8), the next three are the delay.
    uint8_t tcxo_config[4] = {0x02, 0x00, 0x32, 0x00};
    this->send(SET_DIO3_AS_TCXO_CTRL, tcxo_config, 4);
}



void SX1262::clear_errors()
{
    uint8_t buff[2] = {0x00, 0x00};
    this->send(SX1262::CLEAR_DEVICE_ERRORS, buff, 2);
}



void SX1262::print_errors()
{
    uint8_t cmd[2] = {SX1262::GET_DEVICE_ERRORS, 0x00};
    uint8_t error_buffer[2] = {0};

    this->wait_until_ready();
    gpio_put(this->cs_pin, 0);
    spi_write_blocking(this->spi, cmd, 2);
    spi_read_blocking(this->spi, 0x00, error_buffer, 2);
    gpio_put(this->cs_pin, 1);
    this->wait_until_ready();

    // Convert the returned bytes to a 16 bit value.
    uint16_t error = error_buffer[0];
    error << 8;
    error = error | error_buffer[1];

    printf("Error value: 0x%04X\n", error);

    // Check each bit.
    if (error & (1 << 0))
    {
        printf(" RC64K_CALIB_ERR\n");
    }

    if (error & (1 << 1))
    {
        printf(" RC13M_CALIB_ERR\n");
    }

    if (error & (1 << 2))
    {
        printf(" PLL_CALIB_ERR\n");
    }

    if (error & (1 << 3))
    {
        printf(" ADC_CALIB_ERR\n");
    }

    if (error & (1 << 4))
    {
        printf(" IMG_CALIB_ERR\n");
    }

    if (error & (1 << 5))
    {
        printf(" XOSC_START_ERR\n");
    }

    if (error & (1 << 6))
    {
        printf(" PLL_LOCK_ERR\n");
    }

    if (error & (1 << 8))
    {
        printf(" -PA_RAMP_ERR\n");
    }
}



// TODO: I'm not sure if my implementation here is off. My SX1262 often prints 'Reserved' for the `command_status` field.
void SX1262::print_status()
{
    uint8_t cmd = SX1262::GET_STATUS;
    uint8_t status;

    this->wait_until_ready();
    gpio_put(this->cs_pin, 0);
    spi_write_blocking(this->spi, &cmd, 1);
    spi_read_blocking(this->spi, 0x00, &status, 1);
    gpio_put(this->cs_pin, 1);
    this->wait_until_ready();

    uint8_t chip_mode  = (status >> 4) & 0x07;
    uint8_t command_status = (status >> 1) & 0x07;

    const char* mode_name;
    switch (chip_mode)
    {
        case 0: mode_name = "Unused"; break;
        case 1: mode_name = "Reserved"; break;
        case 2: mode_name = "STBY_RC"; break;
        case 3: mode_name = "STBY_XOSC"; break;
        case 4: mode_name = "FX"; break;
        case 5: mode_name = "RS"; break;
        case 6: mode_name = "TX"; break;
        default: mode_name = "Unknown";
    }

    const char* status_name;
    switch (command_status)
    {
        case 0: status_name = "Reserved"; break;
        case 1: status_name = "Reserved"; break;
        case 2: status_name = "Data is available to host"; break;
        case 3: status_name = "Command timeout"; break;
        case 4: status_name = "Command processing error"; break;
        case 5: status_name = "Failure to execute command"; break;
        case 6: status_name = "Command TX done"; break;
        default: status_name = "Unknown";
    }

    printf("SX1262 status: 0x%02X\n", status);
    printf("  chip mode:  %u (%s)\n", chip_mode, mode_name);
    printf("  command:    %u (%s)\n", command_status, status_name);
}
