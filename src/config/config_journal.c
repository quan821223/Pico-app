#include "config_journal.h"

#include "crc32.h"

#include <stddef.h>
#include <string.h>

enum {
    RECORD_CRC_OFFSET = 20u,
};

static const uint8_t RECORD_MAGIC[] = {'P', 'C', 'F', 'G'};

static uint16_t read_u16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8u);
}
static uint32_t read_u32(const uint8_t *data)
{
    return (uint32_t)data[0] |
        ((uint32_t)data[1] << 8u) |
        ((uint32_t)data[2] << 16u) |
        ((uint32_t)data[3] << 24u);
}

static void write_u16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8u);
}

static void write_u32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8u);
    data[2] = (uint8_t)(value >> 16u);
    data[3] = (uint8_t)(value >> 24u);
}

static void encode(
    const runtime_config_t *config,
    uint32_t sequence,
    uint8_t record[CONFIG_JOURNAL_RECORD_SIZE])
{
    memset(record, 0, CONFIG_JOURNAL_RECORD_SIZE);
    memcpy(record, RECORD_MAGIC, sizeof(RECORD_MAGIC));
    write_u16(&record[4], 1u);
    write_u16(&record[6], CONFIG_JOURNAL_RECORD_SIZE);
    write_u32(&record[8], sequence);
    record[12] = (uint8_t)config->board_id;
    write_u16(&record[14], config->response_delay_ms);
    write_u16(&record[16], config->frame_timeout_ms);
    write_u32(&record[RECORD_CRC_OFFSET],
        crc32_compute(record, RECORD_CRC_OFFSET));
}

bool config_journal_decode(
    const uint8_t record[CONFIG_JOURNAL_RECORD_SIZE],
    runtime_config_t *config,
    uint32_t *sequence)
{
    runtime_config_t decoded;

    if (record == NULL || config == NULL || sequence == NULL ||
        memcmp(record, RECORD_MAGIC, sizeof(RECORD_MAGIC)) != 0 ||
        read_u16(&record[4]) != 1u ||
        read_u16(&record[6]) != CONFIG_JOURNAL_RECORD_SIZE ||
        read_u32(&record[RECORD_CRC_OFFSET]) !=
            crc32_compute(record, RECORD_CRC_OFFSET)) {
        return false;
    }

    decoded.board_id = (board_id_t)record[12];
    decoded.response_delay_ms = read_u16(&record[14]);
    decoded.frame_timeout_ms = read_u16(&record[16]);
    if (!runtime_config_is_valid(&decoded)) {
        return false;
    }

    *config = decoded;
    *sequence = read_u32(&record[8]);
    return true;
}

static bool is_newer(uint32_t candidate, uint32_t reference)
{
    return (int32_t)(candidate - reference) > 0;
}

void config_journal_init(
    config_journal_t *journal,
    const config_journal_storage_t *storage)
{
    if (journal == NULL) {
        return;
    }

    memset(journal, 0, sizeof(*journal));
    if (storage != NULL) {
        journal->storage = *storage;
    }
}

bool config_journal_load(
    config_journal_t *journal,
    runtime_config_t *config)
{
    runtime_config_t candidates[CONFIG_JOURNAL_SLOT_COUNT];
    uint32_t sequences[CONFIG_JOURNAL_SLOT_COUNT] = {0};
    bool valid[CONFIG_JOURNAL_SLOT_COUNT] = {false};
    uint8_t records[CONFIG_JOURNAL_SLOT_COUNT][CONFIG_JOURNAL_RECORD_SIZE];

    if (journal == NULL || config == NULL || journal->storage.read_slot == NULL) {
        return false;
    }

    for (uint8_t slot = 0; slot < CONFIG_JOURNAL_SLOT_COUNT; ++slot) {
        valid[slot] = journal->storage.read_slot(
            journal->storage.context, slot, records[slot]) &&
            config_journal_decode(records[slot], &candidates[slot], &sequences[slot]);
    }

    if (!valid[0] && !valid[1]) {
        return false;
    }

    journal->active_slot = valid[1] &&
        (!valid[0] || is_newer(sequences[1], sequences[0])) ? 1u : 0u;
    journal->sequence = sequences[journal->active_slot];
    journal->has_persisted_record = true;
    *config = candidates[journal->active_slot];
    return true;
}

bool config_journal_save(
    config_journal_t *journal,
    const runtime_config_t *config)
{
    uint8_t record[CONFIG_JOURNAL_RECORD_SIZE];
    const uint8_t target_slot = journal != NULL && journal->has_persisted_record
        ? (uint8_t)(journal->active_slot ^ 1u)
        : 0u;
    const uint32_t sequence = journal != NULL && journal->has_persisted_record
        ? journal->sequence + 1u
        : 1u;

    if (journal == NULL || !runtime_config_is_valid(config) ||
        journal->storage.write_slot == NULL) {
        return false;
    }

    encode(config, sequence, record);
    if (!journal->storage.write_slot(
            journal->storage.context, target_slot, record)) {
        return false;
    }

    journal->active_slot = target_slot;
    journal->sequence = sequence;
    journal->has_persisted_record = true;
    return true;
}
