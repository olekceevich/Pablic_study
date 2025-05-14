#include "CO_SDO.h"
#include "can.h"
#include <string.h>
#include <stdio.h>

#ifndef MAX_SDO_TRANSFERS
#define MAX_SDO_TRANSFERS 128
#endif

#define NODE_ID 0x01

void LogMessage(const char *msg);

SDO_Transfer SDO_Buffer[MAX_SDO_TRANSFERS];

void CO_SDO_HandleResponse(uint8_t node_id, uint8_t *data, uint8_t dlc) {
    if (node_id >= MAX_SDO_TRANSFERS || dlc < 8) return;
    SDO_Transfer *sdo = &SDO_Buffer[node_id];

    uint8_t cmd = data[0] & 0xE0;

    if (cmd == 0x40 || cmd == 0x43 || cmd == 0x47) {
        sdo->value = (data[4]) | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
        sdo->ready = 1;
        sdo->pending = 0;
    } else if (cmd == 0x80) {
        sdo->error = 1;
        sdo->pending = 0;
    }
}

HAL_StatusTypeDef CO_SDO_Read(CAN_HandleTypeDef *hcan, uint8_t node_id,
                               uint16_t index, uint8_t subindex, uint32_t *value) {
    if (node_id >= MAX_SDO_TRANSFERS) return HAL_ERROR;
    SDO_Transfer *sdo = &SDO_Buffer[node_id];

    CAN_TxHeaderTypeDef txHeader = {0};
    uint8_t txData[8] = {0x40, index & 0xFF, index >> 8, subindex, 0, 0, 0, 0};
    uint32_t mailbox;

    txHeader.StdId = 0x600 + node_id;
    txHeader.DLC = 8;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;

    sdo->pending = 1;
    sdo->ready = 0;
    sdo->error = 0;
    sdo->index = index;
    sdo->subindex = subindex;
    sdo->timestamp = HAL_GetTick();

    if (HAL_CAN_AddTxMessage(hcan, &txHeader, txData, &mailbox) != HAL_OK) {
        return HAL_ERROR;
    }

    uint32_t start = HAL_GetTick();
    while (!sdo->ready && !sdo->error && (HAL_GetTick() - start < 100)) {}

    if (sdo->ready) {
        *value = sdo->value;
        return HAL_OK;
    } else if (sdo->error) {
        LogMessage("CO_SDO_Read: Abort received");
        return HAL_ERROR;
    } else {
        //LogMessage("CO_SDO_Read: Timeout");
        return HAL_TIMEOUT;
    }
}

HAL_StatusTypeDef CO_SDO_Write(CAN_HandleTypeDef *hcan, uint8_t node_id,
                                uint16_t index, uint8_t subindex, uint32_t value, uint8_t size) {
    CAN_TxHeaderTypeDef txHeader = {0};
    uint8_t txData[8] = {0};
    uint32_t mailbox;

    txHeader.StdId = 0x600 + node_id;
    txHeader.DLC = 8;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;

    switch (size) {
        case 1: txData[0] = 0x2F; txData[4] = value; break;
        case 2: txData[0] = 0x2B; txData[4] = value; txData[5] = value >> 8; break;
        case 4: txData[0] = 0x23; txData[4] = value; txData[5] = value >> 8;
                txData[6] = value >> 16; txData[7] = value >> 24; break;
        default: return HAL_ERROR;
    }

    txData[1] = index & 0xFF;
    txData[2] = index >> 8;
    txData[3] = subindex;

    return HAL_CAN_AddTxMessage(hcan, &txHeader, txData, &mailbox);
}

HAL_StatusTypeDef CO_SDO_Init_SpeedMode(CAN_HandleTypeDef *hcan, uint8_t node_id) {
    LogMessage("=== CO_SDO_Init_SpeedMode (RAW based) ===");
    HAL_StatusTypeDef res = HAL_OK;

    // Устанавливаем режим скорости
    res |= CO_SDO_Write(hcan, node_id, 0x6060, 0x00, 0x03, 1); // Speed mode
    HAL_Delay(50);

    // Поэтапная активация двигателя через Controlword
    uint16_t control_words[] = {0x0080, 0x0006, 0x0007, 0x000F};
    for (int i = 0; i < 4; i++) {
        res |= CO_SDO_Write(hcan, node_id, 0x6040, 0x00, control_words[i], 2);
        HAL_Delay(50); // ✅ Тут задержка нужна между этапами включения
    }

    // ✅ УСТАНОВКА ВРЕМЕНИ РАЗГОНА/ТОРМОЖЕНИЯ
    res |= CO_SDO_Write(hcan, node_id, 0x201C, 0x00, 400, 4);  // Acceleration time = 1000 мс
    HAL_Delay(20);
    res |= CO_SDO_Write(hcan, node_id, 0x201D, 0x00, 1000, 4);  // Deceleration time = 1000 мс
    HAL_Delay(20);

    // ✅ УСТАНОВКА СКОРОСТИ
    int32_t speed = 20000;  // Пониженная скорость
    res |= CO_SDO_Write(hcan, node_id, 0x60FF, 0x00, (uint32_t)speed, 4);
    HAL_Delay(5000);  // Примерная задержка (время вращения)

    // ✅ ОСТАНОВКА
    res |= CO_SDO_Write(hcan, node_id, 0x60FF, 0x00, 0x00000000, 4);
    HAL_Delay(100);

    LogMessage(res == HAL_OK ? "✅ SpeedMode RAW success" : "❌ SpeedMode RAW failed");
    return res;
}

