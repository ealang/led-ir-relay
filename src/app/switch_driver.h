#ifndef SWITCH_DRIVER_H_
#define SWITCH_DRIVER_H_

#include <stdint.h>

typedef struct Mutex Mutex;

// Either autoselect or force an active port.
typedef enum {AutoSelect = -1, ForcePort0 = 0, ForcePort1 = 1} SwitchDriverMode;
typedef enum {SwitchPortNone = -1, SwitchPort0 = 0, SwitchPort1 = 1} SwitchPort;

typedef struct
{
    Mutex *mutex;

    SwitchDriverMode mode;
    SwitchPort last_port;
    SwitchPort active_kvm_port;
}  SwitchDriverState;

void switch_driver_state_init(SwitchDriverState *state, Mutex *mutex);

// Tell switch to select the active port automatically.
void switch_driver_set_mode_auto(SwitchDriverState *state);
// Force an active port.
void switch_driver_set_mode_force(SwitchDriverState *state, uint8_t port);
// Get currently active port, whether in auto or force mode.
uint8_t swtich_driver_get_active_port(const SwitchDriverState *state);
// Alert driver about change in active KVM port.
void swtich_driver_report_active_kvm_port(SwitchDriverState *state, uint8_t port);

#endif
