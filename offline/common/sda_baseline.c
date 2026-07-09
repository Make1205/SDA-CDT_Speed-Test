#include "sda_baseline.h"
#include <string.h>
/* Original Frodo 2^15 CDT PMFs reconstructed from offline/generated/original_baseline_tables.h.
   Historical paper SDA denominators are kept separately as performance references and
   are not returned by this production baseline API. */
static const sda_u128 frodo640_p[]={4669,8761,7235,5260,3366,1896,940,410,157,53,16,4,1};
static const sda_u128 frodo976_p[]={5684,10342,7789,4855,2506,1070,378,111,27,5,1};
static const sda_u128 frodo1344_p[]={9338,14470,6731,1880,315,32,2};
static sda_u128 sum(const sda_u128*p,size_t n){sda_u128 s=0;for(size_t i=0;i<n;i++)s+=p[i];return s;}
const sda_u128 *sda_frodo_original_pmf(const char *ps, size_t *n, sda_u128 *q){ const sda_u128*p=0; size_t m=0; if(!strcmp(ps,"frodo640")){p=frodo640_p;m=13;} else if(!strcmp(ps,"frodo976")){p=frodo976_p;m=11;} else if(!strcmp(ps,"frodo1344")){p=frodo1344_p;m=7;} if(!p)return 0; if(n)*n=m; if(q)*q=sum(p,m); return p; }
int sda_frodo_original_available(const char *ps){ size_t n; sda_u128 q; return sda_frodo_original_pmf(ps,&n,&q)!=0 && q==((sda_u128)1<<15); }
