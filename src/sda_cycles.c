#define _GNU_SOURCE
#include "sda_cycles.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#include <cpuid.h>
static inline void barrier(void){ __asm__ __volatile__("" ::: "memory"); }
int sda_cycles_supported(void){ return 1; }
uint64_t sda_cycles_start(void){ unsigned a,b,c,d; __cpuid(0,a,b,c,d); barrier(); return __rdtsc(); }
uint64_t sda_cycles_stop(void){ unsigned aux; uint64_t t=__rdtscp(&aux); barrier(); unsigned a,b,c,d; __cpuid(0,a,b,c,d); return t; }
#else
#include <time.h>
int sda_cycles_supported(void){ return 0; }
uint64_t sda_cycles_start(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC_RAW,&ts); return (uint64_t)ts.tv_sec*1000000000ull+(uint64_t)ts.tv_nsec; }
uint64_t sda_cycles_stop(void){ return sda_cycles_start(); }
#endif
uint64_t sda_cycles_measure_overhead(void){ uint64_t best=~(uint64_t)0; for(int i=0;i<1000;i++){ uint64_t a=sda_cycles_start(); uint64_t b=sda_cycles_stop(); if(b-a<best) best=b-a; } return best; }
int sda_cycles_pin_to_cpu(int cpu){ cpu_set_t set; CPU_ZERO(&set); CPU_SET(cpu,&set); return sched_setaffinity(0,sizeof(set),&set); }
const char *sda_cycles_cpu_model(void){ static char buf[256]; FILE*f=fopen("/proc/cpuinfo","r"); if(!f){strcpy(buf,"unknown"); return buf;} while(fgets(buf,sizeof buf,f)){ if(!strncmp(buf,"model name",10)){ char*p=strchr(buf,':'); if(p){ while(*++p==' '); memmove(buf,p,strlen(p)+1); char*n=strchr(buf,'\n'); if(n)*n=0; fclose(f); return buf; } } } fclose(f); strcpy(buf,"unknown"); return buf; }
