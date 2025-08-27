#include "galois.h"
#include "rs_decoder.h"
#include "rs_encoder.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ARR_1 {72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33, 32, 84, 104, 105, 115, 32, 105, 115, 32, 97, 32, 116, 101, 115, 116, 32, 109, 101, 115, 115, 97, 103, 101, 32, 102, 111, 114, 32, 82, 101, 101, 100, 45, 83, 111, 108, 111, 109, 111, 110, 32, 101, 114, 114, 111, 114, 32, 99, 111, 114, 114, 101, 99, 116, 105, 111, 110, 46, 32, 84, 104, 105, 115, 32, 97, 114, 114, 97, 121, 32, 99, 111, 110, 116, 97, 105, 110, 115, 32, 50, 50, 51, 32, 98, 121, 116, 101, 115, 32, 111, 102, 32, 100, 97, 116, 97, 32, 116, 104, 97, 116, 32, 119, 105, 108, 108, 32, 98, 101, 32, 101, 110, 99, 111, 100, 101, 100, 32, 97, 110, 100, 32, 100, 101, 99, 111, 100, 101, 100, 32, 117, 115, 105, 110, 103, 32, 121, 111, 117, 114, 32, 82, 83, 32, 105, 109, 112, 108, 101, 109, 101, 110, 116, 97, 116, 105, 111, 110, 46, 32, 71, 111, 111, 100, 32, 108, 117, 99, 107, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

int main() {
    initialise_gf();

    for (int i = 0; i < 10; i++) {
        printf("alpha^%d = %d\n", i, gf_pow(2, i));
    }
    printf("\n");
    printf("\n");

    int info_len = 223;

    int test_num = 2;

    uint8_t *message = calloc(223, sizeof(uint8_t));
    if (test_num == 1) {
        // Fill with zeros
        memset(message, 0, info_len);
    } else if (test_num == 2) {
        // Copy TEST_ARR_1 into message
        static const uint8_t temp[] = TEST_ARR_1;
        size_t copy_len = sizeof(temp) < info_len ? sizeof(temp) : info_len;
        memcpy(message, temp, copy_len);
    } else if (test_num == 3) {
        // Fill message with incremental values
        for (int i = 0; i < info_len; ++i) {
            message[i] = (uint8_t)i;
            printf("%d ", message[i]);
        }
        printf("\n");
    }

    uint8_t *encoded_message = rs_encode(message, info_len);
    int encoded_len = 255;
    printf("encoded_message (without errors): \t");
    for (int i = 0; i < encoded_len; ++i) {
        printf("%d ", encoded_message[i]);
    }
    printf("\n");
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

    printf("encoded_message (with errors): \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%d ", encoded_message[i]);
    }
    printf("\n");

    uint8_t *decoded_message = decode_message(encoded_message, encoded_len);
    free(global_tables.antilog_table);
    free(global_tables.log_table);

    printf("Decoded message: \t\t");
    for (int i = 0; i < encoded_len; i++) {
        printf("%d ", decoded_message[i]);
    }
    printf("\n");

    free(decoded_message);
    return 0;
}
