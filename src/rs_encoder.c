#include <stdint.h>
#include <stdio.h>
#include "galios.h"

#define max_degree 32
#define max_errors 8

uint8_t generator_polynomial;

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

euclideon_result euclideon_algorithm(uint8_t a, uint8_t b) {
    
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
