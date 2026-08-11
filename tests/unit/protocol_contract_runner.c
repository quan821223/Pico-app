#include "app_protocol.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static bool parse_byte(const char *text, uint8_t *value)
{
    char *end;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(text, &end, 16);
    if (errno != 0 || end == text || *end != '\0' || parsed > 0xFFu) {
        return false;
    }

    *value = (uint8_t)parsed;
    return true;
}

int main(int argc, char **argv)
{
    uint8_t request[APP_PROTOCOL_REQUEST_SIZE];
    app_protocol_inputs_t inputs = {0};
    app_protocol_result_t result;

    if (argc != 7) {
        fprintf(stderr, "usage: %s B0 B1 B2 B3 B4 CHAMBER_STATUS\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < APP_PROTOCOL_REQUEST_SIZE; ++index) {
        if (!parse_byte(argv[index + 1u], &request[index])) {
            return EXIT_FAILURE;
        }
    }
    if (!parse_byte(argv[6], &inputs.chamber_status)) {
        return EXIT_FAILURE;
    }

    app_protocol_handle(request, &inputs, &result);
    if (!result.has_response) {
        puts("NONE");
        return EXIT_SUCCESS;
    }

    for (size_t index = 0; index < result.response.length; ++index) {
        printf(index == 0u ? "%02X" : " %02X", result.response.data[index]);
    }
    putchar('\n');
    return EXIT_SUCCESS;
}

