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

    printf("Calculating %d syndromes...\n", syndrome_count);
    
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

    printf("Syndromes: ");
    for (int i = 0; i < syndrome_count; i++) {
        printf("%d ", syndrome_output[i]);
    }
    printf("\n");

    if (!errors_detected) {
        printf("No errors detected - all syndromes are zero\n");
        free(syndrome_output);
        return NULL;
    }

    return syndrome_output;
}

euclidean_result extended_euclidean_algorithm(uint8_t *syndrome_poly, int syndrome_len, int max_errors) {
    printf("\n--- Extended Euclidean Algorithm ---\n");
    printf("Parameters: syndrome_len=%d, max_errors=%d\n", syndrome_len, max_errors);

    int poly_size = syndrome_len + 1;
    uint8_t *initial_poly = calloc(poly_size, sizeof(uint8_t));
    initial_poly[syndrome_len] = 1;
    uint8_t *prev_remainder = initial_poly;
    uint8_t *prev_bezout_coeff = calloc(poly_size, sizeof(uint8_t));
    uint8_t *current_remainder = syndrome_poly;
    uint8_t *new_remainder;
    uint8_t *current_bezout_coeff = calloc(poly_size, sizeof(uint8_t));
    current_bezout_coeff[0] = 1;

    printf("Initial degrees: r_prev=%d, r_curr=%d\n", 
           poly_degree(prev_remainder, poly_size), 
           poly_degree(current_remainder, poly_size));

    int iteration = 1;
    while (poly_degree(current_remainder, poly_size) > max_errors - 1) {
        printf("Iteration %d: eval_deg=%d, loc_deg=%d\n", 
               iteration,
               poly_degree(current_remainder, poly_size),
               poly_degree(current_bezout_coeff, poly_size));

        poly_div_result result = poly_div(prev_remainder, current_remainder, poly_size);
        uint8_t *quotient = result.quotient;
        new_remainder = result.remainder;

        uint8_t *new_bezout_coeff = poly_add(prev_bezout_coeff, poly_mult(quotient, current_bezout_coeff, poly_size), poly_size);

        prev_bezout_coeff = current_bezout_coeff;
        current_bezout_coeff = new_bezout_coeff;
        prev_remainder = current_remainder;
        current_remainder = new_remainder;
        iteration++;
    }

    printf("\nFinal Results:\n");
    printf("Error Locator Polynomial (degree %d): ", poly_degree(current_bezout_coeff, poly_size));
    for (int i = 0; i < 10 && i < poly_size; i++) {
        printf("%d ", current_bezout_coeff[i]);
    }
    if (poly_size > 10) printf("...");
    printf("\n");
    
    printf("Error Evaluator Polynomial (degree %d): ", poly_degree(current_remainder, poly_size));
    for (int i = 0; i < 10 && i < poly_size; i++) {
        printf("%d ", current_remainder[i]);
    }
    if (poly_size > 10) printf("...");
    printf("\n");

    euclidean_result result;
    result.error_locator_polynomial = current_bezout_coeff;
    result.error_evaluator_polynomial = current_remainder;
    result.locator_len = poly_size;
    result.evaluator_len = poly_size;

    return result;
}

uint8_t *calculate_error_values(uint8_t *error_positions, uint8_t *error_evaluator_polynomial, uint8_t *error_locator_polynomial, int error_amount, int error_locator_polynomial_len, int error_evaluator_polynomial_len) {
    uint8_t *error_values = malloc(sizeof(uint8_t) * error_amount);
    for (int i = 0; i < error_amount; ++i) {
        uint8_t *gf_diff_result = gf_diff(error_locator_polynomial, error_locator_polynomial_len);

        int diff_degree = poly_degree(gf_diff_result, error_locator_polynomial_len - 1);
        int eval_degree = poly_degree(error_evaluator_polynomial, error_evaluator_polynomial_len);

        uint8_t eval_num = gf_poly_eval(error_evaluator_polynomial, eval_degree, error_positions[i], error_evaluator_polynomial_len);
        uint8_t eval_den = gf_poly_eval(gf_diff_result, diff_degree, error_positions[i], error_locator_polynomial_len - 1);

        error_values[i] = gf_mult(gf_pow(gf_inv(error_positions[i]), 2 * error_amount + 1), gf_div(eval_num, eval_den));
        free(gf_diff_result);
    }

    return error_values;
}

uint8_t *calculate_error_positions(uint8_t *poly, int poly_len, int *num_positions) {
    int input_poly_degree = poly_degree(poly, poly_len);
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
    printf("Reed-Solomon Decoder - Message Length: %d, Max Errors: %d\n", message_len, max_errors);
    
    int syndrome_len = max_errors * 2;
    uint8_t alpha = 2;
    
    uint8_t *syndrome_poly = find_syndromes(encoded_message, syndrome_len / 2, message_len, alpha);
    if (!syndrome_poly) {
        printf("Message is error-free\n");
        uint8_t *clean_message = malloc(message_len * sizeof(uint8_t));
        memcpy(clean_message, encoded_message, message_len);
        return clean_message;
    }

    euclidean_result euclid_output = extended_euclidean_algorithm(syndrome_poly, syndrome_len, max_errors);
    uint8_t *error_evaluator_polynomial = euclid_output.error_evaluator_polynomial;
    uint8_t *error_locator_polynomial = euclid_output.error_locator_polynomial;
    int locator_len = euclid_output.locator_len;
    int evaluator_len = euclid_output.evaluator_len;

    printf("\nFinding error positions...\n");
    int num_roots = 0;
    uint8_t *error_positions = calculate_error_positions(error_locator_polynomial, locator_len, &num_roots);
    printf("Found %d error roots\n", num_roots);

    uint8_t *error_values = calculate_error_values(error_positions, error_evaluator_polynomial, error_locator_polynomial, num_roots, locator_len, evaluator_len);

    uint8_t *error_vector = malloc(sizeof(uint8_t) * message_len);
    memset(error_vector, 0, sizeof(uint8_t) * message_len);

    printf("\nError Correction Summary:\n");
    printf("Index | Root | Log  | Position | Error Value\n");
    printf("------|------|------|----------|------------\n");

    for (int i = 0; i < num_roots; ++i) {
        uint8_t root = error_positions[i];
        int position = (254 - global_tables.log_table[root]) % message_len;
        
        printf("%-5d | %-4d | %-4d | %-8d | %-11d\n",
               i, root, global_tables.log_table[root], position, error_values[i]);

        if (position >= 0 && position < message_len) {
            error_vector[position] = error_values[i];
        }
    }

    uint8_t *decoded_message = resolve_errors(error_vector, encoded_message, message_len);

    printf("\nDecoding complete - corrected %d errors\n", num_roots);

    free(syndrome_poly);
    free(error_positions);
    free(error_values);
    free(error_vector);
    free(error_evaluator_polynomial);
    free(error_locator_polynomial);

    return decoded_message;
}
