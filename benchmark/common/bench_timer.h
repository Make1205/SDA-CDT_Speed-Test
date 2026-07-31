#ifndef BENCH_TIMER_H
#define BENCH_TIMER_H
#include <stdint.h>
#include <time.h>
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
static inline uint64_t bench_timer_start(void){__asm__ __volatile__("":::"memory");_mm_lfence();return __rdtsc();}
static inline uint64_t bench_timer_stop(void){unsigned aux;uint64_t v=__rdtscp(&aux);_mm_lfence();__asm__ __volatile__("":::"memory");return v;}
#define BENCH_TIMER_KIND "x86-lfence-rdtsc-rdtscp-lfence"
#else
static inline uint64_t bench_clock(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (uint64_t)ts.tv_sec*1000000000u+(uint64_t)ts.tv_nsec;}
static inline uint64_t bench_timer_start(void){return bench_clock();}
static inline uint64_t bench_timer_stop(void){return bench_clock();}
#define BENCH_TIMER_KIND "clock-monotonic-nanoseconds"
#endif
#endif
