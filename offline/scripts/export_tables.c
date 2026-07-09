#include <stdio.h>
#include "sda_generated_tables.h"
int main(void){ char q[64],v[64]; for(size_t i=0;i<sda_generated_tables_count;i++){ const sda_table*t=&sda_generated_tables[i]; sda_print_u128(t->denominator,q,sizeof q); printf("%s q=%s masses=",t->parameter_set,q); for(size_t j=0;j<t->table_length;j++){sda_print_u128(sda_table_mass_at(t,j),v,sizeof v); printf("%s%s",j?" ":"",v);} puts(""); } return 0; }
