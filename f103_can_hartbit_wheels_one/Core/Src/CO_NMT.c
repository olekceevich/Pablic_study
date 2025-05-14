#include "CO_NMT.h"
#include "can.h"
#include <stdio.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "COB_Dispatcher.h"

// --- Флаги состояния ---
volatile uint8_t activation_confirmed = 0;
volatile uint8_t canopen_confirmation_received = 0;

// --- Отправка команды активации  ---
void CAN_SendActivationCommand(CAN_HandleTypeDef *hcan, uint8_t node_id) {
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint8_t data[2] = {0x10, 0x10};
    uint32_t TxMailbox;

    TxHeader.StdId = 0x000 + node_id;
    TxHeader.DLC = 2;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;

    if (HAL_CAN_AddTxMessage(hcan, &TxHeader, data, &TxMailbox) != HAL_OK) {
        LogMessage("Activation command send failed");
        return;
    }

    LogMessage("Activation command 0x10 0x10 sent");

    // Ожидание ответа [AA BB] от колеса
    uint32_t startTick = HAL_GetTick();
    while (!activation_confirmed) {
        if (HAL_GetTick() - startTick > 3000) {
            LogMessage("Timeout waiting for activation confirmation");
            return;
        }
        HAL_Delay(10);
    }

    LogMessage("Activation confirmed by wheel (AA BB)");
}

// --- Отправка NMT команды ---
HAL_StatusTypeDef CO_NMT_Send(CAN_HandleTypeDef *hcan, uint8_t command, uint8_t node_id) {
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint8_t TxData[2] = {command, node_id};
    uint32_t TxMailbox;

    TxHeader.StdId = 0x000;
    TxHeader.DLC = 2;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;

    if (HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, &TxMailbox) == HAL_OK) {
        char debugMsg[64];
        snprintf(debugMsg, sizeof(debugMsg), "NMT command 0x%02X sent to Node %u, Mailbox=%lu", command, node_id, TxMailbox);
        LogMessage(debugMsg);
        return HAL_OK;
    } else {
        LogMessage("NMT send failed");
        return HAL_ERROR;
    }
}

// --- Обработка сообщений, относящихся к NMT (heartbeat, ответы на активацию,  подтверждение)
void CO_NMT_HandleMessage(uint16_t cob_id, uint8_t *data, uint8_t dlc) {
    if (dlc == 2 && data[0] == 0xAA && data[1] == 0xBB) {
        LogMessage("Activation response received: [AA, BB]");
        activation_confirmed = 1;
    } else if (dlc == 1 && data[0] == 0x7F) {
        LogMessage("CANopen confirmation received: [7F]");
        canopen_confirmation_received = 1;
    } else if (dlc >= 1 && cob_id >= 0x700 && cob_id <= 0x7FF) {
        char msg[64];
        uint8_t node_id = cob_id & 0x7F;
        snprintf(msg, sizeof(msg), "Heartbeat from Node %u: state 0x%02X", node_id, data[0]);
        LogMessage(msg);
    }
}
void CO_NMT_ReleaseBrake(CAN_HandleTypeDef *hcan, uint8_t node_id) {
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint8_t data[8] = {0x2F, 0x58, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00};
    uint32_t TxMailbox;

    TxHeader.StdId = 0x600 + node_id; // SDO для Node-ID
    TxHeader.DLC = 8;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;

    char msg[64];
    snprintf(msg, sizeof(msg), "Sending brake release to node %u", node_id);
    LogMessage(msg);

    if (HAL_CAN_AddTxMessage(hcan, &TxHeader, data, &TxMailbox) != HAL_OK) {
        LogMessage("⚠ Failed to send brake release");
        return;
    }

    // Можно добавить ожидание окончания отправки (необязательно)
    HAL_Delay(100);
    LogMessage("✅ Brake release command sent");
}
