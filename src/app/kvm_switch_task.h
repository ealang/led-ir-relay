#ifndef KVM_SWITCH_TASK_H_
#define KVM_SWITCH_TASK_H_

#define KVM_SWITCH_PORT_AUTO -1

// Params for kvm switch task.
typedef struct
{
    // -1 for auto, 0 to force port 1, 1 to force port 2
    char port_selection;
}  KvmSwitchTaskCtrl;

void kvm_switch_task(void *param);

#endif