#include "sda_config.h"
#include "sda_generation.h"
int main(void){
  const char *paths[] = {"../config/frodo640.conf", "../config/frodo976.conf", "../config/frodo1344.conf"};
  const sda_u128 app_q[] = {(sda_u128)16384u, (sda_u128)7442u, (sda_u128)102u};
  const sda_u128 svp_q[] = {(sda_u128)8536u, (sda_u128)5966u, (sda_u128)1986u};
  for (int i = 0; i < 3; i++) {
    sda_config c;
    if (sda_config_load(paths[i], &c)) return 10 + i;
    sda_generation_result r;
    sda_generation_result_init(&r, c.mpfr_precision);
    if (sda_generate_for_config(&c, "exact-linf-svp", &r)) return 20 + i;
    if (r.q != app_q[i]) return 30 + i;
    if (r.exact_svp_q != svp_q[i]) return 40 + i;
    if (!r.baseline_dominance_certified || !r.production_eligible) return 50 + i;
    if (r.final_q_from_exact_svp) return 60 + i;
    sda_generation_result_clear(&r);
  }
  return 0;
}
