#ifndef SDA_GENERATED_TABLES_H
#define SDA_GENERATED_TABLES_H
#include "sda_table.h"
#define SDA_GENERATED_VERSION "deterministic-c-target-q-v1"
static const sda_u128 sda_frodo640_p[]={(sda_u128)2071,(sda_u128)3886,(sda_u128)3209,(sda_u128)2333,(sda_u128)1493,(sda_u128)841,(sda_u128)417,(sda_u128)182,(sda_u128)70,(sda_u128)24,(sda_u128)7,(sda_u128)1,(sda_u128)0};
static const sda_u128 sda_frodo640_c[]={(sda_u128)2071,(sda_u128)5957,(sda_u128)9166,(sda_u128)11499,(sda_u128)12992,(sda_u128)13833,(sda_u128)14250,(sda_u128)14432,(sda_u128)14502,(sda_u128)14526,(sda_u128)14533,(sda_u128)14534,(sda_u128)14534};
static const sda_u128 sda_frodo976_p[]={(sda_u128)1291,(sda_u128)2349,(sda_u128)1769,(sda_u128)1103,(sda_u128)569,(sda_u128)243,(sda_u128)86,(sda_u128)25,(sda_u128)6,(sda_u128)1,(sda_u128)0};
static const sda_u128 sda_frodo976_c[]={(sda_u128)1291,(sda_u128)3640,(sda_u128)5409,(sda_u128)6512,(sda_u128)7081,(sda_u128)7324,(sda_u128)7410,(sda_u128)7435,(sda_u128)7441,(sda_u128)7442,(sda_u128)7442};
static const sda_u128 sda_frodo1344_p[]={(sda_u128)29,(sda_u128)45,(sda_u128)21,(sda_u128)6,(sda_u128)1,(sda_u128)0,(sda_u128)0};
static const sda_u128 sda_frodo1344_c[]={(sda_u128)29,(sda_u128)74,(sda_u128)95,(sda_u128)101,(sda_u128)102,(sda_u128)102,(sda_u128)102};
static const sda_u128 sda_falcon_c[]={((sda_u128)1688501996594986825ULL*1000u+(sda_u128)177u),((sda_u128)3140552495174822643ULL*1000u+(sda_u128)20u),((sda_u128)4064021121781526494ULL*1000u+(sda_u128)454u),((sda_u128)4498354148456746465ULL*1000u+(sda_u128)294u),((sda_u128)4649426102519045973ULL*1000u+(sda_u128)669u),((sda_u128)4688286310671560942ULL*1000u+(sda_u128)787u),((sda_u128)4695678731498558959ULL*1000u+(sda_u128)720u),((sda_u128)4696718719969437723ULL*1000u+(sda_u128)960u),((sda_u128)4696826920814140839ULL*1000u+(sda_u128)268u),((sda_u128)4696835245983194185ULL*1000u+(sda_u128)188u),((sda_u128)4696835719696686440ULL*1000u+(sda_u128)440u),((sda_u128)4696835739630883021ULL*1000u+(sda_u128)131u),((sda_u128)4696835740251456874ULL*1000u+(sda_u128)874u),((sda_u128)4696835740265738826ULL*1000u+(sda_u128)826u),((sda_u128)4696835740265763760ULL*1000u+(sda_u128)555u),((sda_u128)4696835740265763827ULL*1000u+(sda_u128)799u),((sda_u128)4696835740265763827ULL*1000u+(sda_u128)899u),((sda_u128)4696835740265763827ULL*1000u+(sda_u128)900u),((sda_u128)4696835740265763827ULL*1000u+(sda_u128)900u)};
static const sda_u128 sda_falcon_p[]={((sda_u128)1688501996594986825ULL*1000u+(sda_u128)177u),((sda_u128)1452050498579835817ULL*1000u+(sda_u128)843u),((sda_u128)923468626606703851ULL*1000u+(sda_u128)434u),((sda_u128)434333026675219970ULL*1000u+(sda_u128)840u),((sda_u128)151071954062299508ULL*1000u+(sda_u128)375u),((sda_u128)38860208152514969ULL*1000u+(sda_u128)118u),((sda_u128)7392420826998016ULL*1000u+(sda_u128)933u),((sda_u128)1039988470878764ULL*1000u+(sda_u128)240u),(sda_u128)108200844703115308ULL,(sda_u128)8325169053345920ULL,(sda_u128)473713492255252ULL,(sda_u128)19934196580691ULL,(sda_u128)620573853743ULL,(sda_u128)14281951952ULL,(sda_u128)24933729ULL,(sda_u128)67244ULL,(sda_u128)100ULL,(sda_u128)1ULL,(sda_u128)0ULL};
static const sda_table sda_generated_tables[]={
{"Frodo","frodo640","target-q-balanced-rounding",0,12,14,0,0,1,13,(sda_u128)14534,sda_frodo640_p,sda_frodo640_c,2*13*sizeof(sda_u128),13*14},
{"Frodo","frodo976","target-q-balanced-rounding",0,10,13,0,0,1,11,(sda_u128)7442,sda_frodo976_p,sda_frodo976_c,2*11*sizeof(sda_u128),11*13},
{"Frodo","frodo1344","target-q-balanced-rounding",0,6,7,0,0,1,7,(sda_u128)102,sda_frodo1344_p,sda_frodo1344_c,2*7*sizeof(sda_u128),7*7},
{"Falcon","falcon","target-q fixture from existing repository cumulative data",0,18,73,0,0,1,19,(sda_u128)4696835740265763827ULL*1000+900,sda_falcon_p,sda_falcon_c,2*19*sizeof(sda_u128),19*73}
};
static const size_t sda_generated_tables_count=4;
#endif
