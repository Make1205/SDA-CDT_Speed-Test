#include <stdio.h>
#include "sda_generated_tables.h"
int main(int argc,char**argv){(void)argc;(void)argv; puts("offline report for generated production tables (source_is_fixture=false)"); for(size_t i=0;i<sda_generated_tables_count;i++) printf("%s offline solver=%s exact=%d heuristic=%d source_is_fixture=0\n",sda_generated_tables[i].parameter_set,sda_generated_tables[i].solver_mode,sda_generated_tables[i].exact,sda_generated_tables[i].heuristic); return 0; }
