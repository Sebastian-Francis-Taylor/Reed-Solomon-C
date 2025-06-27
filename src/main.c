#include "galios.h"
#include "rs_decoder.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void poly_div_test() {
    // Test the first division from your hand calculation
    uint8_t dividend[7] = {0, 0, 0, 0, 0, 0, 1}; // x^6
    uint8_t divisor[7] = {140, 85, 12, 73, 21, 7, 0};
    // BELOW IS ORIGINAL
    // uint8_t divisor[7] = {7, 21, 73, 12, 85, 140, 0}; // syndrome poly padded to same size
    // uint8_t dividend[7] = {0, 0, 0, 0, 0, 0, 1};      // x^6

    printf("Testing poly_div:\n");
    printf("Dividend: ");
    for (int i = 0; i < 7; i++) printf("%d ", dividend[i]);
    printf("\n");

    printf("Divisor: ");
    for (int i = 0; i < 7; i++) printf("%d ", divisor[i]);
    printf("\n");

    poly_div_result result = poly_div(dividend, divisor, 7);

    printf("Quotient: ");
    for (int i = 0; i < 7; i++) printf("%d ", result.quotient[i]);
    printf("\n");

    printf("Remainder: ");
    for (int i = 0; i < 7; i++) printf("%d ", result.remainder[i]);
    printf("\n");
}

int main() {
    initialise_gf();

    // poly_div_test();
    uint8_t encoded_message[255] = {0};

    int encoded_len = 255;

    printf("Message (without Errors) \t");
    for (int i = 0; i < encoded_len; ++i) {
        printf("%d ", encoded_message[i]);
    }
    printf("\n");

    encoded_message[254] ^= 1;
    encoded_message[253] ^= 1;
    encoded_message[252] ^= 1;
    //    encoded_message[13] ^= 0xA1;

    printf("Encoded (with errors): \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%d ", encoded_message[i]);
    }
    printf("\n");

    int max_errors = 3;
    uint8_t *decoded = decode_message(encoded_message, encoded_len, max_errors);

    printf("Decoded message: \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%d ", decoded[i]);
    }
    printf("\n");

    free(decoded);
    return 0;
}
