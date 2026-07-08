#include "sda_generated_tables.h"
#include <string.h>
int main(void){
  for(size_t i=0;i<sda_generated_tables_count;i++){
    const sda_table *t=&sda_generated_tables[i];
    if(!strcmp(t->solver_mode,"exact-linf-svp")) return 1;
    if(strcmp(t->solver_mode,"exact-denominator-search")) return 2;
    if(t->exact || t->heuristic || t->target_q_mode) return 3;
  }
  return 0;
}
