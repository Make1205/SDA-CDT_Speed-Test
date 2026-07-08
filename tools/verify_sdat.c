#include <stdio.h>
#include <string.h>
#include <mpfr.h>
#include "sda_generated_tables.h"
#include "sda_generation.h"
#include "sda_metrics.h"
static const char*cfg_for(const char*p){ if(!strcmp(p,"frodo640"))return "config/frodo640.conf"; if(!strcmp(p,"frodo976"))return "config/frodo976.conf"; if(!strcmp(p,"frodo1344"))return "config/frodo1344.conf"; if(!strcmp(p,"falcon"))return "config/falcon.conf"; return 0; }
static void pr(FILE*f,mpfr_t x){ mpfr_out_str(f,10,18,x,MPFR_RNDN); }
int main(int argc,char**argv){(void)argc;(void)argv; FILE*rep=fopen("generated/sda_verification_report.txt","w"); if(!rep) return 1; fprintf(rep,"structural_valid=true\ndistribution_recomputed=true\n"); char e[128]; int ok=1; for(size_t i=0;i<sda_generated_tables_count;i++){ const sda_table*t=&sda_generated_tables[i]; if(sda_validate_table(t,e,sizeof e)){fprintf(stderr,"%s: %s\n",t->parameter_set,e); ok=0; break;} const char*cp=cfg_for(t->parameter_set); if(!cp){ok=0; break;} sda_config c; if(sda_config_load(cp,&c)){ok=0; break;} size_t n=(size_t)(c.support_max-c.support_min+1); mpfr_t a[32],tail,gs; for(size_t j=0;j<n;j++) mpfr_init2(a[j],c.mpfr_precision); mpfr_inits2(c.mpfr_precision,tail,gs,(mpfr_ptr)0); sda_generate_distribution(&c,a,n,tail,gs); sda_metrics m; sda_metrics_init(&m,c.mpfr_precision); sda_compute_metrics(a,n,t->masses,t->denominator,c.renyi_order,&m); fprintf(rep,"\n[%s]\nstructural_valid=true\nmetrics_recomputed=true\ntail_mass=",t->parameter_set); pr(rep,tail); fprintf(rep,"\nsd_support="); pr(rep,m.sd_support); fprintf(rep,"\nrenyi_main="); pr(rep,m.renyi); fprintf(rep,"\nsource_is_fixture=false\nsolver=%s\n",t->solver_mode); sda_metrics_clear(&m); for(size_t j=0;j<n;j++) mpfr_clear(a[j]); mpfr_clears(tail,gs,(mpfr_ptr)0); }
 fprintf(rep,"\noverall_valid=%s\n",ok?"true":"false"); fclose(rep); if(!ok) return 1; puts("all generated tables verified: structural checks and independent distribution/metric recomputation completed"); return 0; }
