#ifndef RS_ENCODER
#define RS_ENCODER

#include "galois.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ERRORS 16
#define PARITY_SYMBOLS 2 * MAX_ERRORS
#define PRIMITIVE_ELEMENT 2
#define GENERATOR_POLYNOMIAL                                                                                           \
    {45, 216, 239, 24,  253, 104, 27, 40, 107, 50,  163, 210, 227, 134, 224, 158, 119,                                 \
     13, 158, 1,   238, 164, 82,  43, 15, 232, 246, 142, 50,  189, 29,  232, 1}
#define GENERATOR_LEN 33

void reverse_array(uint8_t *arr, int len);
uint8_t *extend_poly(const uint8_t *poly, int len, int extra);
uint8_t *shift_poly(const uint8_t *poly, int len, int k);
uint8_t *rs_encode(uint8_t *info_poly, int info_poly_len);

#endif
