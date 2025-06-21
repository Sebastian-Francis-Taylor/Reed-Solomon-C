#ifndef GALIOS_H
#define GALIOS_H

#include <stdint.h>

void init_gf_tables();
uint8_t gf_add(uint8_t a, uint8_t b);
uint8_t gf_mult(uint8_t a,uint8_t b);
uint8_t gf_div(uint8_t a,uint8_t b);
uint8_t gf_pow(uint8_t base, uint8_t exponent);
int gf_deg(uint8_t poly);

#endif
