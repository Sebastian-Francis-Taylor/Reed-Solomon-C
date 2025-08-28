#ifndef GALOIS_H
#define GALOIS_H

#include <stdint.h>
#include <stdlib.h>

extern const int FIELD_SIZE;
extern const int MAX_ERRORS;
extern const int NUM_SYNDROMES;

typedef struct {
  uint8_t *log_table;
  uint8_t *antilog_table;
} log_tables;

typedef struct {
  uint8_t *quotient;
  uint8_t *remainder;
} poly_div_result;

extern log_tables global_tables;

// non-poly gf
log_tables init_gf_tables();
void initialise_gf();
uint8_t gf_add(uint8_t a, uint8_t b);
uint8_t gf_mult(uint8_t a, uint8_t b);
uint8_t gf_div(uint8_t a, uint8_t b);
uint8_t gf_pow(uint8_t base, uint8_t exponent);
uint8_t gf_inv(uint8_t x);
int gf_deg(uint8_t poly);
uint8_t *gf_diff(uint8_t *poly, int poly_len);
uint8_t gf_poly_eval(const uint8_t *poly, int degree, uint8_t x, int len);
uint8_t *gf_find_roots(const uint8_t *poly, int degree, int *out_num_roots,
                       int len);

// Polynomial operations
int poly_degree(uint8_t *poly, int len);
uint8_t *poly_add(uint8_t *a, uint8_t *b, int len);
uint8_t *poly_mult(uint8_t *a, uint8_t *b, int len);
poly_div_result poly_div(uint8_t *dividend, uint8_t *divisor, int len);
#endif
