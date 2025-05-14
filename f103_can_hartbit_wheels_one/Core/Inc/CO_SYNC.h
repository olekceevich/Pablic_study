#ifndef CO_SYNC_H
#define CO_SYNC_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

void CO_SYNC_SetEnabled(uint8_t enabled);
uint8_t CO_SYNC_IsEnabled(void);

void CO_SYNC_Send(CAN_HandleTypeDef *hcan);
void CO_SYNC_SendRPDOAndSYNC(CAN_HandleTypeDef *hcan, uint8_t node_id, uint16_t controlword, int32_t value);

void CO_SYNC_ResetStep(void);
extern uint16_t sync_step_counter;

#endif // CO_SYNC_H
