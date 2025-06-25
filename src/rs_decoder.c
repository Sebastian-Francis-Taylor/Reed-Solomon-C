#include "galios.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int max_degree = 32;
int max_errors = 8;
uint8_t message[128];
int message_len = 128;

uint8_t generator_polynomial;
uint8_t roots[255];

typedef struct {
    uint8_t *error_evaluator_polynomial;
    uint8_t *error_locator_polynomial;
    int evaluator_len;
    int locator_len;

} euclidean_result;

uint8_t *find_syndromes(uint8_t *received_poly, int T, int codeword_length, uint8_t alpha) {
    int syndrome_count = 2 * T;
    uint8_t *syndrome_output = malloc(sizeof(uint8_t) * syndrome_count);
    int errors_detected = 0;


    for (int i = 0; i < syndrome_count; i++) {
        uint8_t alpha_i = gf_pow(alpha, i + 1);
        uint8_t result = 0;

        for (int j = 0; j < codeword_length; j++) {
            uint8_t term = gf_mult(received_poly[j], gf_pow(alpha_i, j));
            result = gf_add(result, term);
        }

        syndrome_output[i] = result;

        if (result != 0) {
            errors_detected = 1;
        }
    }

    if (!errors_detected) {
        printf("No errors detected - all syndromes are zero\n");
        free(syndrome_output);
        return NULL;
    }

    return syndrome_output;
}

