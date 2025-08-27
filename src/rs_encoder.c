#include "rs_encoder.h"
#include "galois.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ERRORS 16
#define PARITY_SYMBOLS_AMOUNT 2 * MAX_ERRORS
#define PRIMITIVE_ELEMENT 2

// long so that it does not need to be padded during runtime
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

static const int field_size = 255;

uint8_t *extend_poly(const uint8_t *poly, int len, int extra) {
    uint8_t *out = calloc(len + extra, 1);
    for (int i = 0; i < len; ++i) {
        out[i] = poly[i];
    }
    return out;
}

uint8_t *shift_poly(const uint8_t *poly, int len, int k) {
    uint8_t *out = calloc(len + k, 1);
    for (int i = 0; i < len; ++i) {
        out[i + k] = poly[i];
    }
    return out;
}

uint8_t *rs_encode(uint8_t *info_poly, int info_poly_len) {

    // printf("info_poly: ");
    // for (int i = 0; i < info_poly_len; ++i) {
    // printf("%d ", info_poly[i]);
    // }
    // printf("\n");
    // printf("\n");

    uint8_t *padded_info_poly = shift_poly(info_poly, info_poly_len, PARITY_SYMBOLS_AMOUNT);

    // printf("padded_info_poly_len: %d\n", info_poly_len + PARITY_SYMBOLS_AMOUNT);
    // printf("padded_info_poly: ");
    // for (int i = 0; i < (info_poly_len + PARITY_SYMBOLS_AMOUNT); ++i) {
    // printf("%d ", padded_info_poly[i]);
    // }
    // printf("\n");
    // printf("\n");

    printf("padded_info_poly (dividend): ");
    for (int i = 0; i < field_size; ++i) {
        printf("%d ", padded_info_poly[i]);
    }
    printf("\n");
    printf("\n");

    printf("extended_generator (divisor): ");
    for (int i = 0; i < field_size; ++i) {
        printf("%d ", generator_poly[i]);
    }
    printf("\n");
    printf("\n");

    poly_div_result division_result = poly_div(padded_info_poly, (uint8_t *)generator_poly, field_size);
    uint8_t *parity_symbols = division_result.remainder;

    printf("parity_symbols: ");
    for (int i = 0; i < 32; ++i) {
        printf("%d ", parity_symbols[i]);
    }
    printf("\n");
    printf("\n");

    uint8_t *encoded_message = calloc(field_size, sizeof(uint8_t));

    for (int i = 0; i < PARITY_SYMBOLS_AMOUNT; ++i) {
        encoded_message[i] = parity_symbols[i];
    }

    int iter = 0;
    for (int i = PARITY_SYMBOLS_AMOUNT; i < field_size; ++i) {
        encoded_message[i] = info_poly[iter];
        ++iter;
    }
    // memcpy(encoded_message, shifted_info_poly, info_poly_len);
    // memcpy(encoded_message + info_poly_len, division_result.remainder, PARITY_SYMBOLS);

    return encoded_message;
}
