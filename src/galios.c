#include <stdint.h>
#define gf_bits 8
#define prim_poly 497

uint8_t log_table[256];
uint8_t antilog_table[256];

void init_gf_tables() {
    uint8_t a = 1;

    for (uint8_t i = 0; i < 255 ; ++i) {
        antilog_table[i] = a;

        uint8_t hi_bit = a & 128;
        a <<= 1;
        if (hi_bit == 128) {
            a ^= 283;
        }

        a ^= antilog_table[i];
        log_table[antilog_table[i]] = i;
    }

    antilog_table[255] = antilog_table[0];

    log_table[0] = 0;
    log_table[255] = 1;
}

uint8_t gf_add(uint8_t a, uint8_t b) {
    return a^b;
}

uint8_t gf_mult(uint8_t a,uint8_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0;
    
    uint8_t log_result = (log_table[a] + log_table[b]) % 255;

    return antilog_table[log_result];
}

uint8_t df_div(uint8_t a,uint8_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0;
    
    uint8_t log_result = (log_table[a] - log_table[b] + 255) % 255;

    return antilog_table[log_result];
}

uint8_t gf_pow(uint8_t base, uint8_t exponent) {
    if (exponent == 0) return 1;
    if (base == 0) return 0;

    return antilog_table[(log_table[base]*exponent)%255];
}