/*
 *
 *
 * 400 → быстро

1000 → нормально

3000 → очень плавно (рекомендуется, если мотор «отваливается»)

 *
 * 📌 Примеры:

target_speed = 50000 — медленно

target_speed = 100000 — средне

target_speed = 300000+ — быстро (может отлетать мотор, если резкий старт)
 *
 *
 *
 */




// Existing PositionMode kept below...
/*

HAL_StatusTypeDef CO_SDO_Init_PositionMode(CAN_HandleTypeDef *hcan, uint8_t node_id) {
    LogMessage("=== CO_SDO_Init_PositionMode ===");
    HAL_StatusTypeDef res = HAL_OK;

    // 1. Clear Fault
    res = CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x0080, 2);
    if (res != HAL_OK) { LogMessage("❌ Step 1: Clear Fault failed"); return res; }
    HAL_Delay(10);

    // 2. Set Mode of Operation = 0x01 (Position)
    res = CO_SDO_Write(hcan, node_id, 0x6060, 0x00, 0x01, 1);
    if (res != HAL_OK) { LogMessage("❌ Step 2: Set Position Mode failed"); return res; }
    HAL_Delay(10);

    // 3. Disable RPDO1
    res = CO_SDO_Write(hcan, node_id, 0x1400, 0x01, 0x80000200 + node_id, 4);
    if (res != HAL_OK) { LogMessage("❌ Step 3: Disable RPDO1 failed"); return res; }
    HAL_Delay(10);

    // 4. Clear map
    res = CO_SDO_Write(hcan, node_id, 0x1600, 0x00, 0x00, 1);
    if (res != HAL_OK) { LogMessage("❌ Step 4: Clear RPDO map failed"); return res; }
    HAL_Delay(10);

    // 5. Map 6040 (Controlword)
    res = CO_SDO_Write(hcan, node_id, 0x1600, 0x01, 0x60400010, 4);
    if (res != HAL_OK) { LogMessage("❌ Step 5: Map Controlword failed"); return res; }
    HAL_Delay(10);

    // 6. Map 607A (Target Position)
    res = CO_SDO_Write(hcan, node_id, 0x1600, 0x02, 0x607A0020, 4);
    if (res != HAL_OK) { LogMessage("❌ Step 6: Map Target Position failed"); return res; }
    HAL_Delay(10);

    // 7. Confirm map count
    res = CO_SDO_Write(hcan, node_id, 0x1600, 0x00, 0x02, 1);
    if (res != HAL_OK) { LogMessage("❌ Step 7: Confirm map count failed"); return res; }
    HAL_Delay(10);

    // 8. Enable RPDO1
    res = CO_SDO_Write(hcan, node_id, 0x1400, 0x01, 0x00000200 + node_id, 4);
    if (res != HAL_OK) { LogMessage("❌ Step 8: Enable RPDO1 failed"); return res; }
    HAL_Delay(10);

    // 9. Transmission type = 1 (SYNC)
    res = CO_SDO_Write(hcan, node_id, 0x1400, 0x02, 0x01, 1);
    if (res != HAL_OK) { LogMessage("❌ Step 9: Set transmission type failed"); return res; }
    HAL_Delay(10);

    // 10. Start motor
    res = CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x0006, 2); // Shutdown
    if (res != HAL_OK) { LogMessage("❌ Step 10a: Shutdown failed"); return res; }
    HAL_Delay(10);

    res = CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x0007, 2); // Switch on
    if (res != HAL_OK) { LogMessage("❌ Step 10b: Switch On failed"); return res; }
    HAL_Delay(10);

    res = CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x000F, 2); // Operation Enabled
    if (res != HAL_OK) { LogMessage("❌ Step 10c: Enable Operation failed"); return res; }
    HAL_Delay(10);

    LogMessage("✅ CO_SDO_Init_PositionMode SUCCESS");
    return HAL_OK;
}
*/


// === Init for Position Mode (0x01) ===
HAL_StatusTypeDef CO_SDO_Init_PositionMode(CAN_HandleTypeDef *hcan, uint8_t node_id) {
    LogMessage("=== CO_SDO_Init_PositionMode ===");
    HAL_StatusTypeDef res = HAL_OK;

    res |= CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x0080, 2); // Clear fault
    HAL_Delay(10);
    res |= CO_SDO_Write(hcan, node_id, 0x6060, 0x00, 0x01, 1);   // Mode = position
    HAL_Delay(10);

    res |= CO_SDO_Write(hcan, node_id, 0x1400, 0x01, 0x80000200 + node_id, 4);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x00, 0x00, 1);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x01, 0x60400010, 4);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x02, 0x607A0020, 4);
    res |= CO_SDO_Write(hcan, node_id, 0x1600, 0x00, 0x02, 1);
    res |= CO_SDO_Write(hcan, node_id, 0x1400, 0x01, 0x00000200 + node_id, 4);
    res |= CO_SDO_Write(hcan, node_id, 0x1400, 0x02, 0x01, 1);

    res |= CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x0006, 2);
    res |= CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x0007, 2);
    res |= CO_SDO_Write(hcan, node_id, 0x6040, 0x00, 0x000F, 2);

    LogMessage(res == HAL_OK ? "✅ Position mode init OK" : "❌ Position mode init FAIL");
    return res;
}

