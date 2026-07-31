#ifndef FRODO_WORD_CORE_H
#define FRODO_WORD_CORE_H
#include <stdint.h>
static inline uint16_t frodo640_word_candidate(uint16_t word){return (uint16_t)(word&UINT16_C(0x3fff));}
static inline uint16_t frodo976_word_candidate(uint16_t word){return (uint16_t)(word&UINT16_C(0x1fff));}
static inline uint8_t frodo1344_word_candidate(uint16_t word){return (uint8_t)(word&UINT16_C(0x007f));}
static inline uint8_t frodo640_word_sign(uint16_t word){return (uint8_t)((word>>14)&1u);}
static inline uint8_t frodo976_word_sign(uint16_t word){return (uint8_t)((word>>13)&1u);}
static inline uint8_t frodo1344_word_sign(uint16_t word){return (uint8_t)((word>>7)&1u);}
static inline int frodo640_word_accept(uint16_t candidate){return candidate<UINT16_C(14534);}
static inline int frodo976_word_accept(uint16_t candidate){return candidate<UINT16_C(7442);}
static inline int frodo1344_word_accept(uint8_t candidate){return candidate<UINT8_C(102);}
#endif
