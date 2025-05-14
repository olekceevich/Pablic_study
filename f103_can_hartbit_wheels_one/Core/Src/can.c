#include "can.h"
#include "stm32f1xx_hal_uart.h"
#include <stdio.h>
#include <string.h>
#include "CO_NMT.h"
#include "CO_SDO.h"
#include "COB_Dispatcher.h"

// --- Внешние переменные ---
extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart2;

// --- Прототипы ---
static void CAN_ConfigFilter(void);
extern void LogMessage(const char *message);

// --- Основная инициализация CAN ---
void CAN_Init(void) {
    LogMessage("CAN_Init: Starting CAN_ConfigFilter");
    CAN_ConfigFilter();
    LogMessage("CAN_Init: CAN_ConfigFilter Done");

    if (HAL_CAN_Start(&hcan) != HAL_OK) {
        LogMessage("CAN_Init: HAL_CAN_Start Error");
        Error_Handler();
    } else {
        LogMessage("CAN_Init: HAL_CAN_Start OK");
    }

    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        LogMessage("CAN_Init: HAL_CAN_ActivateNotification Error");
        Error_Handler();
    } else {
        LogMessage("CAN_Init: HAL_CAN_ActivateNotification OK");
    }
}

// --- Прерывание при приёме сообщения ---
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
        char msg[64];
        //sprintf(msg, "CAN IRQ: StdID=0x%03lX, Len=%lu", RxHeader.StdId, RxHeader.DLC);
        LogMessage(msg);

        COB_Dispatch(&RxHeader, RxData);
    } else {
        LogMessage("CAN IRQ: Failed to get message");
    }
}

// ---передал в COB_Dispatcher ---
void CAN_ProcessReceivedMessage(void) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8] = {0};

    if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) == 0) return;

    if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
        COB_Dispatch(&RxHeader, RxData);
    } else {
        LogMessage("ERROR: GetRxMessage failed, although FIFO is non-empty");
    }
}

// --- Конфигурация фильтра ---
static void CAN_ConfigFilter(void) {
    CAN_FilterTypeDef FilterConfig = {0};

    FilterConfig.FilterBank = 0;
    FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    FilterConfig.FilterIdHigh = 0x0000;
    FilterConfig.FilterIdLow = 0x0000;
    FilterConfig.FilterMaskIdHigh = 0x0000;
    FilterConfig.FilterMaskIdLow = 0x0000;
    FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    FilterConfig.FilterActivation = ENABLE;
    FilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan, &FilterConfig) != HAL_OK) {
        LogMessage("CAN_ConfigFilter: Filter configuration failed");
        Error_Handler();
    } else {
        LogMessage("CAN_ConfigFilter: Filter configured successfully");
    }
}
