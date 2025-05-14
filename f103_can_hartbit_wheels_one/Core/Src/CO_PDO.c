#include "CO_PDO.h"
#include "CO_SDO.h"
#include "CO_SYNC.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

extern void LogMessage(const char *);

PDO_Data_t pdo_nodes[MAX_NODES];
uint8_t pdo_received[MAX_NODES] = {0};

// --- Обработка входящего PDO-сообщения ---
void CO_PDO_Handle(uint16_t cob_id, uint8_t *data, uint8_t dlc) {
    uint8_t node_id = cob_id & 0x7F;
    if (node_id >= MAX_NODES) return;

    // Определение номера PDO (0 — TPDO1, 1 — TPDO2 и т.д.)
    uint16_t base_id = cob_id & 0xF80;  // отсекаем nodeID
    uint8_t pdo_num = (base_id - 0x180) / 0x100;  // 0, 1, 2, 3

    if (pdo_num >= 4) return;

    memcpy(pdo_nodes[node_id].pdo_data[pdo_num], data, dlc);
    pdo_nodes[node_id].pdo_len[pdo_num] = dlc;
    pdo_received[node_id] = 1;
}

// --- Отладочный вывод PDO1 ---
void CO_PDO_DebugPrint(uint8_t node_id) {
    if (node_id >= MAX_NODES) return;

    char dbg[128];
    snprintf(dbg, sizeof(dbg), "PDO1 Node %u: %02X %02X %02X %02X",
             node_id,
             pdo_nodes[node_id].pdo_data[0][0],
             pdo_nodes[node_id].pdo_data[0][1],
             pdo_nodes[node_id].pdo_data[0][2],
             pdo_nodes[node_id].pdo_data[0][3]);
    LogMessage(dbg);
}

// --- Получить указатель на данные PDO ---
uint8_t* CO_PDO_Get(uint8_t node_id, uint8_t pdo_num) {
    if (node_id >= MAX_NODES || pdo_num >= 4) return NULL;
    return pdo_nodes[node_id].pdo_data[pdo_num];
}

// --- Проверка: был ли получен хотя бы 1 PDO от узла ---
uint8_t CO_PDO_IsReceived(uint8_t node_id) {
    return (node_id < MAX_NODES) ? pdo_received[node_id] : 0;
}

// --- Настройка RPDO для режима скорости ---
HAL_StatusTypeDef CO_PDO_ConfigForSpeedMode(CAN_HandleTypeDef *hcan, uint8_t node_id) {
    HAL_StatusTypeDef res = HAL_OK;

    res |= CO_SDO_Write(hcan, node_id, 0x1400, 0x01, 0x80000200 + node_id, 4);  // Disable RPDO1
    HAL_Delay(10);
    res |= CO_SDO_Write(hcan, node_id, 0x1400, 0x02, 1, 1);                    // Transmission type = 1 (SYNC)
    HAL_Delay(10);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x00, 0, 1);                     // Clear mapping
    HAL_Delay(10);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x01, 0x60400010, 4);           // Map Controlword
    HAL_Delay(10);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x02, 0x60FF0020, 4);           // Map Target Speed
    HAL_Delay(10);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x00, 2, 1);                    // Confirm map count = 2
    HAL_Delay(10);
    res |= CO_SDO_Write(hcan, node_id, 0x1400, 0x01, 0x00000200 + node_id, 4); // Enable RPDO1
    HAL_Delay(10);

    LogMessage(res == HAL_OK ? "✅ RPDO Speed config OK" : "❌ RPDO Speed config FAIL");
    return res;
}

// --- Запуск мотора через RPDO-последовательность ---
void CO_PDO_StartupSequence_SpeedMode(CAN_HandleTypeDef *hcan, uint8_t node_id, int32_t target_speed) {
    LogMessage("🔁 Starting RPDO FSM activation...");
    uint16_t sequence[] = {0x0080, 0x0006, 0x0007, 0x000F};
    for (int i = 0; i < 4; i++) {
        char dbg[64];
        snprintf(dbg, sizeof(dbg), "  → RPDO Step %d: 0x%04X", i, sequence[i]);
        LogMessage(dbg);
        CO_SYNC_SendRPDOAndSYNC(hcan, node_id, sequence[i], target_speed);
        HAL_Delay(100);  // даём мотору переварить
    }
    LogMessage("✅ RPDO FSM complete");
}
