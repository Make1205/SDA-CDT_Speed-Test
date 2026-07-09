#include "sda_exact_linf.h"
int main(void){ long B[]={2,1,0,3}; sda_linf_stats st; if(sda_exact_linf_solve(B,2,&st)) return 1; long br=sda_linf_bruteforce(B,2,4); return st.best_norm==br?0:1;}
