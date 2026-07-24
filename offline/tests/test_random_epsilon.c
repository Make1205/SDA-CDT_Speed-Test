#include "sda_epsilon.h"
#include <string.h>

int main(void) {
    mpfr_t lo, hi, a, b;
    mpfr_inits2(256, lo, hi, a, b, (mpfr_ptr)0);
    sda_epsilon_bounds(15, 13, lo, hi);
    sda_epsilon_rng r1, r2;
    uint64_t seed = sda_parameter_seed(1, "frodo640");
    if (seed == sda_parameter_seed(1, "frodo976")) return 1;
    sda_epsilon_rng_init(&r1, seed); sda_epsilon_rng_init(&r2, seed);
    for (int i = 0; i < 1000; i++) {
        sda_epsilon_random(&r1, lo, hi, a); sda_epsilon_random(&r2, lo, hi, b);
        if (mpfr_cmp(a, lo) <= 0 || mpfr_cmp(a, hi) >= 0 || mpfr_cmp(a, b)) return 2;
    }
    mpfr_clears(lo, hi, a, b, (mpfr_ptr)0);
    return 0;
}
