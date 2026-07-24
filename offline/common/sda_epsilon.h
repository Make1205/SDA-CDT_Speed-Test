#ifndef SDA_EPSILON_H
#define SDA_EPSILON_H
#include <mpfr.h>
#include <stdint.h>
#include "sda_config.h"

typedef struct { uint64_t state; } sda_epsilon_rng;
void sda_epsilon_bounds(int k, size_t n, mpfr_t lower, mpfr_t upper);
uint64_t sda_parameter_seed(uint64_t master_seed, const char *parameter_set);
void sda_epsilon_rng_init(sda_epsilon_rng *rng, uint64_t seed);
void sda_epsilon_random(sda_epsilon_rng *rng, mpfr_t lower, mpfr_t upper,
                        mpfr_t epsilon);
#endif
