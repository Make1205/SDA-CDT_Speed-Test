#include "sda_u128.h"
int main(void){ sda_u128 f; if(sda_parse_u128("4696835740265763827900",&f)) return 1; return sda_bitlength_u128(102)==7 && sda_bitlength_u128(7442)==13 && sda_bitlength_u128(14534)==14 && sda_bitlength_u128(f)==72 ? 0:1; }
