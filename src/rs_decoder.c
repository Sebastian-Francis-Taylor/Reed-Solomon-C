#include "rs_decoder.h"
#include "galios.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int max_degree = 32;
uint8_t generator_polynomial;
uint8_t roots[255];

uint8_t *find_syndromes(uint8_t *received_poly, int max_errors, int codeword_length, uint8_t alpha) {
    int syndrome_count = 2 * max_errors;
    uint8_t *syndrome_output = malloc(sizeof(uint8_t) * syndrome_count);
    int errors_detected = 0;

    printf("Syndrome output=");
    for (int i = 0; i < syndrome_count; i++) {
        uint8_t alpha_i = gf_pow(alpha, i + 1);
        uint8_t result = received_poly[0];

        for (int j = 1; j < codeword_length; j++) {
            uint8_t term = gf_mult(result, alpha_i);
            result = gf_add(received_poly[j], term);
        }

        syndrome_output[i] = result;
        printf("%hhu ", syndrome_output[i]);

        if (result != 0) {
            errors_detected = 1;
        }
    }

    printf("\n");

    if (!errors_detected) {
        printf("No errors detected - all syndromes are zero\n");
        free(syndrome_output);
        return NULL;
    }

    return syndrome_output;
}

// extended_euclidean_algorithm uses little endian for poly nomials, returns little endian
euclidean_result extended_euclidean_algorithm(uint8_t *syndrome_poly, int syndrome_len, int max_errors) {

    printf("\n");
    printf("---- extended_euclidean_algorithm ----\n");
    printf("\n");

    int poly_size = syndrome_len + 1;
    uint8_t *initial_poly = calloc(poly_size, sizeof(uint8_t));
    initial_poly[syndrome_len] = 1;
    uint8_t *prev_remainder = initial_poly;
    uint8_t *prev_bezout_coeff = calloc(poly_size, sizeof(uint8_t));
    uint8_t *current_remainder = syndrome_poly;
    uint8_t *new_remainder;
    uint8_t *current_bezout_coeff = calloc(poly_size, sizeof(uint8_t));
    current_bezout_coeff[0] = 1;

    printf("Input poly (syndrome_poly): ");
    for (int i = 0; i < syndrome_len; ++i) {
        printf("%d ", syndrome_poly[i]);
    }
    printf("\n");

    printf("poly_size: %d, max_errors: %d\n", poly_size, max_errors);

    printf("initial_poly: ");
    for (int i = 0; i < poly_size; i++) printf("%02X ", initial_poly[i]);
    printf("\n");

    printf("Initial prev_remainder degree: %d\n", poly_degree_little_endian(prev_remainder, poly_size));
    printf("Initial current_remainder degree: %d\n", poly_degree_little_endian(current_remainder, poly_size));

    printf("\n");
    printf("---- ITERATION LOOP STARTING ----\n");
    printf("\n");

    int iteration = 1;

    while (poly_degree_little_endian(current_remainder, poly_size) > max_errors - 1) {
        //        reverse_array(current_remainder, poly_size);
        printf("Iteration %d: current_remainder (evaluator) degree = %d\n", iteration, poly_degree_little_endian(current_remainder, poly_size));
        printf("Iteration %d: current_bezout_coeff (locator) degree = %d\n", iteration, poly_degree_little_endian(current_bezout_coeff, poly_size));

        //       reverse_array(prev_remainder, poly_size);

        printf("current_remainder: ");
        for (int i = 0; i < 7 && i < poly_size; ++i) {
            printf("%d ", current_remainder[i]);
        }
        printf("\n");

        printf("prev_remainder: ");
        for (int i = 0; i < 7 && i < poly_size; ++i) {
            printf("%d ", prev_remainder[i]);
        }
        printf("\n");

        poly_div_result result = poly_div(prev_remainder, current_remainder, poly_size);
        uint8_t *quotient = result.quotient;
        new_remainder = result.remainder;

        printf("quotient: ");
        for (int i = 0; i < syndrome_len && i < poly_size; ++i) {
            printf("%d ", quotient[i]);
        }
        printf("\n");

        uint8_t *new_bezout_coeff = poly_add(prev_bezout_coeff, poly_mult(quotient, current_bezout_coeff, poly_size), poly_size);

        printf("After iteration: quotient = [");
        for (int i = 0; i < poly_size; i++) printf("%d ", quotient[i]);
        printf("]\n");

        printf("new_remainder = [");
        for (int i = 0; i < poly_size; i++) printf("%d ", new_remainder[i]);
        printf("]\n");

        printf("new_bezout_coeff = [");
        for (int i = 0; i < poly_size; i++) printf("%d ", new_bezout_coeff[i]);
        printf("]\n\n");

        prev_bezout_coeff = current_bezout_coeff;
        current_bezout_coeff = new_bezout_coeff;
        prev_remainder = current_remainder;
        current_remainder = new_remainder;
        iteration++;
        ////      reverse_array(current_remainder, poly_size);
    }

    for (int i = 0; i < poly_size; ++i) {
    }
    printf("Final current_remainder (error_evaluator) degree: %d\n", poly_degree_little_endian(current_remainder, poly_size));
    printf("Final current_bezout_coeff (error_locator) degree: %d\n", poly_degree_little_endian(current_bezout_coeff, poly_size));

    euclidean_result result;
    result.error_locator_polynomial = current_bezout_coeff;
    result.error_evaluator_polynomial = current_remainder;
    result.locator_len = poly_size;
    result.evaluator_len = poly_size;
    printf("\n=== FINAL EUCLIDEAN RESULTS ===\n");
    printf("Error Locator Polynomial (current_bezout_coeff):\n");
    printf("  Degree: %d\n", poly_degree_little_endian(current_bezout_coeff, poly_size));
    printf("  Coefficients: ");
    for (int i = 0; i < poly_size; i++) {
        printf("%d ", current_bezout_coeff[i]);
    }
    printf("\n");

    printf("Error Evaluator Polynomial (new_remainder):\n");
    printf("  Degree: %d\n", poly_degree_little_endian(new_remainder, poly_size));
    printf("  Coefficients: ");
    for (int i = 0; i < poly_size; i++) {
        printf("%d ", new_remainder[i]);
    }
    printf("\n");

    printf("Polynomial sizes: poly_size=%d, syndrome_len=%d, max_errors=%d\n",
           poly_size, syndrome_len, max_errors);
    printf("================================\n\n");

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
// mixed endian
uint8_t *calculate_error_values(uint8_t *error_positions, uint8_t *error_evaluator_polynomial, uint8_t *error_locator_polynomial, int error_amount, int error_locator_polynomial_len, int error_evaluator_polynomial_len) {

    // Convert error_evaluator to big-endian once
    reverse_array(error_evaluator_polynomial, error_evaluator_polynomial_len);

    uint8_t *error_values = malloc(sizeof(uint8_t) * error_amount);
    for (int i = 0; i < error_amount; ++i) {
        // Convert error_locator to little-endian for gf_diff
        reverse_array(error_locator_polynomial, error_locator_polynomial_len);
        uint8_t *gf_diff_result = gf_diff(error_locator_polynomial, error_locator_polynomial_len);
        // Convert back to big-endian
        reverse_array(error_locator_polynomial, error_locator_polynomial_len);

        // Convert derivative to big-endian
        reverse_array(gf_diff_result, error_locator_polynomial_len - 1);

        int diff_degree = poly_degree_big_endian(gf_diff_result, error_locator_polynomial_len - 1);
        int eval_degree = poly_degree_big_endian(error_evaluator_polynomial, error_evaluator_polynomial_len);

        uint8_t eval_num = gf_poly_eval(error_evaluator_polynomial, eval_degree, error_positions[i], error_evaluator_polynomial_len);
        uint8_t eval_den = gf_poly_eval(gf_diff_result, diff_degree, error_positions[i], error_locator_polynomial_len - 1);

        // 3 is temporary
        error_values[i] = gf_mult(gf_pow(gf_inv(error_positions[i]), 2 * error_amount + 1), gf_div(eval_num, eval_den));
        free(gf_diff_result);
    }

    // Restore error_evaluator to original endianness
    reverse_array(error_evaluator_polynomial, error_evaluator_polynomial_len);
    return error_values;
}

// big-endian
uint8_t *calculate_error_positions(uint8_t *poly, int poly_len, int *num_positions) {
    // DEBUG PRINT
    printf("error locator polynomial (before roots are found)");
    for (int i = 0; i < poly_len; ++i) {
        printf("%d ", poly[i]);
    }
    printf("\n");
    int input_poly_degree = poly_degree_big_endian(poly, poly_len);
    printf("error locator poly degree (before find roots): %d \n", input_poly_degree);
    uint8_t *error_positions = gf_find_roots(poly, input_poly_degree, num_positions, poly_len);
    return error_positions;
}

uint8_t *resolve_errors(uint8_t *error_vector, uint8_t *received_message, int message_len) {
    uint8_t *decoded_message = malloc(sizeof(uint8_t) * message_len);
    memcpy(decoded_message, received_message, message_len);

    for (int i = 0; i < message_len; i++) {
        decoded_message[i] = gf_add(decoded_message[i], error_vector[i]);
    }

    return decoded_message;
}

uint8_t *decode_message(uint8_t *encoded_message, int message_len, int max_errors) {
    int syndrome_len = max_errors * 2;

    uint8_t alpha = 2;
    uint8_t *syndrome_poly = find_syndromes(encoded_message, syndrome_len / 2, message_len, alpha);

    reverse_array(syndrome_poly, syndrome_len);

    euclidean_result euclid_output = extended_euclidean_algorithm(syndrome_poly, syndrome_len, max_errors);
    uint8_t *error_evaluator_polynomial = euclid_output.error_evaluator_polynomial;
    uint8_t *error_locator_polynomial = euclid_output.error_locator_polynomial;
    int locator_len = euclid_output.locator_len;
    int evaluator_len = euclid_output.evaluator_len;

    printf("max_errors passed to euclidean: %d\n", max_errors);
    printf("syndrome_len: %d\n", syndrome_len);
    printf("Error locator degree after euclidean: %d\n", poly_degree_big_endian(error_locator_polynomial, locator_len));

    reverse_array(error_locator_polynomial, locator_len);
    int num_roots = 0;
    int error_locator_polynomial_len = euclid_output.locator_len;
    uint8_t *error_positions = calculate_error_positions(error_locator_polynomial, error_locator_polynomial_len, &num_roots);

    // Filter out root 0 before calculating error values
    int valid_roots = 0;
    uint8_t *valid_positions = malloc(num_roots * sizeof(uint8_t));
    for (int i = 0; i < num_roots; i++) {
        if (error_positions[i] != 0) {
            valid_positions[valid_roots++] = error_positions[i];
        }
    }

    uint8_t *error_values = calculate_error_values(valid_positions, error_evaluator_polynomial, error_locator_polynomial, valid_roots, locator_len, evaluator_len);

    uint8_t *error_vector = malloc(sizeof(uint8_t) * message_len);
    memset(error_vector, 0, sizeof(uint8_t) * message_len);

    // DEBUG PRINTING
    printf("Found %d roots (%d valid):\n", num_roots, valid_roots);
    for (int i = 0; i < valid_roots; ++i) {

        uint8_t root = valid_positions[i];
        int position = (254 - global_tables.log_table[root]) % message_len;

        printf("Root %d: root=%d, log_table[root]=%d, position=%d\n",
               i, root, global_tables.log_table[root], position);

        printf("Valid root %d: error_pos=%d, position=%d, error_val=%d\n",
               i, valid_positions[i], position, error_values[i]);

        if (position >= 0 && position < message_len) {
            error_vector[position] = error_values[i];
        }
    }

    uint8_t *decoded_message = resolve_errors(error_vector, encoded_message, message_len);

    free(syndrome_poly);
    free(error_positions);
    free(valid_positions);
    free(error_values);
    free(error_vector);
    free(error_evaluator_polynomial);
    free(error_locator_polynomial);

    return decoded_message;
}
