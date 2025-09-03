/**
 * Reed-Solomon Error Correction Encoder
 *
 * Implementation of a systemic Reed-Solomon encoder operating over GF(2**8) with primitive element
 * = 2
 *
 * The encoder implements a RS(223,255) Reed-Solomon codes, meaning that there can be a maximum of
 * 223 information bytes, resulting in 32 parity bytes. The encoder places the parity symbols at the
 * front of the message followed by the original message
 *
 * Memory Layout:
 *  encoded_message: [32 parity symbols][223 information symbols]
 *  Total length: 255 symbols (FIELD_SIZE)
 *
 * Copyright (C) 2025
 * Author: Sebastian Francis Taylor <me@sebastian-taylor.com>
 */
#include "rs_encoder.h"
#include "galois.h"
#include <stdint.h>
#include <stdlib.h>

// zeros so that the array does not need to be padded to length 255 during runtime, handy for
// poly_div
static const uint8_t generator_poly[255] = {
    1,   232, 29,  189, 50, 142, 246, 232, 15,  43,  82, 164, 238, 1,  158, 13, 119, 158, 224,
    134, 227, 210, 163, 50, 107, 40,  27,  104, 253, 24, 239, 216, 45, 0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,  0,   0,  0,   0,   0,
    0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0};

/**
 * @brief Function that pads zeros to the end of an array
 * @param array the array that needs to be extended
 * @param len the length of the array before extension
 * @param extra the new length of the arry
 * @return the extended array
 */
uint8_t *extend_array(const uint8_t *array, int len, int extra) {
    uint8_t *out = calloc(len + extra, 1);
    memcpy(out, array, len * sizeof(uint8_t));

    return out;
}

/**
 * @brief adds zeros to the front of an arra
 * @param array the array that gets zeros in front
 * @param len length of array
 * @param k the amount of zeros to be added to the front of array
 * @return shifted array with k leading zeros
 */
uint8_t *shift_array(const uint8_t *array, int len, int k) {
    uint8_t *out = calloc(len + k, 1);
    memcpy(out + k, array, len * sizeof(uint8_t));

    return out;
}

/**
 * @brief encodeds a message of up to 223 length with a reed-solomon encoder
 * @param info_poly the array that is going to be encoded
 * @param info_poly_len the length of the array that is going to be encoded (max 223 length)
 * @return encoded array of length 255
 */
uint8_t *rs_encode(uint8_t *info_poly, int info_poly_len) {

    uint8_t *padded_info_poly = shift_array(info_poly, info_poly_len, NUM_SYNDROMES);

    poly_div_result division_result =
        poly_div(padded_info_poly, (uint8_t *)generator_poly, FIELD_SIZE);

    free(division_result.quotient);
    uint8_t *parity_symbols = division_result.remainder;

    uint8_t *encoded_message = calloc(FIELD_SIZE, sizeof(uint8_t));

    memcpy(encoded_message, parity_symbols, NUM_SYNDROMES * sizeof(encoded_message[0]));
    memcpy(encoded_message + NUM_SYNDROMES, info_poly,
           (FIELD_SIZE - NUM_SYNDROMES) * sizeof(encoded_message[0]));

    free(padded_info_poly);
    free(parity_symbols);

    return encoded_message;
}
