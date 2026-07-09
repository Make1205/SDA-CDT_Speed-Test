#ifndef ORIGINAL_BASELINE_TABLES_H
#define ORIGINAL_BASELINE_TABLES_H
#include "sda_table.h"
static const uint16_t orig_frodo640_p[]={4669,8761,7235,5260,3366,1896,940,410,157,53,16,4,1};
static const uint16_t orig_frodo640_c[]={4669,13430,20665,25925,29291,31187,32127,32537,32694,32747,32763,32767,32768};
static const sda_table orig_frodo640_table={"Frodo","frodo640","original-frodo-cdt",0,12,15,0,0,0,13,(sda_u128)32768ULL,orig_frodo640_p,orig_frodo640_c,26,208};
static const uint16_t orig_frodo976_p[]={5684,10342,7789,4855,2506,1070,378,111,27,5,1};
static const uint16_t orig_frodo976_c[]={5684,16026,23815,28670,31176,32246,32624,32735,32762,32767,32768};
static const sda_table orig_frodo976_table={"Frodo","frodo976","original-frodo-cdt",0,10,15,0,0,0,11,(sda_u128)32768ULL,orig_frodo976_p,orig_frodo976_c,22,176};
static const uint16_t orig_frodo1344_p[]={9338,14470,6731,1880,315,32,2};
static const uint16_t orig_frodo1344_c[]={9338,23808,30539,32419,32734,32766,32768};
static const sda_table orig_frodo1344_table={"Frodo","frodo1344","original-frodo-cdt",0,6,15,0,0,0,7,(sda_u128)32768ULL,orig_frodo1344_p,orig_frodo1344_c,14,112};
static const sda_table *original_baseline_tables[]={&orig_frodo640_table,&orig_frodo976_table,&orig_frodo1344_table};
static const size_t original_baseline_tables_count=3;
#endif
