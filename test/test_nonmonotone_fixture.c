#include "sda_table.h"
extern const sda_u128 reference_falcon_nonmonotone_c[];
int main(void){ sda_u128 p[19]; p[0]=reference_falcon_nonmonotone_c[0]; for(int i=1;i<19;i++) p[i]=reference_falcon_nonmonotone_c[i]-reference_falcon_nonmonotone_c[i-1]; sda_table t={"Falcon","bad","reference",0,18,72,0,0,0,19,reference_falcon_nonmonotone_c[18],p,reference_falcon_nonmonotone_c,0,0}; char e[128]; return sda_validate_table(&t,e,sizeof e)==0 ? 1:0; }
