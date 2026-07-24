#include <stdio.h>
#include "sda_random_driver.h"
#include "sda_epsilon.h"
#include "sda_falcon_bkz.h"
#include "sda_generation.h"
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

static void print_u128(FILE *f, sda_u128 x) { char b[64]; sda_print_u128(x,b,sizeof b); fputs(b,f); }
static void print_c_u128(FILE *f,sda_u128 x){fprintf(f,"(((sda_u128)%lluULL<<64)|(sda_u128)%lluULL)",(unsigned long long)(x>>64),(unsigned long long)x);}
static int make_dir(const char *p) { return mkdir(p,0777)==0 || errno==EEXIST ? 0 : -1; }
static void write_failure(const sda_config*c,mpfr_t lo,mpfr_t hi,uint64_t master,uint64_t seed,unsigned trials,const char*why,FILE*log){char dir[160],path[220];snprintf(dir,sizeof dir,"offline/generated/%s",c->parameter_set);if(make_dir("offline/generated")||make_dir(dir))return;snprintf(path,sizeof path,"%s/selected_report.txt",dir);FILE*f=fopen(path,"w");if(!f)return;fprintf(f,"parameter_set=%s\nepsilon_lower=",c->parameter_set);mpfr_out_str(f,10,0,lo,MPFR_RNDN);fprintf(f,"\nepsilon_upper=");mpfr_out_str(f,10,0,hi,MPFR_RNDN);fprintf(f,"\nmaster_seed=%llu\nparameter_seed=%llu\ntrials_completed=%u\ngeneration_status=not_found\nmain_failure_reason=%s\nnormalization_applied=false\ntrial_log:\n",(unsigned long long)master,(unsigned long long)seed,trials,why);rewind(log);int ch;while((ch=fgetc(log))!=EOF)fputc(ch,f);fclose(f);}
static const char *reason(const sda_generation_result *r, int rc) {
    if (rc != -8) return "solver_failure";
    if (!r->q) return "q_invalid";
    if (!r->raw_svp_pmf_valid) return "sum_mismatch";
    if (!r->baseline_dominance_certified) return "SD_or_RD_failure";
    return "candidate_invalid";
}
static int write_candidate(const sda_config *c,const sda_generation_result *r,
                           mpfr_t lo,mpfr_t hi,uint64_t master,uint64_t seed,
                           unsigned trial,FILE *log) {
    char dir[160],path[220]; snprintf(dir,sizeof dir,"offline/generated/%s",c->parameter_set);
    if(make_dir("offline/generated")||make_dir(dir))return -1;
    snprintf(path,sizeof path,"%s/selected_report.txt",dir); FILE*f=fopen(path,"w");if(!f)return -1;
    fprintf(f,"parameter_set=%s\nk=%d\nn=%zu\nepsilon_lower=",c->parameter_set,c->precision_k,r->n);mpfr_out_str(f,10,0,lo,MPFR_RNDN);fprintf(f,"\nepsilon_upper=");mpfr_out_str(f,10,0,hi,MPFR_RNDN);fprintf(f,"\nselected_epsilon=");mpfr_out_str(f,10,0,r->epsilon,MPFR_RNDN);
    fprintf(f,"\nmaster_seed=%llu\nparameter_seed=%llu\naccepted_trial=%u\nsolver=%s\nsolver_version=builtin-C17\nsolver_exact=%s\nMPFR_precision=%lu\nq=",(unsigned long long)master,(unsigned long long)seed,trial,r->solver,r->exact_linf_svp?"true":"false",(unsigned long)c->mpfr_precision);print_u128(f,r->q);
    fprintf(f,"\np=");for(size_t i=0;i<r->n;i++){if(i)fputc(' ',f);print_u128(f,r->p[i]);}fprintf(f,"\ncumulative=");for(size_t i=0;i<r->n;i++){if(i)fputc(' ',f);print_u128(f,r->c[i]);}
    fprintf(f,"\nmax_pointwise_error=");mpfr_out_str(f,10,0,r->max_abs_error,MPFR_RNDN);fprintf(f,"\nSD=");mpfr_out_str(f,10,0,r->sd_infinite,MPFR_RNDN);fprintf(f,"\nRD=");mpfr_out_str(f,10,0,r->renyi,MPFR_RNDN);fprintf(f,"\nRenyi_order=%ld\nsum_p_equals_q=true\nq_below_2k=true\nnormalization_applied=false\ngeneration_status=accepted\ntrial_log:\n",c->renyi_order);rewind(log);int ch;while((ch=fgetc(log))!=EOF)fputc(ch,f);fclose(f);
    snprintf(path,sizeof path,"%s/selected_table.txt",dir);f=fopen(path,"w");if(!f)return -1;fprintf(f,"q=");print_u128(f,r->q);fprintf(f,"\np=");for(size_t i=0;i<r->n;i++){if(i)fputc(' ',f);print_u128(f,r->p[i]);}fputc('\n',f);fclose(f);
    snprintf(path,sizeof path,"%s/selected_table.h",dir);f=fopen(path,"w");if(!f)return -1;fprintf(f,"/* Candidate only; not promoted online. */\n#include \"sda_u128.h\"\nstatic const sda_u128 selected_q=");print_c_u128(f,r->q);fprintf(f,";\nstatic const sda_u128 selected_p[]={");for(size_t i=0;i<r->n;i++){if(i)fputc(',',f);print_c_u128(f,r->p[i]);}fprintf(f,"};\nstatic const sda_u128 selected_cumulative[]={");for(size_t i=0;i<r->n;i++){if(i)fputc(',',f);print_c_u128(f,r->c[i]);}fprintf(f,"};\n");fclose(f);return 0;
}
int sda_random_generate_config(const char *path,uint64_t master,unsigned max_trials){
    sda_config c;if(sda_config_load(path,&c))return 1;size_t n=(size_t)(c.support_max-c.support_min+1);uint64_t seed=sda_parameter_seed(master,c.parameter_set);sda_epsilon_rng rng;sda_epsilon_rng_init(&rng,seed);mpfr_t lo,hi,eps;mpfr_inits2(c.mpfr_precision,lo,hi,eps,(mpfr_ptr)0);sda_epsilon_bounds(c.precision_k,n,lo,hi);FILE*log=tmpfile();if(!log)return 1;
    if(!strcmp(c.scheme,"Falcon")){char v[128];if(!sda_falcon_bkz_available(v,sizeof v)){fprintf(log,"falcon,0,%llu,N/A,-1,reject,backend_unavailable\n",(unsigned long long)seed);write_failure(&c,lo,hi,master,seed,0,"fplll_unavailable",log);fprintf(stderr,"falcon: backend unavailable: fplll not found\n");fclose(log);mpfr_clears(lo,hi,eps,(mpfr_ptr)0);return 2;}fprintf(log,"falcon,0,%llu,N/A,-1,reject,bkz20_recovery_unavailable\n",(unsigned long long)seed);write_failure(&c,lo,hi,master,seed,0,"bkz20_candidate_recovery_unavailable",log);fprintf(stderr,"falcon: fplll %s found, but BKZ-20 candidate recovery is unavailable\n",v);fclose(log);mpfr_clears(lo,hi,eps,(mpfr_ptr)0);return 2;}
    for(unsigned t=1;t<=max_trials;t++){sda_epsilon_random(&rng,lo,hi,eps);sda_generation_result r;sda_generation_result_init(&r,c.mpfr_precision);int rc=sda_generate_for_epsilon(&c,eps,&r);fprintf(log,"%s,%u,%llu,",c.parameter_set,t,(unsigned long long)seed);mpfr_out_str(log,10,18,eps,MPFR_RNDN);fprintf(log,",%d,%s,%s\n",rc,rc?"reject":"accept",rc?reason(&r,rc):"none");if(!rc){int w=write_candidate(&c,&r,lo,hi,master,seed,t,log);sda_generation_result_clear(&r);fclose(log);mpfr_clears(lo,hi,eps,(mpfr_ptr)0);return w?1:0;}sda_generation_result_clear(&r);}
    write_failure(&c,lo,hi,master,seed,max_trials,"no_legal_solver_candidate",log);fprintf(stderr,"%s: generation_status=not_found after %u trials\n",c.parameter_set,max_trials);fclose(log);mpfr_clears(lo,hi,eps,(mpfr_ptr)0);return 2;
}
