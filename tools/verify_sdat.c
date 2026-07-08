#include <stdio.h>
#include "sda_generated_tables.h"
int main(int argc,char**argv){(void)argc;(void)argv; char e[128]; for(size_t i=0;i<sda_generated_tables_count;i++){ if(sda_validate_table(&sda_generated_tables[i],e,sizeof e)){fprintf(stderr,"%s: %s\n",sda_generated_tables[i].parameter_set,e);return 1;} } puts("all generated tables verified: nonnegative masses, monotone cumulative thresholds, terminal threshold equals q"); return 0;}
