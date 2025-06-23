#include "galios.h"
#include <stdint.h>
#include <stdlib.h>

int poly_degree(uint8_t *poly, int len) {
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
                result[i + j] = gf_add(result[i + j], gf_mult(a[i], b[j]));
            }
        }
    }
    return result;
}

uint8_t *poly_div(uint8_t *dividend, uint8_t *divisor, int len) {
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
   
   free(temp_dividend);
   return quotient;
}
