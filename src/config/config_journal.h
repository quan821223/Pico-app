#ifndef CONFIG_JOURNAL_H
#define CONFIG_JOURNAL_H

#include "runtime_config.h"

#include <stdbool.h>
#include <stdint.h>

#define CONFIG_JOURNAL_SLOT_COUNT 2u
#define CONFIG_JOURNAL_RECORD_SIZE 24u

typedef bool (*config_journal_read_slot_t)(
    void *context,
    uint8_t slot,
    uint8_t record[CONFIG_JOURNAL_RECORD_SIZE]);
typedef bool (*config_journal_write_slot_t)(
    void *context,
    uint8_t slot,
    const uint8_t record[CONFIG_JOURNAL_RECORD_SIZE]);

typedef struct {
    config_journal_read_slot_t read_slot;
    config_journal_write_slot_t write_slot;
    void *context;
} config_journal_storage_t;

typedef struct {
    config_journal_storage_t storage;
    uint32_t sequence;
    uint8_t active_slot;
    bool has_persisted_record;
} config_journal_t;

void config_journal_init(
    config_journal_t *journal,
    const config_journal_storage_t *storage);

bool config_journal_load(
    config_journal_t *journal,
    runtime_config_t *config);

bool config_journal_save(
    config_journal_t *journal,
    const runtime_config_t *config);

bool config_journal_decode(
    const uint8_t record[CONFIG_JOURNAL_RECORD_SIZE],
    runtime_config_t *config,
    uint32_t *sequence);

#endif
