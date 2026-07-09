#ifndef SDA_CONFIG_H
#define SDA_CONFIG_H
#include "sda_u128.h"
typedef struct {
  char scheme[32], parameter_set[32], solver[32]; char target_q_text[80];
  double sigma; int support_min,support_max,precision_k; long renyi_order; unsigned long mpfr_precision; sda_u128 manuscript_q,target_q;
  double epsilon_min,epsilon_max,epsilon_min_interval_width; int epsilon_initial_trials,epsilon_refinement_rounds,epsilon_refinement_factor,epsilon_max_total_instances,epsilon_deduplicate_q; unsigned long epsilon_initial_precision,epsilon_max_precision; char epsilon_schedule[32];
} sda_config;
int sda_config_builtin(const char *name, sda_config *cfg);
int sda_config_load(const char *path, sda_config *cfg);
void sda_config_defaults(sda_config *cfg);
#endif
