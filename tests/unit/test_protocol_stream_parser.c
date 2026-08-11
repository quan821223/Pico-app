#include "protocol_stream_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CAPTURED_FRAMES 8u

typedef struct {
    uint8_t frames[MAX_CAPTURED_FRAMES][PROTOCOL_FRAME_SIZE];
    size_t count;
} frame_capture_t;

static void fail(const char *expression, int line)
{
    fprintf(stderr, "assertion failed at line %d: %s\n", line, expression);
    exit(EXIT_FAILURE);
}

#define CHECK(expression) \
    do { \
        if (!(expression)) { \
            fail(#expression, __LINE__); \
        } \
    } while (0)

static void capture_frame(
    const uint8_t frame[PROTOCOL_FRAME_SIZE],
    void *context)
{
    frame_capture_t *capture = context;

    CHECK(capture != NULL);
    CHECK(capture->count < MAX_CAPTURED_FRAMES);
    memcpy(capture->frames[capture->count], frame, PROTOCOL_FRAME_SIZE);
    ++capture->count;
}

static void check_frame(
    const frame_capture_t *capture,
    size_t index,
    const uint8_t expected[PROTOCOL_FRAME_SIZE])
{
    CHECK(index < capture->count);
    CHECK(memcmp(capture->frames[index], expected, PROTOCOL_FRAME_SIZE) == 0);
}

static void test_noise_is_ignored_until_header(void)
{
    protocol_stream_parser_t parser;
    frame_capture_t capture = {0};
    const uint8_t input[] = {0x00, 0x52, 0x57, 0x7F};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, input, sizeof(input), 1u, capture_frame, &capture);

    CHECK(capture.count == 0u);
    CHECK(protocol_stream_parser_pending_length(&parser) == 0u);
}

static void test_complete_frame_is_emitted(void)
{
    protocol_stream_parser_t parser;
    frame_capture_t capture = {0};
    const uint8_t frame[] = {0xFA, 0x52, 0x01, 0x1A, 0x00};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, frame, sizeof(frame), 10u, capture_frame, &capture);

    CHECK(capture.count == 1u);
    check_frame(&capture, 0u, frame);
    CHECK(protocol_stream_parser_pending_length(&parser) == 0u);
}

static void test_split_frame_is_reassembled(void)
{
    protocol_stream_parser_t parser;
    frame_capture_t capture = {0};
    const uint8_t first[] = {0xDA, 0x52};
    const uint8_t second[] = {0x20, 0x00, 0x00};
    const uint8_t expected[] = {0xDA, 0x52, 0x20, 0x00, 0x00};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, first, sizeof(first), 20u, capture_frame, &capture);
    CHECK(protocol_stream_parser_pending_length(&parser) == 2u);
    protocol_stream_parser_feed(
        &parser, second, sizeof(second), 21u, capture_frame, &capture);

    CHECK(capture.count == 1u);
    check_frame(&capture, 0u, expected);
}

static void test_new_header_resynchronizes_partial_frame(void)
{
    protocol_stream_parser_t parser;
    frame_capture_t capture = {0};
    const uint8_t input[] = {0xFA, 0x52, 0xDA, 0x52, 0x03, 0x00, 0x00};
    const uint8_t expected[] = {0xDA, 0x52, 0x03, 0x00, 0x00};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, input, sizeof(input), 30u, capture_frame, &capture);

    CHECK(capture.count == 1u);
    check_frame(&capture, 0u, expected);
}

static void test_two_frames_in_one_batch_are_emitted(void)
{
    protocol_stream_parser_t parser;
    frame_capture_t capture = {0};
    const uint8_t input[] = {
        0xFA, 0x52, 0x01, 0x1A, 0x00,
        0xDA, 0x57, 0x03, 0x01, 0x02,
    };
    const uint8_t first[] = {0xFA, 0x52, 0x01, 0x1A, 0x00};
    const uint8_t second[] = {0xDA, 0x57, 0x03, 0x01, 0x02};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, input, sizeof(input), 40u, capture_frame, &capture);

    CHECK(capture.count == 2u);
    check_frame(&capture, 0u, first);
    check_frame(&capture, 1u, second);
}

static void test_timeout_expires_only_at_threshold(void)
{
    protocol_stream_parser_t parser;
    const uint8_t partial[] = {0xFA, 0x52};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, partial, sizeof(partial), 1000u, NULL, NULL);

    CHECK(!protocol_stream_parser_expire(&parser, 1099u, 100u));
    CHECK(protocol_stream_parser_pending_length(&parser) == 2u);
    CHECK(protocol_stream_parser_expire(&parser, 1100u, 100u));
    CHECK(protocol_stream_parser_pending_length(&parser) == 0u);
    CHECK(!protocol_stream_parser_expire(&parser, 1200u, 100u));
}

static void test_timeout_handles_uint32_wraparound(void)
{
    protocol_stream_parser_t parser;
    const uint8_t partial[] = {0xDA};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, partial, sizeof(partial), UINT32_MAX - 10u, NULL, NULL);

    CHECK(!protocol_stream_parser_expire(&parser, 20u, 32u));
    CHECK(protocol_stream_parser_expire(&parser, 21u, 32u));
}

static void test_reset_discards_partial_frame(void)
{
    protocol_stream_parser_t parser;
    const uint8_t partial[] = {0xFA, 0x52, 0x01};

    protocol_stream_parser_init(&parser);
    protocol_stream_parser_feed(
        &parser, partial, sizeof(partial), 1u, NULL, NULL);
    CHECK(protocol_stream_parser_pending_length(&parser) == 3u);

    protocol_stream_parser_init(&parser);
    CHECK(protocol_stream_parser_pending_length(&parser) == 0u);
}

int main(void)
{
    test_noise_is_ignored_until_header();
    test_complete_frame_is_emitted();
    test_split_frame_is_reassembled();
    test_new_header_resynchronizes_partial_frame();
    test_two_frames_in_one_batch_are_emitted();
    test_timeout_expires_only_at_threshold();
    test_timeout_handles_uint32_wraparound();
    test_reset_discards_partial_frame();

    puts("protocol_stream_parser: all tests passed");
    return EXIT_SUCCESS;
}

