#ifndef SDA_U128_H
#define SDA_U128_H
#include <stdint.h>
#include <stddef.h>
typedef unsigned __int128 sda_u128;
int sda_parse_u128(const char *s, sda_u128 *out);
void sda_print_u128(sda_u128 v, char *out, size_t n);
unsigned sda_bitlength_u128(sda_u128 v);
#endif
