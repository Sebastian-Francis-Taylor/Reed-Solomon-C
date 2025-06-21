#include <stdint.h>
#define gf_bits 8
#define prim_poly 0b100011101  // z^8 + z^4 + z^3 + z^2 + 1 F(2^8)
uint8_t log_table[256];
uint8_t antilog_table[256];

uint8_t antilog_table[255];

void init_gf_tables() {
    uint8_t a = 1;

    for (uint8_t i = 0; i < 255; ++i) {
        antilog_table[i] = a;
        log_table[a] = i;
        
        uint8_t hi_bit = a & 0x80;
        a <<= 1;
        if (hi_bit) {
            a ^= 0x1D;
        }
    }
    
    log_table[0] = 0;
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

uint8_t gf_div(uint8_t a,uint8_t b) {
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
