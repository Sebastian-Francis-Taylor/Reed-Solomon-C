#ifndef RS_DECODER_H
#define RS_DECODER_H

#include "galios.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *error_locator;
    uint8_t *error_evaluator;
    int locator_deg;
    int evaluator_deg;
} key_equation_result;

uint8_t *calculate_syndrome(uint8_t *received, int n, int t);
key_equation_result solve_key_equation(uint8_t *S, int t);
uint8_t *find_error_positions(uint8_t *locator, int deg, int n, int *num_errors);
uint8_t *calculate_error_values(uint8_t *positions, int num_errors, 
                                uint8_t *locator, int loc_deg,
                                uint8_t *evaluator, int eval_deg);
uint8_t *decode_rs(uint8_t *received, int n, int t);

#endif
