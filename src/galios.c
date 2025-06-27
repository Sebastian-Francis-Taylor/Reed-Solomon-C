#include "galios.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define gf_bits 8
#define prim_poly 0b100011101 // z^8 + z^4 + z^3 + z^2 + 1 F(2^8)

log_tables global_tables;

// ============================================================================
// GALOIS FIELD OPERATIONS
// ============================================================================

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

void initialise_gf() {
    global_tables = init_gf_tables();
}

uint8_t gf_add(uint8_t a, uint8_t b) {
    return a ^ b;
}

uint8_t gf_mult(uint8_t a, uint8_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0;

    uint8_t log_result = (global_tables.log_table[a] + global_tables.log_table[b]) % 255;
    return global_tables.antilog_table[log_result];
}

uint8_t gf_div(uint8_t a, uint8_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0;

    uint8_t log_result = (global_tables.log_table[a] - global_tables.log_table[b] + 255) % 255;
    return global_tables.antilog_table[log_result];
}

uint8_t gf_pow(uint8_t base, uint8_t exponent) {
    if (exponent == 0) return 1;
    if (base == 0) return 0;

    return global_tables.antilog_table[(global_tables.log_table[base] * exponent) % 255];
}

int gf_deg(uint8_t poly) {
    if (poly == 0) return -1;

    int degree = 7;
    while (degree >= 0) {
        if (poly & (1 << degree)) {
            return degree;
        }
        --degree;
    }

    return -1;
}

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
        // Even powers automatically become 0 due to calloc
    }

    return poly_diff;
}

// big-endian
uint8_t gf_poly_eval(const uint8_t *poly, int degree, uint8_t x, int len) {
    uint8_t result = 0;
    int start_idx = len - 1 - degree;
    for (int i = 0; i <= degree; i++) {
        result = gf_mult(result, x);
        result = gf_add(result, poly[start_idx + i]);
    }
    return result;
}

// big-endian
uint8_t *gf_find_roots(const uint8_t *poly, int degree, int *out_num_roots, int len) {
    uint8_t *roots = malloc(256 * sizeof(uint8_t));

    int num_roots = 0;
    for (uint16_t x = 0; x < 256; x++) {
        if (gf_poly_eval(poly, degree, (uint8_t)x, len) == 0) {
            roots[num_roots++] = (uint8_t)x;
        }
    }

    // DEBUG PRINT
    printf("Raw roots found by gf_find_roots: ");
    for (int i = 0; i < num_roots; ++i) {
        printf("%d ", roots[i]);
    }
    printf(" (total: %d)\n", num_roots);

    printf("roots mapping\n");
    for (int i = 0; i < num_roots; ++i) {
        printf("root: %d log_table[root]: %d\n", roots[i], global_tables.log_table[roots[i]]);
    }
    printf("\n");

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

// ============================================================================
// POLYNOMIAL OPERATIONS
// ============================================================================

// For big-endian: poly[0] = highest degree coefficient  
int poly_degree_big_endian(uint8_t *poly, int len) {
    for (int i = 0; i < len; i++) {
        if (poly[i] != 0) return len - 1 - i;
    }
    return -1;
}

// For little-endian: poly[i] = coefficient of x^i
int poly_degree_little_endian(uint8_t *poly, int len) {
    for (int i = len - 1; i >= 0; i--) {
        if (poly[i] != 0) return i;
    }
    return -1;
}

uint8_t *poly_add(uint8_t *a, uint8_t *b, int len) {
    uint8_t *result = malloc(len * sizeof(uint8_t));
    for (int i = 0; i < len; i++) {
        result[i] = gf_add(a[i], b[i]);
    }
    return result;
}

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

void reverse_array(uint8_t *arr, int len) {
    for (int i = 0; i < len / 2; i++) {
        uint8_t temp = arr[i];
        arr[i] = arr[len - 1 - i];
        arr[len - 1 - i] = temp;
    }
}

poly_div_result poly_div(uint8_t *dividend, uint8_t *divisor, int len) {
    
    uint8_t *quotient = calloc(len, sizeof(uint8_t));
    uint8_t *temp_dividend = malloc(len * sizeof(uint8_t));
    for (int i = 0; i < len; i++) {
        temp_dividend[i] = dividend[i];
    }

    int dividend_deg = poly_degree_little_endian(temp_dividend, len);
    int divisor_deg = poly_degree_little_endian(divisor, len);

    while (dividend_deg >= divisor_deg && dividend_deg >= 0) {
        uint8_t coeff = gf_div(temp_dividend[dividend_deg], divisor[divisor_deg]);
        // printf("temp_dividend[dividend_deg]: %hhu \n", temp_dividend[dividend_deg]);
        int deg_diff = dividend_deg - divisor_deg;
        quotient[deg_diff] = coeff;

        for (int i = 0; i <= divisor_deg; i++) {
            temp_dividend[i + deg_diff] = gf_add(temp_dividend[i + deg_diff], gf_mult(coeff, divisor[i]));
        }
        dividend_deg = poly_degree_little_endian(temp_dividend, len);
    }
    poly_div_result result;
    result.quotient = quotient;
    result.remainder = temp_dividend;

    return result;
}

uint8_t gf_inverse(uint8_t a) {
    return gf_pow(a, 254);
}
