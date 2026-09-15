#include "pico/stdlib.h"
#include "SX1262.h"
#include <cstdio>


int main()
{
    stdio_init_all();

    SX1262 lora_module("Pico-LoRa-SX1262");
    lora_module.set_frequency(915000000);

    uint8_t buff[256];

    while (true)
    {
        int packet_length = lora_module.read_packet(buff);
        if (packet_length > 0)
        {
            printf("Packet received!");
            printf("\n  Byte values: ");
            for (int i = 0; i < packet_length; i++)
            {
                printf(" %u ", buff[i]);
            }
            printf("\n  Characters: ");
            for (int i = 0; i < packet_length; i++)
            {
                printf(" %c ", buff[i]);
            }
            printf("\n");
        }
        sleep_ms(200);
    }

    return 0;
}