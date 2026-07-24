#include "sda_falcon_bkz.h"
int main(void){char version[128]={0};int available=sda_falcon_bkz_available(version,sizeof version);return available&&!version[0];}
