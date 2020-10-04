#include <stdint.h>

#include "os/led.h"
#include "os/time.h"
#include "os/light_sensor.h"

#include "bin_rolling_avg.h"

static const LedNumber port1_led = Led1;
static const LedNumber port2_led = Led2;

void kvm_switch_task(void *param);
void kvm_switch_task(void *param)
{
    BinaryAverage avg = {0, 0, 0};
    while(1)
    {
        char cur_sensor = read_max_light_sensor() == Sensor1 ? 0 : 1;
        char avg_sensor = compute_rolling_avg(cur_sensor, &avg);
        led_set_state(port1_led, avg_sensor);

        await_sleep(MS_TO_TICKS(100 / 16));
    }
}
