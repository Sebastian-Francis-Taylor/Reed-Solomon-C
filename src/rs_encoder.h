#ifndef RS_ENCODER
#define RS_ENCODER

#include "galois.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void reverse_array(uint8_t *arr, int len);
uint8_t *extend_poly(const uint8_t *poly, int len, int extra);
uint8_t *shift_poly(const uint8_t *poly, int len, int k);
uint8_t *rs_encode(uint8_t *info_poly, int info_poly_len);

#endif
