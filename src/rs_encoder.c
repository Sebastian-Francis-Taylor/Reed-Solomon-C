#include "rs_encoder.h"
#include "galois.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ERRORS 16
#define PARITY_SYMBOLS_AMOUNT 2 * MAX_ERRORS
#define PRIMITIVE_ELEMENT 2

// zeros so that they do not need to be padded during runtime
static const uint8_t generator_poly[255] = {
    1,  232, 29,  189, 50, 142, 246, 232, 15, 43, 82, 164, 238, 1, 158, 13, 119, 158, 224, 134, 227, 210, 163, 50, 107,
    40, 27,  104, 253, 24, 239, 216, 45,  0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,   0, 0,   0,  0};

uint8_t *extend_poly(const uint8_t *poly, int len, int extra) {
    uint8_t *out = calloc(len + extra, 1);
    memcpy(out, poly, len * sizeof(uint8_t));

    return out;
}

uint8_t *shift_poly(const uint8_t *poly, int len, int k) {
    uint8_t *out = calloc(len + k, 1);
    memcpy(out + k, poly, len * sizeof(uint8_t));

    return out;
}

uint8_t *rs_encode(uint8_t *info_poly, int info_poly_len) {

    uint8_t *padded_info_poly = shift_poly(info_poly, info_poly_len, PARITY_SYMBOLS_AMOUNT);

    poly_div_result division_result = poly_div(padded_info_poly, (uint8_t *)generator_poly, FIELD_SIZE);
    free(division_result.quotient);
    uint8_t *parity_symbols = division_result.remainder;

    uint8_t *encoded_message = calloc(FIELD_SIZE, sizeof(uint8_t));

    memcpy(encoded_message, parity_symbols, PARITY_SYMBOLS_AMOUNT * sizeof(encoded_message[0]));
    memcpy(encoded_message + PARITY_SYMBOLS_AMOUNT, info_poly,
           (FIELD_SIZE - PARITY_SYMBOLS_AMOUNT) * sizeof(encoded_message[0]));

    free(padded_info_poly);
    free(parity_symbols);

    return encoded_message;
}
