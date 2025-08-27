#include "galois.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define gf_bits 8

log_tables global_tables;

/**
 * @brief Creates log and antilog tables for the galois field GF(256) using primitive polynomial
 * @return Returns struct log_tables with log and antilog tables
 */
log_tables init_gf_tables() {
    uint8_t a = 1;
    uint8_t *log_table = malloc(sizeof(uint8_t) * 256);
    uint8_t *antilog_table = malloc(sizeof(uint8_t) * 256);

    for (uint8_t i = 0; i < 255; ++i) {
        antilog_table[i] = a;
        log_table[a] = i;

        uint8_t hi_bit = a & 0x80;
        a <<= 1;
        if (hi_bit) {
            a ^= 0x1D;
        }
    }

    log_table[0] = 0;

    log_tables result;
    result.log_table = log_table;
    result.antilog_table = antilog_table;

    return result;
}

/**
 * @brief Initialises log and antilog tables
 */
void initialise_gf() {
    global_tables = init_gf_tables();
}

/**
 * @brief adds two numbers together in Galois field using bitwise XOR
 * @param a First element
 * @param b Second element
 * @return a and b added together within galois field
 */
uint8_t gf_add(uint8_t a, uint8_t b) {
    return a ^ b;
}

/**
 * @brief multiplies two numbers together in GF(256) using logarithm addition
 * @param a First element
 * @param b Second element
 * @return a and b multiplied together in GF(256)
 */
uint8_t gf_mult(uint8_t a, uint8_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0;
    uint8_t log_result = (global_tables.log_table[a] + global_tables.log_table[b]) % 255;

    return global_tables.antilog_table[log_result];
}

/**
 * @brief Divides two numbers in GF(256) using logarithm subtraction
 * @param a First element
 * @param b Second element
 * @return a divided by b in GF(256) or 0 if either parater is 0
 */
uint8_t gf_div(uint8_t a, uint8_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0;
    uint8_t log_result = (global_tables.log_table[a] - global_tables.log_table[b] + 255) % 255;

    return global_tables.antilog_table[log_result];
}

/**
 * @brief Calculates power of a number in GF(256) using log and antilog tables
 * @param base Base element
 * @param exponent Exponent value
 * @return base to the power of exponent, 1 if exponent is 0 and 0 if base is 0
 */
uint8_t gf_pow(uint8_t base, uint8_t exponent) {
    if (exponent == 0) return 1;
    if (base == 0) return 0;

    return global_tables.antilog_table[(global_tables.log_table[base] * exponent) % 255];
}

/**
 * @brief Calculates multiplicative inverse in GF(256)
 * @param x Element to invert
 * @return multiplicative inverse of x, or 0 if x is 0
 */
uint8_t gf_inv(uint8_t x) {
    if (x == 0) return 0;

    return gf_pow(x, 254);
}

/**
 * @brief Calculates the derivative of a polynomial in GF(256)
 * @param poly Polynomial coefficients in little-endian format
 * @param poly_len Length of the polynomial
 * @return Differentiated polynomial with length poly_len-1
 */
uint8_t *gf_diff(uint8_t *poly, int poly_len) {
    if (poly_len <= 1) {
        uint8_t *result = calloc(1, sizeof(uint8_t));
        return result;
    }

    uint8_t *poly_diff = calloc(poly_len - 1, sizeof(uint8_t));

    for (int i = 1; i < poly_len; i++) {
        if (i % 2 == 1) {
            poly_diff[i - 1] = poly[i];
        }
    }

    return poly_diff;
}

/**
 * @brief Evaluates polynomial at point x in GF(256) using Horner's method
 * @param poly Polynomial coefficients in little-endian format
 * @param degree Degree of the polynomial
 * @param x Point to evaluate at
 * @param len Length of polynomial array
 * @return Value of polynomial evaluated at x
 */
uint8_t gf_poly_eval(const uint8_t *poly, int degree, uint8_t x, int len) {
    uint8_t result = 0;
    for (int i = degree; i >= 0; i--) {
        result = gf_mult(result, x);
        result = gf_add(result, poly[i]);
    }
    return result;
}

