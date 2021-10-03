;.include "m328pdef.inc"

; Wrapper function that injects the param into the thread.
.global _thread_startup_wrapper
_thread_startup_wrapper:

    ; Retrieve threads starting param
    pop r25
    pop r24

    ; Retrieve thread func addr
    pop zh
    pop zl

    ; Call func
    icall

    ; Not supporting thread termination. For now restart the entire program.
    jmp 0

; Switch from one thread to another.
; @param thread_from pointer to current thread struct
; @param thread_to pointer to other thread struct
.global _scheduler_switch_threads
_scheduler_switch_threads:

    ; Push registers
    push r0
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    push r28
    push r29

    ; Save stack pointer
    mov xh, r25  ; param1 h
    mov xl, r24  ; param1 l

    in r18, 0x3d  ; SPL
    st X+, r18
    in r18, 0x3e  ; SPH
    st X, r18

    ; shift param2 to param1
    mov r25, r23
    mov r24, r22


; Switch to given thread
; @param thread pointer to Thread struct
.global _scheduler_resume_thread
_scheduler_resume_thread:

    ; Restore stack pointer
    mov xh, r25  ; param1 h
    mov xl, r24  ; param1 l

    ld r18, X+
    out 0x3d, r18  ; SPL
    ld r18, X
    out 0x3e, r18  ; SPH

    ; Pop registers
    pop r29
    pop r28
    pop r17
    pop r16
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r0

    ret
