#include "rs_decoder.h"
#include "galios.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Calculates the syndrome polynomial using equation 11.1 from referenced
 *      book
 * @param received_poly Polynomial representing the received encoded message
 * @param max_errors The maximum amount of errors the program should correct for
 * @param codeword_length Length of received_poly
 * @param alpha Primitive element in GF(256)
 * @return Syndrome polynomial for the received encoded message, or NULL if no
 *      errors are detected
 */
uint8_t *find_syndromes(uint8_t *received_poly, int max_errors,
                        int codeword_length, uint8_t alpha) {
    int syndrome_count = 2 * max_errors;
    uint8_t *syndrome_output = malloc(sizeof(uint8_t) * syndrome_count);
    int errors_detected = 0;

    for (int i = 0; i < syndrome_count; i++) {
        uint8_t alpha_i = gf_pow(alpha, i + 1);
        uint8_t result = received_poly[0];

        for (int j = 1; j < codeword_length; j++) {
            uint8_t term = gf_mult(result, alpha_i);
            result = gf_add(received_poly[j], term);
        }

        syndrome_output[syndrome_count - 1 - i] = result;

        if (result != 0) {
            errors_detected = 1;
        }
    }

    if (!errors_detected) {
        free(syndrome_output);
        return NULL;
    }

    return syndrome_output;
}

/**
 * @brief Performs the extended euclidean algorithm to calculate the error value
 *      & error locator polynomials
 * @param syndrome_poly The syndrome polynomial for the encoded message
 * @param syndrome_len Length of the syndrome polynomial array
 * @param max_errors The maximum amount of errors the program should correct for
 * @return struct containing the error value & error locator polynomials
 *      including their respective lengths
 */
euclidean_result extended_euclidean_algorithm(uint8_t *syndrome_poly,
                                              int syndrome_len,
                                              int max_errors) {
    int poly_size = syndrome_len + 1;
    uint8_t *initial_poly = calloc(poly_size, sizeof(uint8_t));
    initial_poly[syndrome_len] = 1;
    uint8_t *prev_remainder = initial_poly;
    uint8_t *prev_bezout_coeff = calloc(poly_size, sizeof(uint8_t));
    uint8_t *current_remainder = syndrome_poly;
    uint8_t *new_remainder;
    uint8_t *current_bezout_coeff = calloc(poly_size, sizeof(uint8_t));
    current_bezout_coeff[0] = 1;

    int iteration = 1;
    while (poly_degree(current_remainder, poly_size) > max_errors - 1) {
        poly_div_result result =
            poly_div(prev_remainder, current_remainder, poly_size);
        uint8_t *quotient = result.quotient;
        new_remainder = result.remainder;

        uint8_t *new_bezout_coeff = poly_add(
            prev_bezout_coeff,
            poly_mult(quotient, current_bezout_coeff, poly_size), poly_size);

        prev_bezout_coeff = current_bezout_coeff;
        current_bezout_coeff = new_bezout_coeff;
        prev_remainder = current_remainder;
        current_remainder = new_remainder;
        iteration++;
    }

    euclidean_result result;
    result.error_locator_polynomial = current_bezout_coeff;
    result.error_evaluator_polynomial = current_remainder;
    result.locator_len = poly_size;
    result.evaluator_len = poly_size;

    return result;
}

/*
 * @brief Calculates the error values using thm 11.2.2 in the referenced book
 * @param error_positions Array containing the error positions
 * @param error_evaluator_polynomial Polynomial from Euclidean algorithm for
 *      calculating error values
 * @param error_locator_polynomial Polynomial from Euclidean algorithm for
 *      calculating error positions
 * @param error_amount Amount of errors detected
 * @param error_locator_polynomial_len Length of the error locator polynomial
 * @param error_evaluator_polynomial_len Length of the error evaluator
 *      polynomial
 * @return Array with the error values
 */
