#ifndef EDS_DATA_H
#define EDS_DATA_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// --- Структура одного EDS-параметра с размером ---
typedef struct {
    uint16_t index;          // Индекс в объектном словаре
    uint8_t  subindex;       // Субиндекс
    uint32_t defaultValue;   // Значение по умолчанию
    uint8_t  size;           // Размер в байтах (1, 2, 4)
} EDS_Entry;

// --- Таблица параметров ---
extern const EDS_Entry eds_table[];
extern const uint16_t eds_table_size;

// --- Поиск значения по индексу и сабиндексу ---
const EDS_Entry* EDS_Find(uint16_t index, uint8_t subindex);

// --- Получение значения по умолчанию ---
uint32_t EDS_GetDefaultValue(uint16_t index, uint8_t subindex);

// --- Логгирование параметра по индексу ---
void EDS_Log(uint16_t index, uint8_t subindex);

// --- SDO-запись значения из EDS таблицы ---
HAL_StatusTypeDef CO_SDO_Write_ByEDS(CAN_HandleTypeDef *hcan, uint8_t node_id, uint16_t index, uint8_t subindex);

// --- SDO-чтение параметра с логом ---
HAL_StatusTypeDef CO_SDO_Read_ByEDS(CAN_HandleTypeDef *hcan, uint8_t node_id, uint16_t index, uint8_t subindex);

// --- Удобные команды (настраиваемые под твой проект) ---
#define SEND_TARGET_POSITION() CO_SDO_Write_ByEDS(&hcan, 0x03, 0x607A, 0x00)
#define LOG_TARGET_SPEED()     CO_SDO_Read_ByEDS(&hcan,  0x03, 0x606C, 0x00)

// В будущем: можно добавить EDS_UpdateValue(), если хочешь менять значения вручную

#endif // EDS_DATA_H