/**
 * @brief Calculates roots of polynomial in GF(256) by brute force approach
 * @param poly Polynomial coefficients in little-endian format
 * @param degree Degree of polynomial
 * @param out_num_roots Output parameter for number of roots found
 * @param poly_len Length of polynomial array
 * @return Array of roots found, or NULL if no roots exist
 */
uint8_t *gf_find_roots(const uint8_t *poly, int degree, int *out_num_roots, int poly_len) {
    uint8_t *roots = malloc(256 * sizeof(uint8_t));

    int num_roots = 0;
    for (uint16_t x = 0; x < 256; x++) {
        if (gf_poly_eval(poly, degree, (uint8_t)x, poly_len) == 0) {
            roots[num_roots++] = (uint8_t)x;
        }
    }

    if (num_roots > 0) {
        uint8_t *resized = realloc(roots, num_roots * sizeof(uint8_t));
        if (resized) roots = resized;
    } else {
        free(roots);
        roots = NULL;
    }

    *out_num_roots = num_roots;
    return roots;
}

/**
 * @brief Calculates degree of polynomial
 * @param poly Polynomial coefficients in little-endian format
 * @param poly_len Length of polynomial
 * return Index of last exponent or -1 if the polynomial is zero
 */
int poly_degree(uint8_t *poly, int len) {
    for (int i = len - 1; i >= 0; i--) {
        if (poly[i] != 0) return i;
    }
    return -1;
}

/**
 * @brief Sum of two arrays in Galois field
 * @param a Polynomial in little-endian format
 * @param b Polynomial in little-endian format
 * @param len Length of the two polynomials, both polynomials must be of same length
 * @return The sum of the polynomials a and b
 */
uint8_t *poly_add(uint8_t *a, uint8_t *b, int len) {
    uint8_t *result = malloc(len * sizeof(uint8_t));
    for (int i = 0; i < len; i++) {
        result[i] = gf_add(a[i], b[i]);
    }
    return result;
}

/**
 * @brief multiplies two polynomials together using gf_mult and gf_add
 * @param a Polynomial in little-endian format
 * @param b Polynomial in little-endian format
 * @param len Length of the two polynomials, both polynomials must be of same length
 * @return The two polynomials multiplied together
 */
uint8_t *poly_mult(uint8_t *a, uint8_t *b, int len) {
    uint8_t *result = calloc(len, sizeof(uint8_t));
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            if (i + j < len) {
                uint8_t term = gf_mult(a[i], b[j]);
                result[i + j] = gf_add(result[i + j], term);
            }
        }
    }
    return result;
}

/**
 * @brief Performs polynomial long division in GF(256)
 * @param dividend Dividend polynomial coefficients in little-endian format
 * @param divisor Divisor polynomial coefficients in little-endian format
 * @param len Length of polynomial arrays
 * @return Struct containing quotient and remainder polynomials
 */
poly_div_result poly_div(uint8_t *dividend, uint8_t *divisor, int len) {
    uint8_t *quotient = calloc(len, sizeof(uint8_t));
    uint8_t *temp_dividend = malloc(len * sizeof(uint8_t));
    for (int i = 0; i < len; i++) {
        temp_dividend[i] = dividend[i];
    }

    int dividend_deg = poly_degree(temp_dividend, len);
    int divisor_deg = poly_degree(divisor, len);

    while (dividend_deg >= divisor_deg && dividend_deg >= 0) {
        uint8_t coeff = gf_div(temp_dividend[dividend_deg], divisor[divisor_deg]);
        int deg_diff = dividend_deg - divisor_deg;
        quotient[deg_diff] = coeff;

        for (int i = 0; i <= divisor_deg; i++) {
            temp_dividend[i + deg_diff] = gf_add(temp_dividend[i + deg_diff], gf_mult(coeff, divisor[i]));
        }
        dividend_deg = poly_degree(temp_dividend, len);
    }
    poly_div_result result;
    result.quotient = quotient;
    result.remainder = temp_dividend;

    return result;
}
