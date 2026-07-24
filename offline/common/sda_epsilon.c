#include "sda_epsilon.h"

static uint64_t splitmix64(uint64_t *state) {
    uint64_t z = (*state += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}

void sda_epsilon_bounds(int k, size_t n, mpfr_t lower, mpfr_t upper) {
    mpfr_t exponent;
    mpfr_init2(exponent, mpfr_get_prec(lower));
    mpfr_set_si(exponent, -k, MPFR_RNDN);
    mpfr_div_ui(exponent, exponent, (unsigned long)n, MPFR_RNDN);
    mpfr_exp2(lower, exponent, MPFR_RNDN);
    mpfr_set_si(exponent, -k, MPFR_RNDN);
    mpfr_div_ui(exponent, exponent, (unsigned long)n + 1, MPFR_RNDN);
    mpfr_exp2(upper, exponent, MPFR_RNDN);
    mpfr_clear(exponent);
}

uint64_t sda_parameter_seed(uint64_t master_seed, const char *name) {
    uint64_t h = master_seed ^ UINT64_C(0xcbf29ce484222325);
    for (const unsigned char *p = (const unsigned char *)name; *p; p++)
        h = (h ^ *p) * UINT64_C(0x100000001b3);
    return splitmix64(&h);
}

void sda_epsilon_rng_init(sda_epsilon_rng *rng, uint64_t seed) { rng->state = seed; }

void sda_epsilon_random(sda_epsilon_rng *rng, mpfr_t lower, mpfr_t upper,
                        mpfr_t epsilon) {
    uint64_t x = splitmix64(&rng->state);
    mpfr_prec_t prec = mpfr_get_prec(epsilon);
    mpfr_t u, width;
    mpfr_inits2(prec, u, width, (mpfr_ptr)0);
    /* (x + 1) / (2^64 + 1) is strictly inside (0,1). */
    mpfr_set_ui_2exp(u, (unsigned long)(x >> 32), 32, MPFR_RNDN);
    mpfr_add_ui(u, u, (unsigned long)(x & UINT32_MAX), MPFR_RNDN);
    mpfr_add_ui(u, u, 1, MPFR_RNDN);
    mpfr_t denominator;
    mpfr_init2(denominator, prec);
    mpfr_set_ui_2exp(denominator, 1, 64, MPFR_RNDN);
    mpfr_add_ui(denominator, denominator, 1, MPFR_RNDN);
    mpfr_div(u, u, denominator, MPFR_RNDN);
    mpfr_sub(width, upper, lower, MPFR_RNDN);
    mpfr_mul(epsilon, u, width, MPFR_RNDN);
    mpfr_add(epsilon, epsilon, lower, MPFR_RNDN);
    mpfr_clears(u, width, denominator, (mpfr_ptr)0);
}
