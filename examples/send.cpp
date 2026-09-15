#include "pico/stdlib.h"
#include "SX1262.h"
#include <cstdio>


int main()
{
    stdio_init_all();

    SX1262 lora_module("Pico-LoRa-SX1262");
    lora_module.set_frequency(915000000);

    uint8_t buff[16] = "Hello, world!";

    while (true)
    {
        lora_module.send_packet(buff, 13);
        sleep_ms(2500);
    }

    return 0;
}
