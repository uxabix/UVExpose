#include "Services/presets_service.h"
#include "Services/flash_storage.h"
#include "config.h"

#include <stdbool.h>
#include <string.h>

#define PRESET_FLAG_BUZZER_MASK 0x03u
#define PRESET_FLAG_UNTIL_OFF   0x04u
#define PRESET_FLAG_ACTIVE      0x08u

/*
 * STM32F1 flash write granularity is half-word, so we keep one padding byte.
 * Logical record is still {minutes, seconds, flags}.
 */
#define PRESET_RECORD_STORAGE_SIZE 4u
#define PRESET_PAGE_CAPACITY (PRESETS_FLASH_PAGE_SIZE / PRESET_RECORD_STORAGE_SIZE)

typedef struct {
    uint8_t minutes;
    uint8_t seconds;
    uint8_t flags;
    uint8_t pad; // always 0xFF
} preset_record_storage_t;

typedef struct {
    preset_t preset;
    uint16_t flash_offset;
} preset_entry_t;

static preset_entry_t g_presets[PRESETS_MAX_COUNT];
static uint8_t g_presets_count = 0u;
static uint8_t g_has_deleted_records = 0u;

static bool preset_is_valid(const preset_t* preset)
{
    if (preset == NULL) {
        return false;
    }

    return (preset->minutes <= 99u) &&
           (preset->seconds <= 59u) &&
           (preset->buzzer_mode <= 2u) &&
           (preset->until_off <= 1u);
}

static bool preset_equals(const preset_t* a, const preset_t* b)
{
    if (a == NULL || b == NULL) {
        return false;
    }

    return (a->minutes == b->minutes) &&
           (a->seconds == b->seconds) &&
           (a->buzzer_mode == b->buzzer_mode) &&
           (a->until_off == b->until_off);
}

static uint8_t preset_flags_encode(const preset_t* preset, uint8_t active)
{
    uint8_t flags = (uint8_t)(preset->buzzer_mode & PRESET_FLAG_BUZZER_MASK);
    if (preset->until_off != 0u) {
        flags |= PRESET_FLAG_UNTIL_OFF;
    }
    if (active != 0u) {
        flags |= PRESET_FLAG_ACTIVE;
    }
    return flags;
}

static bool preset_record_decode(const preset_record_storage_t* record, preset_t* out_preset, uint8_t* out_active)
{
    if (record == NULL || out_preset == NULL || out_active == NULL) {
        return false;
    }

    out_preset->minutes = record->minutes;
    out_preset->seconds = record->seconds;
    out_preset->buzzer_mode = (uint8_t)(record->flags & PRESET_FLAG_BUZZER_MASK);
    out_preset->until_off = (uint8_t)((record->flags & PRESET_FLAG_UNTIL_OFF) != 0u);
    *out_active = (uint8_t)((record->flags & PRESET_FLAG_ACTIVE) != 0u);

    return preset_is_valid(out_preset);
}

static uint8_t find_preset_index(const preset_t* preset)
{
    for (uint8_t i = 0u; i < g_presets_count; ++i) {
        if (preset_equals(&g_presets[i].preset, preset)) {
            return i;
        }
    }

    return 0xFFu;
}

static void remove_preset_by_index_internal(uint8_t index)
{
    if (index >= g_presets_count) {
        return;
    }

    for (uint8_t i = index; (i + 1u) < g_presets_count; ++i) {
        g_presets[i] = g_presets[i + 1u];
    }

    if (g_presets_count > 0u) {
        g_presets_count--;
    }
}

static bool record_is_erased(const preset_record_storage_t* record)
{
    return (record->minutes == 0xFFu) &&
           (record->seconds == 0xFFu) &&
           (record->flags == 0xFFu) &&
           (record->pad == 0xFFu);
}

static bool write_record_at_offset(uint16_t offset, const preset_t* preset, uint8_t active)
{
    preset_record_storage_t record;
    record.minutes = preset->minutes;
    record.seconds = preset->seconds;
    record.flags = preset_flags_encode(preset, active);
    record.pad = 0xFFu;

    uint32_t addr = PRESETS_FLASH_ADDR + offset;
    uint16_t first_halfword = (uint16_t)(((uint16_t)record.seconds << 8u) | record.minutes);
    uint16_t second_halfword = (uint16_t)(((uint16_t)record.pad << 8u) | record.flags);

    if (!FlashStorage_ProgramHalfWord(addr, first_halfword)) {
        return false;
    }
    if (!FlashStorage_ProgramHalfWord(addr + 2u, second_halfword)) {
        return false;
    }

    return true;
}

static uint16_t find_first_erased_offset(void)
{
    preset_record_storage_t record;

    for (uint16_t i = 0u; i < PRESET_PAGE_CAPACITY; ++i) {
        uint16_t offset = (uint16_t)(i * PRESET_RECORD_STORAGE_SIZE);
        if (!FlashStorage_Read(PRESETS_FLASH_ADDR + offset, &record, sizeof(record))) {
            return 0xFFFFu;
        }

        if (record_is_erased(&record)) {
            return offset;
        }
    }

    return 0xFFFFu;
}

