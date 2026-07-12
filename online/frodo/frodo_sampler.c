#include "frodo_sampler.h"
#include "sdat_avx2.h"
#include <string.h>

static frodo_sampler_params params[] = {
    {FRODO_PARAM_640,"frodo640",&original_cdt_table_frodo640,&sda_table_frodo640,14534u,14u,14u,0x3fffu,0,13,0,11,12,5120},
    {FRODO_PARAM_976,"frodo976",&original_cdt_table_frodo976,&sda_table_frodo976,7442u,13u,13u,0x1fffu,0,11,0,9,10,7808},
    {FRODO_PARAM_1344,"frodo1344",&original_cdt_table_frodo1344,&sda_table_frodo1344,102u,7u,7u,0x007fu,0,7,0,4,6,10752},
};

const frodo_sampler_params *frodo_get_sampler_params(frodo_param_id id){
    if(id<0 || (size_t)id>=sizeof(params)/sizeof(params[0]))return 0;
    frodo_sampler_params *p=&params[id];
    p->original_cdf=(const uint16_t *)p->original_table->thresholds;
    p->sda_thresholds=p->sda_table->thresholds;
    return p;
}
const char *frodo_sampler_kind_name(frodo_sampler_kind k){return k==FRODO_SAMPLER_ORIGINAL_CDT?"original-cdt":k==FRODO_SAMPLER_SDA_CDT?"sda-cdt":"unknown";}
const char *frodo_backend_name(frodo_backend b){return b==FRODO_BACKEND_REFERENCE?"reference":b==FRODO_BACKEND_AVX2?"avx2":"unknown";}
const char *frodo_frontend_name(frodo_frontend f){return f==FRODO_FRONTEND_ORIGINAL_WORD?"original-word":f==FRODO_FRONTEND_PACKED_BIT?"packed-bit":f==FRODO_FRONTEND_WORD_ORIENTED?"word-oriented":"unknown";}
const char *frodo_implementation_label(frodo_sampler_kind k,frodo_backend b,frodo_frontend f){
    if(k==FRODO_SAMPLER_ORIGINAL_CDT)return b==FRODO_BACKEND_AVX2?"original-avx2":"original-reference";
    if(f==FRODO_FRONTEND_PACKED_BIT)return b==FRODO_BACKEND_AVX2?"sda-packed-avx2":"sda-packed-reference";
    if(f==FRODO_FRONTEND_WORD_ORIENTED)return b==FRODO_BACKEND_AVX2?"sda-word-avx2":"sda-word-reference";
    return "invalid";
}
int frodo_backend_available(frodo_backend b){return b==FRODO_BACKEND_REFERENCE || (b==FRODO_BACKEND_AVX2 && sdat_avx2_cpu_supported());}
int frodo_sample_n_dispatch(frodo_sampler_kind kind,frodo_backend backend,frodo_frontend frontend,
                            frodo_param_id param,uint16_t*out,size_t n,
                            const uint8_t*packed_source,size_t packed_source_len,
                            const uint16_t*word_source,size_t word_count,
                            frodo_sampler_stats*fs){
    const frodo_sampler_params*p=frodo_get_sampler_params(param); if(!p||!out)return -1;
    if(!frodo_backend_available(backend))return -9;
    if(fs)*fs=(frodo_sampler_stats){0};
    if(kind==FRODO_SAMPLER_ORIGINAL_CDT){
        if(frontend!=FRODO_FRONTEND_ORIGINAL_WORD||!word_source||word_count<n)return -2;
        memcpy(out,word_source,n*sizeof *out);
        return backend==FRODO_BACKEND_AVX2?frodo_original_sample_n_avx2(out,n,p->original_table):frodo_original_sample_n(out,n,p->original_table);
    }
    if(kind!=FRODO_SAMPLER_SDA_CDT)return -3;
    if(frontend==FRODO_FRONTEND_PACKED_BIT){
        if(!packed_source)return -4;
        sdat_bitreader_fast br; sdat_bitreader_fast_init(&br,packed_source,packed_source_len);
        int rc=backend==FRODO_BACKEND_AVX2?frodo_sda_sample_n_fast_avx2(out,n,&br,p->sda_table,fs?&fs->stats:0):frodo_sda_sample_n_fast(out,n,&br,p->sda_table,fs?&fs->stats:0);
        if(fs)fs->reader=br;
        return rc;
    }
    if(frontend==FRODO_FRONTEND_WORD_ORIENTED){
        if(!word_source)return -5;
        return backend==FRODO_BACKEND_AVX2?frodo_sda_word_sample_n_avx2(out,n,word_source,word_count,p->sda_table,fs?&fs->stats:0):frodo_sda_word_sample_n(out,n,word_source,word_count,p->sda_table,fs?&fs->stats:0);
    }
    return -6;
}
