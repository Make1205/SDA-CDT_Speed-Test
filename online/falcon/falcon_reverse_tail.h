#ifndef FALCON_REVERSE_TAIL_H
#define FALCON_REVERSE_TAIL_H

#include <stddef.h>
#include <stdint.h>
#include "sdat_tables.h"

#define FALCON_GAUSSIAN0_TAIL_ROWS 19u

/* Falcon's Gaussian0 representation: three little-endian 24-bit limbs.
 * v0 is least significant and v2 is most significant. */
typedef struct {
    uint32_t v0;
    uint32_t v1;
    uint32_t v2;
} falcon_u72_limbs;

falcon_u72_limbs falcon_u72_limbs_from_le9(const uint8_t in[9]);
falcon_u72_limbs falcon_u72_limbs_from_sdat(sdat_u72 x);
sdat_u72 falcon_u72_limbs_to_sdat(falcon_u72_limbs x);
int falcon_u72_limbs_lt(falcon_u72_limbs a, falcon_u72_limbs b);

/* This is the sole compiled mapping kernel used by both online variants. */
uint32_t falcon_gaussian0_reverse_tail_lookup(
    falcon_u72_limbs candidate,
    const falcon_u72_limbs table[FALCON_GAUSSIAN0_TAIL_ROWS]);

const falcon_u72_limbs *falcon_gaussian0_original_reverse_tail_table(void);
const falcon_u72_limbs *falcon_gaussian0_sda_reverse_tail_table(void);
falcon_u72_limbs falcon_gaussian0_sda_q_limbs(void);

#endif
