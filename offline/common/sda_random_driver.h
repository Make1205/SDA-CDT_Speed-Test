#ifndef SDA_RANDOM_DRIVER_H
#define SDA_RANDOM_DRIVER_H
#include <stdint.h>
int sda_random_generate_config(const char *config_path, uint64_t master_seed,
                               unsigned max_trials);
#endif
