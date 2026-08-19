#include "hardware/spi.h"



class SX1262
{
public:
    SX1262(spi_inst_t *spi, uint8_t cs_pin, uint8_t busy_pin, uint8_t reset_pin);

    void send_packet(uint8_t *packet, uint8_t packet_length);

    // Lower level functions.
    // ----------------------
    // Sleeps until the SX1262 module is ready.
    void wait_until_ready();
    // Reset the SX1262 module.
    void reset();
    // Send a command and any associated data to the module.
    void send(uint8_t command, const uint8_t *data, size_t length);
    // Sends the TX command, transmitting data in the modules buffer.
    void set_TX();

    void set_frequency(uint32_t frequency);

    // Taken from DS_SX1261-2_V1.2.pdf
    // Operational Modes Commands
    static const uint8_t SET_SLEEP = 0x84;
    static const uint8_t SET_STANDBY = 0x80;
    static const uint8_t SET_FS = 0xc1;
    static const uint8_t SET_TX = 0x83;
    static const uint8_t STET_RX = 0x82;
    static const uint8_t STOP_TIMER_ON_PREAMBLE = 0x9f;
    static const uint8_t SER_RX_DUTY_CYCLE = 0x94;
    static const uint8_t SET_CAD = 0xc5;
    static const uint8_t SET_TX_CONTINUOUS_WAVE = 0xd1;
    static const uint8_t SET_TX_INFINITE_PREAMBLE = 0xd2;
    static const uint8_t SET_REGULATOR_MODE = 0x96;
    static const uint8_t CALIBRATE = 0x89;
    static const uint8_t CALIBRATE_IMAGE = 0x98;
    static const uint8_t SET_PA_CONFIG = 0x95;
    static const uint8_t SET_RX_FALLBACK_MODE = 0x93;

    // Register and Buffer Access Commands
    static const uint8_t WRITE_REGISTER = 0x0d;
    static const uint8_t READ_REGISTER = 0x1d;
    static const uint8_t WRITE_BUFFER = 0x0e;
    static const uint8_t READ_BUFFER = 0x1e;

    // DIO and IRQ Control
    static const uint8_t SET_DIO_IRQ_PARAMS = 0x08;
    static const uint8_t GET_IRQ_STATUS = 0x12;
    static const uint8_t CLEAR_IRQ_STATUS = 0x02;
    static const uint8_t SET_DIO_AS_RF_SWITCH_CTRL = 0x9d;
    static const uint8_t SET_DIO3_AS_TCXO_CTRL = 0x97;

    // RF, Modulation and Packet Commands
    static const uint8_t SET_RF_FREQUENCY = 0x86;
    static const uint8_t SET_PACKET_TYPE = 0x8a;
    static const uint8_t GET_PACKET_TYPE = 0x11;
    static const uint8_t SET_TX_PARAMS = 0x8e;
    static const uint8_t SET_MODULATION_PARAMS = 0x8b;
    static const uint8_t SET_PACKET_PARAMS = 0x8c;
    static const uint8_t SET_CAD_PARAMS = 0x88;
    static const uint8_t SET_BUFFER_BASE_ADDRESS = 0x8f;
    static const uint8_t SET_LORA_SYMB_NUM_TIMEOUT = 0xa0;

    // Status Commands
    static const uint8_t GET_STATUS = 0xc0;
    static const uint8_t GET_RSSI_INST = 0x15;
    static const uint8_t GET_RX_BUFFER_STATUS = 0x13;
    static const uint8_t GET_PACKET_STATUS = 0x14;
    static const uint8_t GET_DEVICE_ERRORS = 0x17;
    static const uint8_t CLEAR_DEVICE_ERRORS = 0x07;
    static const uint8_t GET_STATS = 0x10;
    static const uint8_t RESET_STATS = 0x00;

private:
    spi_inst_t *spi;
    uint8_t cs_pin;
    uint8_t busy_pin;
    uint8_t reset_pin;
};
