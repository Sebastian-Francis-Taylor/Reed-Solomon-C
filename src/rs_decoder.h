#ifndef RS_DECODER_H
#define RS_DECODER_H

#include "galois.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Global constants
extern int max_degree;
extern int max_errors;
extern uint8_t message[128];
extern int message_len;
extern uint8_t generator_polynomial;
extern uint8_t roots[255];

// Structure for Extended Euclidean Algorithm result
typedef struct {
    uint8_t *error_evaluator_polynomial;
    uint8_t *error_locator_polynomial;
    int evaluator_len;
    int locator_len;
} euclidean_result;

// Core Reed-Solomon decoding functions
uint8_t *calculate_syndrome(uint8_t *received_poly, int syndrome_count, int codeword_length);
euclidean_result extended_euclidean_algorithm(uint8_t *syndrome_poly, int syndrome_len, int max_errors);
uint8_t *calculate_error_values(uint8_t *error_positions, uint8_t *error_evaluator_polynomial,
                                uint8_t *error_locator_polynomial, int error_amount,
                                int error_locator_polynomial_len, int error_evaluator_polynomial_len);
uint8_t *calculate_error_positions(uint8_t *poly, int poly_len, int *num_positions);
uint8_t *resolve_errors(uint8_t *error_values, uint8_t *received_message, int message_len);

// Main decoding function
uint8_t *decode_message(uint8_t *encoded_message, int message_len, int max_errors);

#endif // RS_DECODER_H
