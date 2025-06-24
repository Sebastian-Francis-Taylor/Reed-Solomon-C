#include "galios.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declare decoder function
uint8_t *decode_message(uint8_t *encoded_message, int message_len);

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

    uint8_t original_message[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int message_len = 8;

    printf("Message (without Errors) \t");
    for (int i = 0; i < message_len; ++i) {
        printf("%02X ", original_message[i]);
    }
    printf("\n");

    uint8_t encoded_message[14] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 20, 30, 40, 50, 60};
    int encoded_len = 14;

    encoded_message[2] ^= 0x55;
    encoded_message[5] ^= 0xAA;
    encoded_message[10] ^= 0x11;

    printf("Encoded (with errors): \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%02X ", encoded_message[i]);
    }
    printf("\n");

    uint8_t *decoded = decode_message(encoded_message, encoded_len);

    printf("Decoded message: \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%02X ", decoded[i]);
    }
    printf("\n");

    free(decoded);
    return 0;
}
