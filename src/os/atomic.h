#ifndef ATOMIC_H_
#define ATOMIC_H_

#include <avr/interrupt.h>

#define ATOMIC(X) {\
    cli();\
    X;\
    sei();\
}

#endif