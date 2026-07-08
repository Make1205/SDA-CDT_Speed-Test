#include "sda_generated_tables.h"
#include <string.h>
int main(void){
  for(size_t i=0;i<sda_generated_tables_count;i++){
    const sda_table *t=&sda_generated_tables[i];
    if(!strcmp(t->solver_mode,"exact-denominator-search")){
      if(t->exact || t->heuristic || t->target_q_mode) return 1;
    } else if(!strcmp(t->solver_mode,"exact-linf-svp")){
      if(!t->exact || t->heuristic || t->target_q_mode) return 2;
    } else return 3;
  }
  return 0;
}
