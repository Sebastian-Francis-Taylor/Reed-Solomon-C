#ifndef POLY_H
#define POLY_H

#include "galios.h"
#include <stdint.h>
#include <stdlib.h>

int poly_degree(uint8_t *poly, int len); 
uint8_t *poly_add(uint8_t *a, uint8_t *b, int len); 
uint8_t *poly_mult(uint8_t *a, uint8_t *b, int len); 
uint8_t *poly_div(uint8_t *dividend, uint8_t *divisor, int len); 

#endif
