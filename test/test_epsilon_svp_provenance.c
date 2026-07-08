#include "sda_generation.h"
#include <string.h>
int main(void){ sda_config c; if(sda_config_builtin("frodo1344",&c)) return 1; c.renyi_order=200; sda_generation_result r; sda_generation_result_init(&r,c.mpfr_precision); if(sda_generate_for_config(&c,"epsilon-svp-generated",&r)) return 2; int ok=(r.q==r.raw_svp_q && r.q==r.exact_svp_q && r.final_q_from_exact_svp && r.denominator_from_exact_svp && r.global_svp_certified && r.baseline_dominance_certified); sda_generation_result_clear(&r); return ok?0:3; }