uint8_t *calculate_error_values(uint8_t *error_positions,
                                uint8_t *error_evaluator_polynomial,
                                uint8_t *error_locator_polynomial,
                                int error_amount,
                                int error_locator_polynomial_len,
                                int error_evaluator_polynomial_len) {
    uint8_t *error_values = malloc(sizeof(uint8_t) * error_amount);
    for (int i = 0; i < error_amount; ++i) {
        uint8_t *gf_diff_result =
            gf_diff(error_locator_polynomial, error_locator_polynomial_len);

        int diff_degree =
            poly_degree(gf_diff_result, error_locator_polynomial_len - 1);
        int eval_degree = poly_degree(error_evaluator_polynomial,
                                      error_evaluator_polynomial_len);

        uint8_t eval_num =
            gf_poly_eval(error_evaluator_polynomial, eval_degree,
                         error_positions[i], error_evaluator_polynomial_len);
        uint8_t eval_den =
            gf_poly_eval(gf_diff_result, diff_degree, error_positions[i],
                         error_locator_polynomial_len - 1);

        error_values[i] =
            gf_mult(gf_pow(gf_inv(error_positions[i]), 2 * error_amount + 1),
                    gf_div(eval_num, eval_den));
        free(gf_diff_result);
    }

    return error_values;
}

/**
 * @brief Calculates the error positions by finding roots of the error locator
 *      polynomial
 * @param poly Error locator polynomial coefficients
 * @param poly_len Length of the error locator polynomial array
 * @param num_positions Output parameter for number of error positions found
 * @return Array of error position roots, or NULL if no roots found
 */
uint8_t *calculate_error_positions(uint8_t *poly, int poly_len,
                                   int *num_positions) {
    int input_poly_degree = poly_degree(poly, poly_len);
    uint8_t *error_positions =
        gf_find_roots(poly, input_poly_degree, num_positions, poly_len);
    return error_positions;
}

/**
 * @brief Corrects errors in received message by adding error values at their
 *      positions
 * @param error_vector Array with error values at their corresponding error
 *      positions
 * @param received_message Array containing the received message with errors
 * @param message_len Length of received_message array
 * @return Corrected decoded message
 */
uint8_t *resolve_errors(uint8_t *error_vector, uint8_t *received_message,
                        int message_len) {
    uint8_t *decoded_message = malloc(sizeof(uint8_t) * message_len);
    memcpy(decoded_message, received_message, message_len);

    for (int i = 0; i < message_len; i++) {
        decoded_message[i] = gf_add(decoded_message[i], error_vector[i]);
    }

    return decoded_message;
}

/**
 * @brief Main Reed-Solomon decoding function that corrects errors in received
 *      message
 * @param encoded_message Received message potentially containing errors
 * @param message_len Length of the message
 * @param max_errors Maximum number of errors to correct (up to 16)
 * @return Decoded message with errors corrected
 */
uint8_t *decode_message(uint8_t *encoded_message, int message_len,
                        int max_errors) {
    int syndrome_len = max_errors * 2;
    uint8_t alpha = 2;

    uint8_t *syndrome_poly =
        find_syndromes(encoded_message, syndrome_len / 2, message_len, alpha);
    if (!syndrome_poly) {
        uint8_t *clean_message = malloc(message_len * sizeof(uint8_t));
        memcpy(clean_message, encoded_message, message_len);
        return clean_message;
    }

    euclidean_result euclid_output =
        extended_euclidean_algorithm(syndrome_poly, syndrome_len, max_errors);
    uint8_t *error_evaluator_polynomial =
        euclid_output.error_evaluator_polynomial;
    uint8_t *error_locator_polynomial = euclid_output.error_locator_polynomial;
    int locator_len = euclid_output.locator_len;
    int evaluator_len = euclid_output.evaluator_len;

    int num_roots = 0;
    uint8_t *error_positions = calculate_error_positions(
        error_locator_polynomial, locator_len, &num_roots);

    uint8_t *error_values = calculate_error_values(
        error_positions, error_evaluator_polynomial, error_locator_polynomial,
        num_roots, locator_len, evaluator_len);

    uint8_t *error_vector = malloc(sizeof(uint8_t) * message_len);
    memset(error_vector, 0, sizeof(uint8_t) * message_len);

    for (int i = 0; i < num_roots; ++i) {
        uint8_t root = error_positions[i];
        int position = (254 - global_tables.log_table[root]) % message_len;

        if (position >= 0 && position < message_len) {
            error_vector[position] = error_values[i];
        }
    }

    uint8_t *decoded_message =
        resolve_errors(error_vector, encoded_message, message_len);

    free(syndrome_poly);
    free(error_positions);
    free(error_values);
    free(error_vector);
    free(error_evaluator_polynomial);
    free(error_locator_polynomial);

    return decoded_message;
}
