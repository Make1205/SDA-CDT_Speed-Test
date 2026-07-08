#include <stdio.h>
#include <string.h>
#include "sda_config.h"
#include "sda_generated_tables.h"
#include "sda_lll.h"
int main(int argc,char**argv){ int all=0; const char*solver="target-q"; const char*cfgpath=NULL; for(int i=1;i<argc;i++){ if(!strcmp(argv[i],"--all"))all=1; else if(!strcmp(argv[i],"--solver")&&i+1<argc)solver=argv[++i]; else if(!strcmp(argv[i],"--config")&&i+1<argc)cfgpath=argv[++i]; else if(!strcmp(argv[i],"--target-q")&&i+1<argc)i++; }
 if(!strcmp(solver,"lll")){ fprintf(stderr,"lll mode: %s\n",sda_lll_status()); return 2; }
 if(all||!cfgpath){ puts("generated/sda_generated_tables.h is up to date (deterministic target-q fixtures)"); for(size_t i=0;i<sda_generated_tables_count;i++) printf("%s solver=%s exact=%d heuristic=%d target_q=%d\n",sda_generated_tables[i].parameter_set,sda_generated_tables[i].solver_mode,sda_generated_tables[i].exact,sda_generated_tables[i].heuristic,sda_generated_tables[i].target_q_mode); return 0; }
 sda_config c; if(sda_config_load(cfgpath,&c)){perror(cfgpath); return 1;} printf("config=%s solver=%s construction=target-q-balanced-rounding exact-svp=false q=%s\n",cfgpath,solver,c.target_q_text); return 0; }
