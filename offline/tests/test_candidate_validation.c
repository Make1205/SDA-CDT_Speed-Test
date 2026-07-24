#include "sda_generation.h"

int main(void) {
    sda_config c;
    if (sda_config_builtin("frodo1344", &c)) return 1;
    c.precision_k = 7;
    const long long valid[] = {40, 30, 20, 10, 2};
    const long long wrong_sum[] = {40, 30, 20, 10, 1};
    const long long negative[] = {40, 30, -1, 31, 2};
    if (sda_validate_signed_candidate(&c, valid, 5, 102)) return 2;
    if (!sda_validate_signed_candidate(&c, wrong_sum, 5, 102)) return 3;
    if (!sda_validate_signed_candidate(&c, negative, 5, 102)) return 4;
    if (!sda_validate_signed_candidate(&c, valid, 5, 128)) return 5;
    if (!sda_validate_signed_candidate(&c, valid, 5, 0)) return 6;
    return 0;
}
