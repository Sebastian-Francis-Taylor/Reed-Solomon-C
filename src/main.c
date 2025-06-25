#include "galios.h"
#include "rs_decoder.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    initialise_gf();

    // DEBUG PRINT
    //    printf("GF table check:\n");
    //    printf("antilog[0] = %02X (should be 01)\n", global_tables.antilog_table[0]);
    //    printf("antilog[1] = %02X (should be 02)\n", global_tables.antilog_table[1]);
    //    printf("log[1] = %d (should be 0)\n", global_tables.log_table[1]);
    //    printf("log[2] = %d (should be 1)\n", global_tables.log_table[2]);
    //    printf("2 * 3 = %02X\n", gf_mult(2, 3));
    //    printf("inv(2) = %02X\n", gf_inverse(2));
    // Use a non-zero message
    uint8_t encoded_message[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    int encoded_len = 14;

    printf("Message (without Errors) \t");
    for (int i = 0; i < encoded_len; ++i) {
        printf("%02X ", encoded_message[i]);
    }
    printf("\n");

    // IT WORKS WITH 2 ERRORS
    encoded_message[2] ^= 0x55;
    encoded_message[5] ^= 0xAA;
    encoded_message[10] ^= 0x11;
    //    encoded_message[13] ^= 0xA1;

    printf("Encoded (with errors): \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%02X ", encoded_message[i]);
    }
    printf("\n");

    int max_errors = 3;
    uint8_t *decoded = decode_message(encoded_message, encoded_len, max_errors);

    printf("Decoded message: \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%02X ", decoded[i]);
    }
    printf("\n");

    free(decoded);
    return 0;
}
