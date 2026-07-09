#include <stdint.h>
typedef struct{int valid,baseline; unsigned long q,gap; double rgap,sd,rd,err,eps;} C;
static int better(C a,C b){ if(a.valid!=b.valid)return a.valid>b.valid; if(a.baseline!=b.baseline)return a.baseline>b.baseline; if(a.q!=b.q)return a.q<b.q; if(a.gap!=b.gap)return a.gap<b.gap; if(a.rgap!=b.rgap)return a.rgap<b.rgap; if(a.sd!=b.sd)return a.sd<b.sd; if(a.rd!=b.rd)return a.rd<b.rd; if(a.err!=b.err)return a.err<b.err; return a.eps<b.eps; }
int main(void){ C small={1,1,100,28,.21,1,1,1,.4}; C close={1,1,127,1,.007,0,0,0,.3}; if(!better(small,close)) return 1; C a={1,1,100,30,.3,1,1,1,.4}; C b={1,1,100,20,.2,1,1,1,.4}; if(!better(b,a)) return 2; return 0; }
