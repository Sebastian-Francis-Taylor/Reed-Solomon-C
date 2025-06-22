#include <stdint.h>
#include <stdlib.h>
#include "galios.h"

int max_degree = 32;
int max_errors = 8;
uint8_t message[128];
int message_len = 128;

uint8_t generator_polynomial;
uint8_t roots[255];


typedef struct {
    uint8_t error_locator_polynomial;
    uint8_t error_evaluator_polynomial;
} euclideon_result;

uint8_t calculate_syndrome(uint8_t *received_poly, int syndrome_count, int codeword_length) {
   int errors_detected = 0;
   uint8_t syndrome_output[syndrome_count];
   
   for (int i = 1; i <= syndrome_count; i++) {
       uint8_t alpha_i = gf_pow(2, i);
       uint8_t result = 0;
       uint8_t x_power = 1;
       
       for (int j = 0; j < codeword_length; j++) {
           result = gf_add(result, gf_mult(received_poly[j], x_power));
           x_power = gf_mult(x_power, alpha_i);
       }
       syndrome_output[i-1] = result;
       
       if (result != 0) {
           errors_detected = 1;
       }
   }
   
   return errors_detected;
}

euclideon_result extended_euclideon_algorithm(uint8_t a, uint8_t b) {
    
    uint8_t remainder_prev = a;
    uint8_t remainder_curr = b;
    uint8_t helper_s_prev = 1, helper_s_curr = 0;
    uint8_t helper_t_prev = 0, helper_t_curr = 1;
    int iteration = 0;

    while ((gf_deg(remainder_curr) >=  max_errors) && (remainder_curr != 0)) {
        iteration += 1;
        uint8_t quotient = gf_div(remainder_prev, remainder_curr);

        uint8_t remainder_next = gf_add(remainder_prev, gf_mult(quotient, remainder_curr));
        uint8_t helper_s_next = gf_add(helper_s_prev, gf_mult(quotient, helper_s_curr));
        uint8_t helper_t_next = gf_add(helper_t_prev, gf_mult(quotient, helper_t_curr));

        remainder_prev = remainder_curr;
        remainder_curr = remainder_next;
        helper_s_prev = helper_s_curr;
        helper_s_curr = helper_s_next;
        helper_t_prev = helper_t_curr;
        helper_t_curr = helper_t_next;
    }

    euclideon_result result;
    result.error_evaluator_polynomial = remainder_curr;
    result.error_locator_polynomial = helper_t_curr;

    return result;
}

uint8_t poly_roots(uint8_t poly, uint8_t x) {
    uint8_t result = 0;
    uint8_t x_power = 1;
    
    for (int bit = 0; bit < 8; bit++) {
        if (poly & (1 << bit)) {
            result = gf_add(result, x_power);
        }
        x_power = gf_mult(x_power, x);
    }
    return result;
}

// --------------------------------------------------------------------------------
// @brief calculates error value vector from error positions, error evaluator
//        and error locator polynomial using D. Fornay's formula
// @input error_positions 
// @input error_evaluator_polynomial
// @input error_locator_polynomial
// @return error value vector
//
uint8_t* error_value(uint8_t error_positions[message_len], uint8_t error_evaluator_polynomial, uint8_t error_locator_polynomial) {
    
    uint8_t* error_values = malloc(sizeof(uint8_t)*message_len);
    for (int i = 0; i <= message_len; ++i) {
        uint8_t error_position = gf_pow(-i,-(2*max_errors+1));
        error_values[i] = gf_mult(gf_pow(-i,-(2*max_errors+1)), gf_div(error_evaluator_polynomial, gf_diff(error_locator_polynomial)));
    }

    return error_values;
}

uint8_t* resolve_errors(uint8_t* error_values, uint8_t* recieved_message) {
    uint8_t* decoded_message = malloc(sizeof(uint8_t)*message_len);
    for (int i = 0; i < message_len; ++i) {
        decoded_message[i] = recieved_message[i]- error_values[i];
    }
    return decoded_message;
}
