#include "falcon_reverse_tail.h"

/* The Falcon reference gaussian0_sampler stores each threshold as three
 * 24-bit words ordered most-significant, middle, least-significant.  Its
 * comparison loads them in the reverse order into v2, v1, v0 and propagates
 * subtraction borrow from the least-significant limb upward. */
#define L72(a, b, c) { (c), (b), (a) }

static const falcon_u72_limbs original_tail[FALCON_GAUSSIAN0_TAIL_ROWS] = {
    L72(10745844u,3068844u,3741698u), L72(5559083u,1580863u,8248194u),
    L72(2260429u,13669192u,2736639u), L72(708981u,4421575u,10046180u),
    L72(169348u,7122675u,4136815u), L72(30538u,13063405u,7650655u),
    L72(4132u,14505003u,7826148u), L72(417u,16768101u,11363290u),
    L72(31u,8444042u,8086568u), L72(1u,12844466u,265321u),
    L72(0u,1232676u,13644283u), L72(0u,38047u,9111839u),
    L72(0u,870u,6138264u), L72(0u,14u,12545723u),
    L72(0u,0u,3104126u), L72(0u,0u,28824u), L72(0u,0u,198u),
    L72(0u,0u,1u), L72(0u,0u,0u)
};

/* Exact reverse tails T_j=q-sum_{i<j}p_i for j=1..18, followed by the
 * zero sentinel used by the official fixed 19-row Falcon loop. */
static const falcon_u72_limbs sda_tail[FALCON_GAUSSIAN0_TAIL_ROWS] = {
    L72(10685138u,6094140u,7629470u), L72(5527678u,8670096u,14258550u),
    L72(2247660u,1865395u,13975599u), L72(704976u,1013644u,5723061u),
    L72(168391u,12334147u,15547454u), L72(30366u,4325902u,15083834u),
    L72(4109u,8674399u,7074295u), L72(415u,10705185u,8982890u),
    L72(31u,5458207u,9679761u), L72(1u,12677126u,1949237u),
    L72(0u,1225713u,2179374u), L72(0u,37832u,10125434u),
    L72(0u,865u,7532402u), L72(0u,14u,11147952u),
    L72(0u,0u,3086591u), L72(0u,0u,28662u), L72(0u,0u,197u),
    L72(0u,0u,1u), L72(0u,0u,0u)
};

falcon_u72_limbs falcon_u72_limbs_from_le9(const uint8_t in[9]) {
    falcon_u72_limbs x;
    x.v0 = (uint32_t)in[0] | ((uint32_t)in[1] << 8) | ((uint32_t)in[2] << 16);
    x.v1 = (uint32_t)in[3] | ((uint32_t)in[4] << 8) | ((uint32_t)in[5] << 16);
    x.v2 = (uint32_t)in[6] | ((uint32_t)in[7] << 8) | ((uint32_t)in[8] << 16);
    return x;
}

falcon_u72_limbs falcon_u72_limbs_from_sdat(sdat_u72 x) {
    falcon_u72_limbs r = {
        (uint32_t)(x.lo & 0xFFFFFFu),
        (uint32_t)((x.lo >> 24) & 0xFFFFFFu),
        (uint32_t)(((x.lo >> 48) & 0xFFFFu) | ((uint32_t)x.hi << 16))
    };
    return r;
}

sdat_u72 falcon_u72_limbs_to_sdat(falcon_u72_limbs x) {
    sdat_u72 r;
    r.lo = (uint64_t)x.v0 | ((uint64_t)x.v1 << 24) | ((uint64_t)(x.v2 & 0xFFFFu) << 48);
    r.hi = (uint8_t)(x.v2 >> 16);
    return r;
}

int falcon_u72_limbs_lt(falcon_u72_limbs a, falcon_u72_limbs b) {
    uint32_t cc = (a.v0 - b.v0) >> 31;
    cc = (a.v1 - b.v1 - cc) >> 31;
    return (int)((a.v2 - b.v2 - cc) >> 31);
}

uint32_t falcon_gaussian0_reverse_tail_lookup(
    falcon_u72_limbs candidate,
    const falcon_u72_limbs table[FALCON_GAUSSIAN0_TAIL_ROWS]) {
    uint32_t z = 0;
    for (size_t u = 0; u < FALCON_GAUSSIAN0_TAIL_ROWS; u++) {
        uint32_t cc = (candidate.v0 - table[u].v0) >> 31;
        cc = (candidate.v1 - table[u].v1 - cc) >> 31;
        cc = (candidate.v2 - table[u].v2 - cc) >> 31;
        z += cc;
    }
    return z;
}

const falcon_u72_limbs *falcon_gaussian0_original_reverse_tail_table(void) { return original_tail; }
const falcon_u72_limbs *falcon_gaussian0_sda_reverse_tail_table(void) { return sda_tail; }
falcon_u72_limbs falcon_gaussian0_sda_q_limbs(void) {
    const falcon_u72_limbs q = { 15028336u, 8925203u, 16682437u };
    return q;
}
