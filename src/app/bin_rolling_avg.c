#include "bin_rolling_avg.h"

char compute_rolling_avg(char bit, BinaryAverage *inst)
{
    char oldest_bit = inst->history >> 15;
    inst->sum += bit - oldest_bit;

    inst->history = (inst->history << 1) | bit;
    if (inst->count < 16)
    {
        ++inst->count;
        // not enough history yet, return current
        return bit;
    }
    return (inst->sum >= 8) ? 1 : 0;
}