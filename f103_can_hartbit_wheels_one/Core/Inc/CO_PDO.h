#ifndef CO_PDO_H
#define CO_PDO_H

#include <stdint.h>
#include "stm32f1xx_hal.h"  // обязательно, если используешь HAL_StatusTypeDef

#define MAX_NODES 8

// --- Хранилище для PDO каждого узла ---
typedef struct {
    uint8_t pdo_data[4][8];  // до 4 PDO, каждый по 8 байт
    uint8_t pdo_len[4];      // длина каждого PDO
} PDO_Data_t;

// --- Глобальные переменные ---
extern PDO_Data_t pdo_nodes[MAX_NODES];
extern uint8_t pdo_received[MAX_NODES];

// --- Основной обработчик входящих PDO сообщений ---
void CO_PDO_Handle(uint16_t cob_id, uint8_t *data, uint8_t dlc);

// --- Вывод отладочной информации (только PDO1) ---
void CO_PDO_DebugPrint(uint8_t node_id);

// --- Получение указателя на конкретный PDO ---
uint8_t* CO_PDO_Get(uint8_t node_id, uint8_t pdo_num);

// --- Проверка получения хотя бы одного PDO ---
uint8_t CO_PDO_IsReceived(uint8_t node_id);

// --- Конфигурация RPDO для Speed Mode ---
HAL_StatusTypeDef CO_PDO_ConfigForSpeedMode(CAN_HandleTypeDef *hcan, uint8_t node_id);

// --- Конфигурация RPDO для Speed Mode ---
HAL_StatusTypeDef CO_PDO_ConfigForSpeedMode(CAN_HandleTypeDef *hcan, uint8_t node_id);

// --- Последовательная активация мотора через RPDO (FSM) ---
void CO_PDO_StartupSequence_SpeedMode(CAN_HandleTypeDef *hcan, uint8_t node_id, int32_t target_speed);

#endif // CO_PDO_H

