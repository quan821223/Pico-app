#include "config_journal.h"
#include "configuration_service.h"
#include "crc32.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t slots[CONFIG_JOURNAL_SLOT_COUNT][CONFIG_JOURNAL_RECORD_SIZE];
    bool fail_write;
} memory_storage_t;

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

static bool read_slot(void *context, uint8_t slot, uint8_t *record)
{
    memory_storage_t *storage = context;
    memcpy(record, storage->slots[slot], CONFIG_JOURNAL_RECORD_SIZE);
    return true;
}

static bool write_slot(void *context, uint8_t slot, const uint8_t *record)
{
    memory_storage_t *storage = context;
    if (storage->fail_write) {
        return false;
    }
    memcpy(storage->slots[slot], record, CONFIG_JOURNAL_RECORD_SIZE);
    return true;
}

static config_journal_storage_t storage_port(memory_storage_t *storage)
{
    const config_journal_storage_t port = {
        .read_slot = read_slot,
        .write_slot = write_slot,
        .context = storage,
    };
    return port;
}

static void test_crc32_standard_vector(void)
{
    static const uint8_t data[] = "123456789";
    CHECK(crc32_compute(data, sizeof(data) - 1u) == UINT32_C(0xCBF43926));
}

static void test_journal_uses_newest_valid_slot_and_falls_back(void)
{
    memory_storage_t storage;
    config_journal_storage_t port;
    config_journal_t journal;
    config_journal_t reloaded;
    runtime_config_t first = runtime_config_defaults(BOARD_ID_PICO);
    runtime_config_t second = first;
    runtime_config_t loaded;

    memset(&storage, 0xFF, sizeof(storage));
    storage.fail_write = false;
    port = storage_port(&storage);
    config_journal_init(&journal, &port);

    first.response_delay_ms = 75u;
    second.response_delay_ms = 125u;
    CHECK(config_journal_save(&journal, &first));
    CHECK(config_journal_save(&journal, &second));

    config_journal_init(&reloaded, &port);
    CHECK(config_journal_load(&reloaded, &loaded));
    CHECK(loaded.response_delay_ms == 125u);

    storage.slots[1][5] ^= 0x01u;
    config_journal_init(&reloaded, &port);
    CHECK(config_journal_load(&reloaded, &loaded));
    CHECK(loaded.response_delay_ms == 75u);
}

static void test_failed_save_does_not_advance_journal(void)
{
    memory_storage_t storage;
    config_journal_storage_t port;
    config_journal_t journal;
    runtime_config_t config = runtime_config_defaults(BOARD_ID_PICO);

    memset(&storage, 0xFF, sizeof(storage));
    storage.fail_write = false;
    port = storage_port(&storage);
    config_journal_init(&journal, &port);
    CHECK(config_journal_save(&journal, &config));

    storage.fail_write = true;
    config.response_delay_ms = 99u;
    CHECK(!config_journal_save(&journal, &config));
    CHECK(journal.sequence == 1u);
    CHECK(journal.active_slot == 0u);
}

static void test_control_updates_reads_and_saves(void)
{
    memory_storage_t storage;
    config_journal_storage_t port;
    configuration_service_t service;
    runtime_config_t defaults = runtime_config_defaults(BOARD_ID_PICO);
    app_response_t response;
    const uint32_t rp2040_mask =
        (UINT32_C(1) << BOARD_ID_PICO) |
        (UINT32_C(1) << BOARD_ID_PICO_W) |
        (UINT32_C(1) << BOARD_ID_WAVESHARE_RP2040_ZERO);
    const uint8_t set_delay[] = {0xCF, 0x57, 0x02, 0x00, 0x4B};
    const uint8_t read_delay[] = {0xCF, 0x52, 0x02, 0x00, 0x00};
    const uint8_t set_board[] = {0xCF, 0x57, 0x01, 0x00, 0x02};
    const uint8_t save[] = {0xCF, 0x57, 0x7E, 0xA5, 0x5A};

    memset(&storage, 0xFF, sizeof(storage));
    storage.fail_write = false;
    port = storage_port(&storage);
    configuration_service_init(&service, &defaults, rp2040_mask, &port);

    configuration_service_handle(&service, set_delay, &response);
    CHECK(response.data[1] == 0u);
    CHECK(service.current.response_delay_ms == 75u);
    configuration_service_handle(&service, read_delay, &response);
    CHECK(response.length == 7u);
    CHECK(response.data[3] == 0u && response.data[4] == 75u);

    configuration_service_handle(&service, set_board, &response);
    CHECK(service.current.board_id == BOARD_ID_WAVESHARE_RP2040_ZERO);
    CHECK(service.restart_required);
    configuration_service_handle(&service, save, &response);
    CHECK(response.data[1] == 0u);
    CHECK(service.journal.has_persisted_record);
}

static void test_rp2350_rejects_rp2040_profile(void)
{
    memory_storage_t storage;
    config_journal_storage_t port;
    configuration_service_t service;
    runtime_config_t defaults = runtime_config_defaults(BOARD_ID_PICO2);
    app_response_t response;
    const uint8_t set_pico[] = {0xCF, 0x57, 0x01, 0x00, 0x00};

    memset(&storage, 0xFF, sizeof(storage));
    storage.fail_write = false;
    port = storage_port(&storage);
    configuration_service_init(
        &service,
        &defaults,
        UINT32_C(1) << BOARD_ID_PICO2,
        &port);
    configuration_service_handle(&service, set_pico, &response);

    CHECK(response.data[1] == 0x02u);
    CHECK(service.current.board_id == BOARD_ID_PICO2);
}

int main(void)
{
    test_crc32_standard_vector();
    test_journal_uses_newest_valid_slot_and_falls_back();
    test_failed_save_does_not_advance_journal();
    test_control_updates_reads_and_saves();
    test_rp2350_rejects_rp2040_profile();

    puts("runtime_configuration: all tests passed");
    return EXIT_SUCCESS;
}
