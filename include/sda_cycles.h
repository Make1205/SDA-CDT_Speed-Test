#ifndef SDA_CYCLES_H
#define SDA_CYCLES_H
#include <stdint.h>
int sda_cycles_supported(void);
uint64_t sda_cycles_start(void);
uint64_t sda_cycles_stop(void);
uint64_t sda_cycles_measure_overhead(void);
int sda_cycles_pin_to_cpu(int cpu);
const char *sda_cycles_cpu_model(void);
#endif
