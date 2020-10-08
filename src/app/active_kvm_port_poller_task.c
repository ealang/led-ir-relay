#include "os/time.h"
#include "os/light_sensor.h"
#include "bin_rolling_avg.h"
#include "switch_driver.h"

void active_kvm_port_poller_task(void *param);
void active_kvm_port_poller_task(void *param)
{
    SwitchDriverState *switch_driver_state = (SwitchDriverState*)param;

    BinaryAverage sensor_history = {0, 0, 0};

    char initialized = 0;
    char last_active_port = 0;
    while (1)
    {
        uint8_t cur_active_port = compute_rolling_avg(read_max_light_sensor(), &sensor_history);
        if (!initialized || cur_active_port != last_active_port)
        {
            initialized = 1;
            last_active_port = cur_active_port;
            swtich_driver_report_active_kvm_port(switch_driver_state, cur_active_port);
        }
        await_sleep(MS_TO_TICKS(500 / 16));
    }
}