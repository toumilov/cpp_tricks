
#include "ledgpio_client.h"
#include <cstdio>
#include <unistd.h>

int main()
{
    Gpio::Led::State leds;

    leds.set(18, true);
    leds.set(12, true);

    Gpio::Led ports(leds); // Initialize ports 12 and 18

    if (!ports.is_open())
    {
        printf("failed to open driver\n");
        return 1;
    }

    leds.clean();
    leds.set(18, true); 

    if (!ports.set(leds)) // Turn 18 on
    {
        printf("failed to set LEDs\n");
        return 1;
    }

    sleep(5);

    if (!ports.get(leds)) // Check state
    {
        printf("failed to get LEDs\n");
        return 1;
    }

    leds.set(18, false);

    if (!ports.set(leds)) // Turn 18 off
    {
        printf("failed to set LEDs\n");
        return 1;
    }

    return 0;
}
