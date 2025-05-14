#include "eds_data.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "can.h"
#include "CO_SDO.h"
#include "COB_Dispatcher.h"

/*
 * {0x6040, 0x00, 0x0000, 2}:

index = 0x6040 — индекс в Object Dictionary (OD) CANopen

subindex = 0x00

defaultValue = 0x0000

size = 2 байта (это uint16_t)
 */
// --- Таблица EDS объектов ---
const EDS_Entry eds_table[] = {
    {0x6040, 0x00, 0x0000, 2},       // Controlword (uint16)
    {0x6041, 0x00, 0x0000, 2},       // Statusword (uint16)
    {0x6060, 0x00, 0x03,   1},       // Mode of operation = 3 (velocity)
    {0x6061, 0x00, 0x00,   1},       // Mode display
    {0x60FF, 0x00, 0x00000000, 4},   // Target speed
    {0x606C, 0x00, 0x00000000, 4},   // Actual speed
    {0x607A, 0x00, 0x00000000, 4},   // Target position
    {0x6064, 0x00, 0x00000000, 4},   // Actual position
    {0x6081, 0x00, 10000, 4},        // Profile velocity
    {0x6083, 0x00, 3000, 4},         // Acceleration time (pos mode)
    {0x6084, 0x00, 3000, 4},         // Deceleration time (pos mode)
    {0x201C, 0x00, 3000, 4},         // Acceleration time (speed mode)
    {0x201D, 0x00, 3000, 4},         // Deceleration time (speed mode)
    {0x2058, 0x00, 0x03, 1},         // Brake release (0x03)
    // TPDO-related diagnostics (read-only)
    {0x3000, 0x00, 0x0000, 2},       // TPDO1 speed
    {0x3001, 0x00, 0x0000, 2},       // TPDO2 position
    {0x3002, 0x00, 0x0000, 2},       // TPDO3 torque or temp (example)
};

const uint16_t eds_table_size = sizeof(eds_table) / sizeof(EDS_Entry);

// --- Поиск параметра в таблице ---
const EDS_Entry* EDS_Find(uint16_t index, uint8_t subindex) {
    for (size_t i = 0; i < eds_table_size; i++) {
        if (eds_table[i].index == index && eds_table[i].subindex == subindex) {
        	//Ищем по паре index + subindex. Как только совпадает — возвращаем адрес:
            return &eds_table[i];
        }
    }
    return NULL;
}

// --- Получить значение по умолчанию ---
uint32_t EDS_GetDefaultValue(uint16_t index, uint8_t subindex) {
    const EDS_Entry* entry = EDS_Find(index, subindex);
    if (entry) {
        return entry->defaultValue;//Если найдено — вернём entry->defaultValue
    }
    return 0xFFFFFFFF;
}

// --- Логировать параметр ---
void EDS_Log(uint16_t index, uint8_t subindex) {
	const EDS_Entry* entry = EDS_Find(index, subindex);
	if (entry) {
		char msg[100];
		sprintf(msg, "EDS 0x%04X/0x%02X = 0x%08lX [%u bytes]",
/*
 * %04X — hex, 4 знака, дополняется нулями

%02X — hex, 2 знака

%08lX — 8 знаков (32 бита), long unsigned int

%u — unsigned int (размер в байтах)
*/
				index, subindex, entry->defaultValue, entry->size);
		LogMessage(msg);
	} else {
		LogMessage("EDS param not found");
	}
}

/*
Передаётся:

CAN

Node ID

Index + subindex

Значение по умолчанию

Размер (1, 2, 4 байта)

Функция возвращает HAL_StatusTypeDef (HAL_OK или HAL_ERROR).
 */
// --- Запись по EDS (использует defaultValue) ---
HAL_StatusTypeDef CO_SDO_Write_ByEDS(CAN_HandleTypeDef *hcan, uint8_t node_id, uint16_t index, uint8_t subindex) {
    const EDS_Entry* entry = EDS_Find(index, subindex);
    if (!entry) {
        LogMessage("EDS entry not found");
        return HAL_ERROR;
    }
    return CO_SDO_Write(hcan, node_id, index, subindex, entry->defaultValue, entry->size);
}

// --- Чтение параметра и логгирование ---
HAL_StatusTypeDef CO_SDO_Read_ByEDS(CAN_HandleTypeDef *hcan, uint8_t node_id, uint16_t index, uint8_t subindex) {
    uint32_t val = 0;
    if (CO_SDO_Read(hcan, node_id, index, subindex, &val) == HAL_OK) {
        char dbg[64];
        sprintf(dbg, "READ 0x%04X/0x%02X = 0x%08lX", index, subindex, val);
        LogMessage(dbg);
        return HAL_OK;
    } else {
        LogMessage("CO_SDO_Read_ByEDS failed");
        return HAL_ERROR;
    }
}