static bool presets_gc_rewrite_active_page(void)
{
    preset_entry_t entries_copy[PRESETS_MAX_COUNT];
    uint8_t count_copy = g_presets_count;
    memcpy(entries_copy, g_presets, sizeof(entries_copy));

    if (!FlashStorage_ErasePage(PRESETS_FLASH_ADDR)) {
        return false;
    }

    for (uint8_t i = 0u; i < count_copy; ++i) {
        uint16_t offset = (uint16_t)(i * PRESET_RECORD_STORAGE_SIZE);
        if (!write_record_at_offset(offset, &entries_copy[i].preset, 1u)) {
            return false;
        }
        g_presets[i].preset = entries_copy[i].preset;
        g_presets[i].flash_offset = offset;
    }

    g_presets_count = count_copy;
    g_has_deleted_records = 0u;
    return true;
}

static bool presets_format_page(void)
{
    g_presets_count = 0u;
    g_has_deleted_records = 0u;
    return FlashStorage_ErasePage(PRESETS_FLASH_ADDR);
}

static presets_status_t append_journal_record(const preset_t* preset, uint8_t active, uint16_t* out_offset)
{
    uint16_t offset = find_first_erased_offset();
    if (offset == 0xFFFFu) {
        // Recovery path: page has no erased slots and no active/deleted state in RAM.
        // This usually means legacy garbage in flash page before presets feature.
        if (g_presets_count == 0u && g_has_deleted_records == 0u) {
            if (!presets_format_page()) {
                return PRESETS_STATUS_ERROR;
            }
            offset = 0u;
        }
    }

    if (offset == 0xFFFFu) {
        if (g_has_deleted_records == 0u) {
            return PRESETS_STATUS_FULL;
        }

        if (!presets_gc_rewrite_active_page()) {
            return PRESETS_STATUS_ERROR;
        }

        offset = find_first_erased_offset();
        if (offset == 0xFFFFu) {
            return PRESETS_STATUS_FULL;
        }
    }

    if (!write_record_at_offset(offset, preset, active)) {
        return PRESETS_STATUS_ERROR;
    }

    if (out_offset != NULL) {
        *out_offset = offset;
    }

    return PRESETS_STATUS_OK;
}

void Presets_Init(void)
{
    g_presets_count = 0u;
    g_has_deleted_records = 0u;

    preset_record_storage_t record;

    for (uint16_t i = 0u; i < PRESET_PAGE_CAPACITY; ++i) {
        uint16_t offset = (uint16_t)(i * PRESET_RECORD_STORAGE_SIZE);
        if (!FlashStorage_Read(PRESETS_FLASH_ADDR + offset, &record, sizeof(record))) {
            break;
        }

        if (record_is_erased(&record)) {
            break;
        }

        preset_t decoded_preset;
        uint8_t active = 0u;
        if (!preset_record_decode(&record, &decoded_preset, &active)) {
            // If first slot is invalid, treat page as legacy/garbage and format it.
            if (i == 0u) {
                (void)presets_format_page();
            }
            // Partial/corrupted tail record: stop replay.
            break;
        }

        uint8_t existing_index = find_preset_index(&decoded_preset);

        if (active != 0u) {
            if (existing_index != 0xFFu) {
                g_presets[existing_index].flash_offset = offset;
            } else if (g_presets_count < PRESETS_MAX_COUNT) {
                g_presets[g_presets_count].preset = decoded_preset;
                g_presets[g_presets_count].flash_offset = offset;
                g_presets_count++;
            }
        } else {
            g_has_deleted_records = 1u;
            if (existing_index != 0xFFu) {
                remove_preset_by_index_internal(existing_index);
            }
        }
    }
}

uint8_t Presets_Count(void)
{
    return g_presets_count;
}

presets_status_t Presets_Get(uint8_t index, preset_t* out_preset)
{
    if (out_preset == NULL) {
        return PRESETS_STATUS_INVALID_ARG;
    }
    if (index >= g_presets_count) {
        return PRESETS_STATUS_INVALID_ARG;
    }

    *out_preset = g_presets[index].preset;
    return PRESETS_STATUS_OK;
}

presets_status_t Presets_Add(const preset_t* preset)
{
    if (!preset_is_valid(preset)) {
        return PRESETS_STATUS_INVALID_ARG;
    }

    if (find_preset_index(preset) != 0xFFu) {
        return PRESETS_STATUS_DUPLICATE;
    }

    if (g_presets_count >= PRESETS_MAX_COUNT) {
        return PRESETS_STATUS_FULL;
    }

    uint16_t new_record_offset = 0u;
    presets_status_t append_status = append_journal_record(preset, 1u, &new_record_offset);
    if (append_status != PRESETS_STATUS_OK) {
        return append_status;
    }

    g_presets[g_presets_count].preset = *preset;
    g_presets[g_presets_count].flash_offset = new_record_offset;
    g_presets_count++;

    return PRESETS_STATUS_OK;
}

presets_status_t Presets_Delete(uint8_t index)
{
    if (index >= g_presets_count) {
        return PRESETS_STATUS_INVALID_ARG;
    }

    preset_t deleted_preset = g_presets[index].preset;
    presets_status_t append_status = append_journal_record(&deleted_preset, 0u, NULL);
    if (append_status != PRESETS_STATUS_OK) {
        return append_status;
    }

    remove_preset_by_index_internal(index);
    g_has_deleted_records = 1u;

    return PRESETS_STATUS_OK;
}
