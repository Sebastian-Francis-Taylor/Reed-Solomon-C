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

    encoded_message[10] ^= 73;
    encoded_message[25] ^= 156;
    encoded_message[34] ^= 42;
    encoded_message[45] ^= 219;
    encoded_message[67] ^= 91;
    encoded_message[89] ^= 183;
    encoded_message[98] ^= 67;
    encoded_message[123] ^= 204;
    encoded_message[145] ^= 38;
    encoded_message[156] ^= 147;
    encoded_message[178] ^= 85;
    encoded_message[200] ^= 193;
    encoded_message[221] ^= 76;
    encoded_message[252] ^= 129;
    encoded_message[253] ^= 241;
    encoded_message[254] ^= 58;

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
