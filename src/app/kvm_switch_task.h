#ifndef KVM_SWITCH_TASK_H_
#define KVM_SWITCH_TASK_H_

#define KVM_PORT_SELECTION_AUTO -1

// Params for kvm switch task.
typedef struct
{
    // -1 for auto, 0 to force port 0, 1 to force port 1
    char port_selection;
}  KvmSwitchTaskCtrl;

void kvm_switch_task(void *param);

#endif
