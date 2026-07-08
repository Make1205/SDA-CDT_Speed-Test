#include "sda_baseline.h"
#include <string.h>
static const sda_u128 frodo640_p[]={2071,3886,3209,2333,1493,841,417,182,70,24,7,1,0};
static const sda_u128 frodo976_p[]={1291,2349,1769,1103,569,243,86,25,6,1,0};
static const sda_u128 frodo1344_p[]={29,45,21,6,1,0,0};
static sda_u128 sum(const sda_u128*p,size_t n){sda_u128 s=0;for(size_t i=0;i<n;i++)s+=p[i];return s;}
const sda_u128 *sda_frodo_original_pmf(const char *ps, size_t *n, sda_u128 *q){ const sda_u128*p=0; size_t m=0; if(!strcmp(ps,"frodo640")){p=frodo640_p;m=13;} else if(!strcmp(ps,"frodo976")){p=frodo976_p;m=11;} else if(!strcmp(ps,"frodo1344")){p=frodo1344_p;m=7;} if(!p)return 0; if(n)*n=m; if(q)*q=sum(p,m); return p; }
int sda_frodo_original_available(const char *ps){ size_t n; sda_u128 q; return sda_frodo_original_pmf(ps,&n,&q)!=0 && q>0; }
