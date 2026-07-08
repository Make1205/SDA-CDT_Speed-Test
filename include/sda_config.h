#ifndef SDA_CONFIG_H
#define SDA_CONFIG_H
#include "sda_u128.h"
typedef struct { char scheme[32], parameter_set[32], solver[32]; char target_q_text[80]; double sigma; int support_min,support_max,precision_k; long renyi_order; unsigned long mpfr_precision; sda_u128 manuscript_q,target_q; } sda_config;
int sda_config_builtin(const char *name, sda_config *cfg);
int sda_config_load(const char *path, sda_config *cfg);
void sda_config_defaults(sda_config *cfg);
#endif
