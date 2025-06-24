#include "galios.h"
#include "poly.h"
#include <stdint.h>
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

uint8_t *calculate_syndrome(uint8_t *received_poly, int syndrome_count, int codeword_length) {
    uint8_t *syndrome_output = malloc(sizeof(uint8_t) * syndrome_count);
    int errors_detected = 0;

    for (int i = 0; i < syndrome_count; i++) {
        uint8_t alpha_i = gf_pow(2, i + 1);
        uint8_t result = 0;
        uint8_t x_power = 1;

        for (int j = 0; j < codeword_length; j++) {
            result = gf_add(result, gf_mult(received_poly[j], x_power));
            x_power = gf_mult(x_power, alpha_i);
        }
        syndrome_output[i] = result;

        if (result != 0) {
            errors_detected = 1;
        }
    }

    return syndrome_output;
}

euclidean_result extended_euclidean_algorithm(uint8_t *syndrome_poly, int syndrome_len, int max_errors) {
    uint8_t *remainder_prev = malloc(syndrome_len * sizeof(uint8_t));
    uint8_t *remainder_curr = malloc(syndrome_len * sizeof(uint8_t));
    uint8_t *helper_t_prev = malloc(syndrome_len * sizeof(uint8_t));
    uint8_t *helper_t_curr = malloc(syndrome_len * sizeof(uint8_t));

    for (int i = 0; i < syndrome_len; i++) {
        remainder_prev[i] = syndrome_poly[i];
        remainder_curr[i] = 0;
        helper_t_prev[i] = 0;
        helper_t_curr[i] = 0;
    }
    remainder_curr[syndrome_len - 1] = 1;
    helper_t_curr[0] = 1;

    while ((poly_degree(remainder_curr, syndrome_len) >= max_errors) && (poly_degree(remainder_curr, syndrome_len) != -1)) {
        uint8_t *quotient = poly_div(remainder_prev, remainder_curr, syndrome_len);
        uint8_t *temp_mult = poly_mult(quotient, remainder_curr, syndrome_len);
        uint8_t *remainder_next = poly_add(remainder_prev, temp_mult, syndrome_len);
        free(temp_mult);

        temp_mult = poly_mult(quotient, helper_t_curr, syndrome_len);
        uint8_t *helper_t_next = poly_add(helper_t_prev, temp_mult, syndrome_len);
        free(temp_mult);

        free(remainder_prev);
        remainder_prev = remainder_curr;
        remainder_curr = remainder_next;

        free(helper_t_prev);
        helper_t_prev = helper_t_curr;
        helper_t_curr = helper_t_next;

        free(quotient);
    }

    euclidean_result result;
    result.error_evaluator_polynomial = remainder_curr;
    result.error_locator_polynomial = helper_t_curr;
    result.evaluator_len = syndrome_len;
    result.locator_len = syndrome_len;

    free(remainder_prev);
    free(helper_t_prev);
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
        error_values[i] = gf_mult(
            gf_pow(error_positions[i], 2 * max_errors + 1),
            gf_div(
                gf_poly_eval(error_evaluator_polynomial, error_evaluator_polynomial_len - 1, error_positions[i]),
                gf_poly_eval(gf_diff_result, error_locator_polynomial_len - 1, error_positions[i])));
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

uint8_t *decode_message(uint8_t *encoded_message, int message_len) {
    int max_errors = 3;
    int syndrome_len = max_errors * 2;

    uint8_t *syndrome_poly = calculate_syndrome(encoded_message, syndrome_len, message_len);

    euclidean_result euclid_output = extended_euclidean_algorithm(syndrome_poly, syndrome_len, max_errors);
    uint8_t *error_evaluator_polynomial = euclid_output.error_evaluator_polynomial;
    uint8_t *error_locator_polynomial = euclid_output.error_locator_polynomial;

    int num_roots = 0;
    int error_locator_polynomial_len = euclid_output.locator_len;
    uint8_t *error_positions = calculate_error_positions(error_locator_polynomial, error_locator_polynomial_len, &num_roots);

    int locator_len = euclid_output.locator_len;
    int evaluator_len = euclid_output.evaluator_len;
    uint8_t *error_values = calculate_error_values(error_positions, error_evaluator_polynomial, error_locator_polynomial, num_roots, locator_len, evaluator_len);

    uint8_t *error_vector = malloc(sizeof(uint8_t) * message_len);
    memset(error_vector, 0, sizeof(uint8_t) * message_len);

    for (int i = 0; i < num_roots; ++i) {
        error_vector[error_positions[i]] = error_values[i];
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
