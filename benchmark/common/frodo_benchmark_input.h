#ifndef FRODO_BENCHMARK_INPUT_H
#define FRODO_BENCHMARK_INPUT_H
#include <stddef.h>
#include <stdint.h>
static inline uint64_t frodo_benchmark_mix64(uint64_t x){x^=x>>30;x*=UINT64_C(0xbf58476d1ce4e5b9);x^=x>>27;x*=UINT64_C(0x94d049bb133111eb);return x^(x>>31);}
static inline uint64_t frodo_benchmark_seed(unsigned param,size_t process,size_t repetition,unsigned stream){return frodo_benchmark_mix64(UINT64_C(0x53444146524f444f)^((uint64_t)param<<48)^((uint64_t)process<<24)^(uint64_t)repetition^((uint64_t)stream<<56));}
static inline uint64_t frodo_benchmark_prng_next(uint64_t*s){*s=*s*UINT64_C(6364136223846793005)+UINT64_C(1442695040888963407);return *s;}
static inline void frodo_benchmark_fill_words(uint16_t*out,size_t n,uint64_t seed){for(size_t i=0;i<n;i++)out[i]=(uint16_t)(frodo_benchmark_prng_next(&seed)>>32);}
static inline void frodo_benchmark_fill_bytes(uint8_t*out,size_t n,uint64_t seed){for(size_t i=0;i<n;i++)out[i]=(uint8_t)(frodo_benchmark_prng_next(&seed)>>32);}
#endif
