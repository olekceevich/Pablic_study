#ifndef CAN_H
#define CAN_H

#include "stm32f1xx_hal.h"
#include "main.h"  // Если global hcan, huart2 здесь

// --- Основные функции ---
void CAN_Init(void);
void CAN_ProcessReceivedMessage(void);

// --- Логгирование ---
void LogMessage(const char *message);
void LogMessage_Once(const char *msg);

// --- Обработчик ошибок ---
void Error_Handler(void);

// --- Глобальные переменные ---
extern CAN_HandleTypeDef hcan;

// --- HAL callback (если используется глобально) ---
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#endif // CAN_H
