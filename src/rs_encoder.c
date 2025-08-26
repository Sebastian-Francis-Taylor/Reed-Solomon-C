#include "rs_encoder.h"
#include "galois.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ERRORS 16
#define PARITY_SYMBOLS 2 * MAX_ERRORS
#define PRIMITIVE_ELEMENT 2
#define GENERATOR_POLYNOMIAL {45, 216, 239, 24, 253, 104, 27, 40, 107, 50, 163, 210, 227, 134, 224, 158, 119, 13, 158, 1, 238, 164, 82, 43, 15, 232, 246, 142, 50, 189, 29, 232, 1}
#define GENERATOR_LEN 33
#define FIELD_SIZE 255

uint8_t *shift_polynomial_left(uint8_t *poly, int poly_len, int shift_number) {
    uint8_t *shifted_polynomial = calloc(poly_len + shift_number, sizeof(uint8_t));
    for (int i = 0; i < poly_len; ++i) {
        shifted_polynomial[i] = poly[i];
    }

    return shifted_polynomial;
}

uint8_t *rs_encode(uint8_t *info_poly, int info_poly_len) {

    uint8_t *shifted_info_poly = shift_polynomial_left(info_poly, info_poly_len, PARITY_SYMBOLS);
    int shifted_info_poly_len = info_poly_len + PARITY_SYMBOLS;

    printf("shifted_info_poly: ");
    for (int i = 0; i < shifted_info_poly_len; ++i) {
        printf("%d ", shifted_info_poly[i]);
    }
    printf("\n");

    uint8_t generator[] = GENERATOR_POLYNOMIAL;
    int generator_len = GENERATOR_LEN;
    uint8_t *shifted_generator = shift_polynomial_left(generator, generator_len, FIELD_SIZE - generator_len);
    int generator_deg = poly_degree(shifted_generator, FIELD_SIZE - generator_len);
    printf("generator_deg: %d\n", generator_deg);

    poly_div_result division_result = poly_div(shifted_info_poly, shifted_generator, FIELD_SIZE);
    uint8_t *parity_symbols = division_result.remainder;
    int parity_symbols_len = 32;

    printf("parity_symbols: ");
    for (int i = 0; i < parity_symbols_len; ++i) {
        printf("%d ", parity_symbols[i]);
    }
    printf("\n");

    uint8_t *encoded_message = calloc(FIELD_SIZE, sizeof(uint8_t));

    printf("info_poly_len: %d\n", info_poly_len);
    for (int i = 0; i < info_poly_len; ++i) {
        encoded_message[i] = info_poly[i];
    }

    int iter = PARITY_SYMBOLS - 1;
    for (int i = info_poly_len; i < FIELD_SIZE; ++i) {
        encoded_message[i] = parity_symbols[iter];
        iter--;
    }
    printf("\n");
    // memcpy(encoded_message, shifted_info_poly, info_poly_len);
    // memcpy(encoded_message + info_poly_len, division_result.remainder, PARITY_SYMBOLS);

    return encoded_message;
}
