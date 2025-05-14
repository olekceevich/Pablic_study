#ifndef CO_SDO_H
#define CO_SDO_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// Максимальное количество отслеживаемых SDO-запросов (по Node-ID)
#define MAX_SDO_TRANSFERS 128

// Структура для хранения статуса одного SDO-транзакта
typedef struct {
    uint8_t pending;     // Ожидается ли ответ
    uint8_t ready;       // Ответ получен
    uint8_t error;       // Ответ с ошибкой (Abort)
    uint16_t index;      // Последний адресный индекс
    uint8_t subindex;    // Последний субиндекс
    uint32_t value;      // Принятое значение (если Read)
    uint32_t timestamp;  // Метка времени запроса
} SDO_Transfer;

// Глобальный буфер состояния для каждого узла
extern SDO_Transfer SDO_Buffer[MAX_SDO_TRANSFERS];

// --- Чтение значения из CANopen-устройства через SDO Upload ---
HAL_StatusTypeDef CO_SDO_Read(CAN_HandleTypeDef *hcan, uint8_t node_id,
                               uint16_t index, uint8_t subindex, uint32_t *value);

// --- Запись значения в CANopen-устройство через SDO Download ---
HAL_StatusTypeDef CO_SDO_Write(CAN_HandleTypeDef *hcan, uint8_t node_id,
                                uint16_t index, uint8_t subindex, uint32_t value, uint8_t size);

// --- Отдельная инициализация режимов ---
HAL_StatusTypeDef CO_SDO_Init_SpeedMode(CAN_HandleTypeDef *hcan, uint8_t node_id);
HAL_StatusTypeDef CO_SDO_Init_PositionMode(CAN_HandleTypeDef *hcan, uint8_t node_id);

// --- Обработка входящего ответа на SDO (из COB_Dispatch) ---
void CO_SDO_HandleResponse(uint8_t node_id, uint8_t *data, uint8_t dlc);

// --- Отладочный вывод данных CAN-фрейма ---
void CO_SDO_DebugLog(uint8_t *data, uint8_t len, const char *prefix);

#endif // CO_SDO_H
