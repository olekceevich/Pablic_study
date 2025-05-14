#ifndef INC_CO_NMT_H_
#define INC_CO_NMT_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

// Команды NMT (стандарт)
#define NMT_START           0x01
#define NMT_STOP            0x02
#define NMT_PRE_OPERATIONAL 0x80
#define NMT_RESET_NODE      0x81
#define NMT_RESET_COMM      0x82

// Отправка NMT-команды (Reset, Start и т.д.)
HAL_StatusTypeDef CO_NMT_Send(CAN_HandleTypeDef *hcan, uint8_t command, uint8_t node_id);

// Отправка команды активации (0x10 0x10), ожидание подтверждения
void CAN_SendActivationCommand(CAN_HandleTypeDef *hcan, uint8_t node_id);

void CO_NMT_HandleMessage(uint16_t cob_id, uint8_t *data, uint8_t dlc);

// Обработка heartbeat и подтверждающих сообщений (0x7F, AA BB)
void CO_NMT_HandleHeartbeatOrAck(uint8_t node_id, uint8_t *data, uint8_t dlc);
void CO_NMT_ReleaseBrake(CAN_HandleTypeDef *hcan, uint8_t node_id);

// Глобальные флаги для отслеживания состояния и подтверждений
extern volatile uint8_t activation_confirmed;
extern volatile uint8_t canopen_confirmation_received;

#endif /* INC_CO_NMT_H_ */
