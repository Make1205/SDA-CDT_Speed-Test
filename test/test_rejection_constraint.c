#include "sda_config.h"
#include "sda_generation.h"
#include <mpfr.h>
static int draw_bits(sda_u128 q){ int b=0; sda_u128 v=q-1; while(v){b++;v>>=1;} return b?b:1; }
static int acc_ge(sda_u128 q,sda_u128 h){ return q*(((sda_u128)1)<<draw_bits(h)) >= h*(((sda_u128)1)<<draw_bits(q)); }
int main(void){ if(acc_ge(8536,14534)) return 1; if(acc_ge(278,7442)) return 2; if(acc_ge(88,102)) return 3; if(!acc_ge(14534,14534)) return 4; return 0; }
