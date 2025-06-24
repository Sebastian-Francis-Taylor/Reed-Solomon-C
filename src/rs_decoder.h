#ifndef RS_ENCODER
#define RS_ENCODER

#include "galois.h"
#include "poly.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

extern int max_degree;
extern int max_errors;
extern uint8_t message[128];
extern int message_len;

extern uint8_t generator_polynomial;
extern uint8_t roots[255];

typedef struct {
    uint8_t *error_evaluator_polynomial;
    uint8_t *error_locator_polynomial;
    int evaluator_len;
    int locator_len;
} euclidean_result;

uint8_t *calculate_syndrome(uint8_t *received_poly, int syndrome_count, int codeword_length); 
euclidean_result extended_euclidean_algorithm(uint8_t *syndrome_poly, int syndrome_len, int max_errors); 
uint8_t *calculate_error_values(uint8_t *error_positions, uint8_t *error_evaluator_polynomial, uint8_t *error_locator_polynomial, int error_amount, int error_locator_polynomial_len, int error_evaluator_polynomial_len); 
uint8_t *calculate_error_positions(uint8_t *poly, int poly_len, int *num_positions); 
uint8_t *resolve_errors(uint8_t *error_values, uint8_t *received_message); 
uint8_t *decode_message(uint8_t *encoded_message, int message_len); 

#endif
