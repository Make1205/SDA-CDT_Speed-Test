#include "sda_generated_tables.h"
int main(void){char e[64]; for(size_t i=0;i<sda_generated_tables_count;i++) if(sda_validate_table(&sda_generated_tables[i],e,sizeof e)) return 1; return 0;}
