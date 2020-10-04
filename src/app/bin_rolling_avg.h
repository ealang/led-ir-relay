#ifndef BIN_ROLLING_AVG_H_
#define BIN_ROLLING_AVG_H_

#include <stdint.h>

typedef struct {
    uint8_t count;
    uint8_t sum;
    uint16_t history;
} BinaryAverage;

// Single bit rolling average. History of len 16.
char compute_rolling_avg(char bit, BinaryAverage *inst);

#endif