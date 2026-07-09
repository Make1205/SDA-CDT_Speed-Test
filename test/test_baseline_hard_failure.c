#include "sda_u128.h"
int main(void){ sda_u128 compact_limit=((sda_u128)1)<<15; int reject_equal = !(((sda_u128)32768) < compact_limit); int accept_below = (((sda_u128)32767) < compact_limit); return (reject_equal && accept_below)?0:1; }
