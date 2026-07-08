#include <stdio.h>
#include "sda_generated_tables.h"
int main(void){ for(size_t i=0;i<sda_generated_tables_count;i++) printf("%s offline solver=%s generation_time=fixture deterministic exact=%d heuristic=%d\n",sda_generated_tables[i].parameter_set,sda_generated_tables[i].solver_mode,sda_generated_tables[i].exact,sda_generated_tables[i].heuristic); return 0; }
