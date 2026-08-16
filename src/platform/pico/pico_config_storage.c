#include "pico_config_storage.h"

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"
#include "pico/platform.h"

#include <string.h>

_Static_assert(CONFIG_JOURNAL_SLOT_COUNT == 2u,
    "Pico storage reserves exactly two Flash sectors");
_Static_assert(CONFIG_JOURNAL_RECORD_SIZE <= FLASH_PAGE_SIZE,
    "configuration record must fit one Flash page");

static uint32_t slot_offset(uint8_t slot)
{
    return (uint32_t)PICO_FLASH_SIZE_BYTES -
        ((uint32_t)CONFIG_JOURNAL_SLOT_COUNT - slot) * FLASH_SECTOR_SIZE;
}
static bool read_slot(
    void *context,
    uint8_t slot,
    uint8_t record[CONFIG_JOURNAL_RECORD_SIZE])
{
    const uint8_t *source;
    (void)context;

    if (slot >= CONFIG_JOURNAL_SLOT_COUNT || record == NULL) {
        return false;
    }

    source = (const uint8_t *)(XIP_BASE + slot_offset(slot));
    memcpy(record, source, CONFIG_JOURNAL_RECORD_SIZE);
    return true;
}

static bool write_slot(
    void *context,
    uint8_t slot,
    const uint8_t record[CONFIG_JOURNAL_RECORD_SIZE])
{
    uint8_t page[FLASH_PAGE_SIZE];
    uint32_t interrupt_state;
    const uint32_t offset = slot_offset(slot);
    const uint8_t *written = (const uint8_t *)(XIP_BASE + offset);
    (void)context;

    if (slot >= CONFIG_JOURNAL_SLOT_COUNT || record == NULL) {
        return false;
    }

    memset(page, 0xFF, sizeof(page));
    memcpy(page, record, CONFIG_JOURNAL_RECORD_SIZE);

    interrupt_state = save_and_disable_interrupts();
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
    flash_range_program(offset, page, sizeof(page));
    restore_interrupts(interrupt_state);

    return memcmp(written, record, CONFIG_JOURNAL_RECORD_SIZE) == 0;
}

config_journal_storage_t pico_config_storage(void)
{
    const config_journal_storage_t storage = {
        .read_slot = read_slot,
        .write_slot = write_slot,
        .context = NULL,
    };
    return storage;
}
