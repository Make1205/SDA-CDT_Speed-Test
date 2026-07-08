#include "sda_generation.h"
#include <string.h>
int main(void){
  const char*names[]={"frodo640","frodo976","frodo1344"};
  for(size_t i=0;i<3;i++){
    sda_config c; if(sda_config_builtin(names[i],&c)) return 10+i; c.renyi_order=200;
    sda_generation_result r; sda_generation_result_init(&r,c.mpfr_precision);
    if (sda_generate_for_config(&c, "exact-linf-svp", &r)) return 20 + i;
    if (!r.exact_svp_q || r.q != r.exact_svp_q || r.raw_svp_q != r.q) return 30 + i;
    if (!r.baseline_dominance_certified || !r.production_eligible) return 50 + i;
    if (!r.final_q_from_exact_svp || !r.denominator_from_exact_svp) return 60 + i;
    sda_generation_result_clear(&r);
  }
  return 0;
}
