#include "kvm_switch_task.h"

#include "os/input.h"
#include "os/led.h"

#include <stdint.h>

static const LedNumber status_led = Led2;

void ui_task(void *param);
void ui_task(void *param)
{
    KvmSwitchTaskCtrl *ctrl = (KvmSwitchTaskCtrl*)param;
    led_set_state(status_led, 1);

    while (1)
    {
        Gesture press = await_input(Button0);
        if (press == ShortPress)
        {
            if (ctrl->port_selection == 0)
            {
                ctrl->port_selection = 1;
            }
            else if (ctrl->port_selection == 1)
            {
                ctrl->port_selection = KVM_SWITCH_PORT_AUTO;
            }
            else if (ctrl->port_selection == KVM_SWITCH_PORT_AUTO)
            {
                ctrl->port_selection = 0;
            }
        }
    }
}