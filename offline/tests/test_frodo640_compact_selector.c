#include <stdint.h>
#include <string.h>

typedef struct {
  uint64_t q, sd_num, sd_den, rd_num, rd_den;
  int certified;
  const char *id;
} cand;
static unsigned bits(uint64_t q){ unsigned b=0; uint64_t m=q-1; while(m){b++;m>>=1;} return b?b:1; }
static uint64_t ceil_pow2(uint64_t q){ return 1ull<<bits(q); }
static int feasible(cand c, cand orig){ return c.q>0 && c.q < 32768 && c.certified && c.sd_num*orig.sd_den <= orig.sd_num*c.sd_den && c.rd_num*orig.rd_den <= orig.rd_num*c.rd_den; }
static int gap_less(uint64_t q1,uint64_t q2){ uint64_t Q1=ceil_pow2(q1),Q2=ceil_pow2(q2); return (Q1-q1)*Q2 < (Q2-q2)*Q1; }
static int elog_less(uint64_t q1,uint64_t q2){ unsigned b1=bits(q1),b2=bits(q2); uint64_t Q1=ceil_pow2(q1),Q2=ceil_pow2(q2); return (uint64_t)b1*Q1*q2 < (uint64_t)b2*Q2*q1; }
static int rat_less(uint64_t an,uint64_t ad,uint64_t bn,uint64_t bd){ return an*bd < bn*ad; }
static int better(cand a,cand b,cand orig){ int fa=feasible(a,orig), fb=feasible(b,orig); if(fa!=fb)return fa>fb; if(!fa)return 0; if(gap_less(a.q,b.q))return 1; if(gap_less(b.q,a.q))return 0; if(elog_less(a.q,b.q))return 1; if(elog_less(b.q,a.q))return 0; if(a.q!=b.q)return a.q<b.q; if(rat_less(a.sd_num,a.sd_den,b.sd_num,b.sd_den))return 1; if(rat_less(b.sd_num,b.sd_den,a.sd_num,a.sd_den))return 0; if(rat_less(a.rd_num,a.rd_den,b.rd_num,b.rd_den))return 1; if(rat_less(b.rd_num,b.rd_den,a.rd_num,a.rd_den))return 0; return strcmp(a.id,b.id)<0; }
int main(void){ cand orig={32768,1,1000,1,1000,1,"orig"}; cand current={14534,1,2000,1,2000,1,"cur"}; cand too_big={32768,0,1,0,1,1,"big"}; if(feasible(too_big,orig))return 1; cand uncert={16000,0,1,0,1,0,"unc"}; if(feasible(uncert,orig))return 2; cand badsd={16000,2,1000,0,1,1,"sd"}; if(feasible(badsd,orig))return 3; cand badrd={16000,0,1,2,1000,1,"rd"}; if(feasible(badrd,orig))return 4; cand close={16300,1,2000,1,2000,1,"close"}; cand small={8000,1,2000,1,2000,1,"small"}; if(!better(close,small,orig))return 5; cand elog={8100,1,2000,1,2000,1,"elog"}; if(!better(elog,small,orig))return 6; cand qsmall={8191,1,2000,1,2000,1,"a"}; cand qbig={8191,1,2000,1,2000,1,"b"}; if(!better(qsmall,qbig,orig))return 7; if(better(current,current,orig))return 8; cand arr1[3]={small,close,elog}; cand best=arr1[0]; for(int i=1;i<3;i++) if(better(arr1[i],best,orig)) best=arr1[i]; cand arr2[3]={elog,small,close}; cand best2=arr2[0]; for(int i=1;i<3;i++) if(better(arr2[i],best2,orig)) best2=arr2[i]; if(strcmp(best.id,best2.id))return 9; return 0; }
