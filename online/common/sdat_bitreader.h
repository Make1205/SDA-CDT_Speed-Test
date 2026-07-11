#ifndef SDAT_BITREADER_H
#define SDAT_BITREADER_H
#include <stddef.h>
#include <stdint.h>
/* LSB-first bit order: bit 0 of buf[0] is consumed first; multi-bit takes
 * return the low-order requested bits in chronological stream order. */
typedef struct { const uint8_t *buf; size_t len; size_t pos; uint64_t reservoir; unsigned available; uint64_t bits_consumed; } sdat_bitreader;
void sdat_bitreader_init(sdat_bitreader *reader,const uint8_t *buf,size_t len);
int sdat_bitreader_take(sdat_bitreader *reader,unsigned bits,uint32_t *out);
size_t sdat_bitreader_source_bytes_consumed(const sdat_bitreader *reader);
#endif
