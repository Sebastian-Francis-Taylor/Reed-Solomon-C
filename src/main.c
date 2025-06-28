#include "galois.h"
#include "rs_decoder.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    initialise_gf();

    uint8_t encoded_message[255] = {0};
    int encoded_len = 255;

    printf("Message (without Errors) \t");
    for (int i = 0; i < encoded_len; ++i) {
        printf("%d ", encoded_message[i]);
    }
    printf("\n");

    encoded_message[5] ^= 163;
    encoded_message[22] ^= 77;
    encoded_message[38] ^= 201;
    encoded_message[56] ^= 29;
    encoded_message[74] ^= 112;
    encoded_message[95] ^= 186;
    encoded_message[118] ^= 43;
    encoded_message[141] ^= 224;
    encoded_message[167] ^= 95;
    encoded_message[189] ^= 158;
    encoded_message[213] ^= 71;
    encoded_message[237] ^= 190;

    printf("Encoded (with errors): \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%d ", encoded_message[i]);
    }
    printf("\n");

    int max_errors = 16;
    uint8_t *decoded = decode_message(encoded_message, encoded_len, max_errors);

    printf("Decoded message: \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%d ", decoded[i]);
    }
    printf("\n");

    free(decoded);
    return 0;
}
