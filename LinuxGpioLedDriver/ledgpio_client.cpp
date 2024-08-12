
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include "ledgpio.h"
#include "ledgpio_client.h"

namespace Gpio
{

bool Led::State::set(unsigned port, bool on)
{
    if (port < Led::min() || port > Led::max())
        return false;

    unsigned m = 1 << (port - GPIO_MIN_PORT);
    bool res = mask_ & m;

    if (on)
        mask_ |= m;
    else
        mask_ ^= m;

    return res;
}

bool Led::State::get(unsigned port) const
{
    if (port < Led::min() || port > Led::max())
        return false;

    return mask_ & (1 << (port - 2));
}

void Led::State::clean()
{
    mask_ = 0;
}

unsigned Led::min()
{
    return GPIO_MIN_PORT;
}

unsigned Led::max()
{
    return GPIO_MAX_PORT;
}

Led::Led(const State &state)
{
    f_ = 0;

    if ((f_ = open("/dev/" DEVICE_NAME, O_RDWR)) >= 0)
    {
        led_cmd_t cmd = {LED_OP_START, state.mask_};

        if (write(f_, &cmd, sizeof(cmd)) < 0)
        {
            close(f_);
            f_ = 0;
        }
    }
}

Led::~Led()
{
    if (f_)
    {
        led_cmd_t cmd = {LED_OP_STOP, 0};

        write(f_, &cmd, sizeof(cmd));
        close(f_);
    }
}

bool Led::is_open() const
{
    return f_;
}

bool Led::get(State &state) const
{
    if (!f_)
        return false;

    led_cmd_t cmd = {LED_OP_GET, state.mask_};

    int ret = read(f_, &cmd, sizeof(cmd));
    state.mask_ = cmd.leds;

    return ret >= 0;
}

bool Led::set(const State &state) const
{
    if (!f_)
        return false;

    led_cmd_t cmd = {LED_OP_SET, state.mask_};

    return write(f_, &cmd, sizeof(cmd)) >= 0;
}

} // namespace Gpio