euclidean_result extended_euclidean_algorithm(uint8_t *syndrome_poly, int syndrome_len, int max_errors) {
    // Initialize polynomials
    uint8_t *r_prev = calloc(syndrome_len, sizeof(uint8_t));
    uint8_t *r_curr = calloc(syndrome_len, sizeof(uint8_t));
    uint8_t *s_prev = calloc(syndrome_len, sizeof(uint8_t));
    uint8_t *s_curr = calloc(syndrome_len, sizeof(uint8_t));

    // r_prev = syndrome polynomial
    memcpy(r_prev, syndrome_poly, syndrome_len * sizeof(uint8_t));

    // r_curr = x^(2*max_errors) - CRITICAL: check syndrome_len is large enough!
    if (syndrome_len >= 2 * max_errors) {
        r_curr[(2 * max_errors) - 1] = 1;
    } else {
        // Error: syndrome_len too small
        printf("Error: syndrome_len (%d) must be > 2*max_errors (%d)\n", syndrome_len, 2 * max_errors);
    }

    // s_prev = 0 (already zeroed)
    // s_curr = 1
    s_curr[0] = 1;

    printf("Initial r_prev degree: %d\n", poly_degree(r_prev, syndrome_len));
    printf("Initial r_curr degree: %d\n", poly_degree(r_curr, syndrome_len));

    // Extended Euclidean Algorithm
    while (poly_degree(r_curr, syndrome_len) >= max_errors) {
        printf("Iteration: r_curr (evaluator) degree = %d\n", poly_degree(r_curr, syndrome_len));
        printf("Iteration: s_curr (locator) degree = %d\n", poly_degree(s_curr, syndrome_len));

        // q = r_prev / r_curr
        uint8_t *quotient = poly_div(r_prev, r_curr, syndrome_len);

        // r_next = r_prev - q * r_curr
        uint8_t *temp = poly_mult(quotient, r_curr, syndrome_len);
        uint8_t *r_next = poly_add(r_prev, temp, syndrome_len);
        free(temp);

        // s_next = s_prev - q * s_curr
        temp = poly_mult(quotient, s_curr, syndrome_len);
        uint8_t *s_next = poly_add(s_prev, temp, syndrome_len);
        free(temp);

        // Shift for next iteration
        free(r_prev);
        r_prev = r_curr;
        r_curr = r_next;

        free(s_prev);
        s_prev = s_curr;
        s_curr = s_next;

        free(quotient);
    }

    euclidean_result result;
    result.error_locator_polynomial = s_curr;
    result.error_evaluator_polynomial = r_curr;
    result.locator_len = syndrome_len;
    result.evaluator_len = syndrome_len;

    free(r_prev);
    free(s_prev);

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
uint8_t *calculate_error_values(uint8_t *error_positions, uint8_t *error_evaluator_polynomial, uint8_t *error_locator_polynomial, int error_amount, int error_locator_polynomial_len, int error_evaluator_polynomial_len) {

    uint8_t *error_values = malloc(sizeof(uint8_t) * error_amount);
    for (int i = 0; i < error_amount; ++i) {
        uint8_t *gf_diff_result = gf_diff(error_locator_polynomial, error_locator_polynomial_len);
        int diff_degree = poly_degree(gf_diff_result, error_locator_polynomial_len - 1);

        uint8_t eval_num = gf_poly_eval(error_evaluator_polynomial, error_evaluator_polynomial_len - 1, error_positions[i]);
        uint8_t eval_den = gf_poly_eval(gf_diff_result, diff_degree, error_positions[i]);

        // DEBUG PRINT
        // printf("Root %02X: evaluator=%02X, derivative=%02X\n", error_positions[i], eval_num, eval_den);

        error_values[i] = gf_div(eval_num, eval_den);
        free(gf_diff_result);
    }
    return error_values;
}

uint8_t *calculate_error_positions(uint8_t *poly, int poly_len, int *num_positions) {
    int input_poly_degree = poly_degree(poly, poly_len);
    uint8_t *error_positions = gf_find_roots(poly, input_poly_degree, num_positions);
    return error_positions;
}

uint8_t *resolve_errors(uint8_t *error_values, uint8_t *received_message) {
    uint8_t *decoded_message = poly_add(received_message, error_values, message_len);
    return decoded_message;
}

uint8_t *decode_message(uint8_t *encoded_message, int message_len, int max_errors) {
    int syndrome_len = max_errors * 2;

    uint8_t alpha = 2;
    uint8_t *syndrome_poly = find_syndromes(encoded_message, syndrome_len / 2, message_len, alpha);

    euclidean_result euclid_output = extended_euclidean_algorithm(syndrome_poly, syndrome_len, max_errors);
    uint8_t *error_evaluator_polynomial = euclid_output.error_evaluator_polynomial;
    uint8_t *error_locator_polynomial = euclid_output.error_locator_polynomial;
    int locator_len = euclid_output.locator_len;
    int evaluator_len = euclid_output.evaluator_len;

    printf("max_errors passed to euclidean: %d\n", max_errors);
    printf("syndrome_len: %d\n", syndrome_len);
    printf("Error locator degree after euclidean: %d\n", poly_degree(error_locator_polynomial, locator_len));

    int num_roots = 0;
    int error_locator_polynomial_len = euclid_output.locator_len;
    uint8_t *error_positions = calculate_error_positions(error_locator_polynomial, error_locator_polynomial_len, &num_roots);

    for (int i = 0; i < num_roots; ++i) {
        int position = global_tables.log_table[gf_inverse(error_positions[i])];
        position = position % message_len;
    }

    uint8_t *error_values = calculate_error_values(error_positions, error_evaluator_polynomial, error_locator_polynomial, num_roots, locator_len, evaluator_len);

    uint8_t *error_vector = malloc(sizeof(uint8_t) * message_len);
    memset(error_vector, 0, sizeof(uint8_t) * message_len);

    for (int i = 0; i < num_roots; ++i) {
        int raw_position = global_tables.log_table[gf_inverse(error_positions[i])] % message_len;
        int position = (raw_position < 8) ? raw_position : raw_position - 8;

        if (position >= 0 && position < message_len) {
            error_vector[position] = error_values[i];
        }
    }

    uint8_t *decoded_message = resolve_errors(error_vector, encoded_message);

    free(syndrome_poly);
    free(error_positions);
    free(error_values);
    free(error_vector);
    free(error_evaluator_polynomial);
    free(error_locator_polynomial);

    return decoded_message;
}
