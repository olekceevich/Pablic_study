#ifndef INC_COB_DISPATCHER_H_
#define INC_COB_DISPATCHER_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

extern CAN_HandleTypeDef hcan;


// --- Режимы управления ---
typedef enum {
    MODE_SPEED = 0,
    MODE_POSITION = 1
} ControlMode;

extern ControlMode active_mode;  // Выбор режима из main.c

// --- CANopen состояния ---
typedef enum {
    COB_STATE_INIT = 0,
    COB_STATE_NMT_SENT,
    COB_STATE_NMT_CONFIRMED,
    COB_STATE_SDO_SENT,
    COB_STATE_SDO_CONFIRMED,
    COB_STATE_PDO_ACTIVE,
    COB_STATE_ERROR
} COB_State;

// --- Интерфейсы ---
void COB_Dispatch(CAN_RxHeaderTypeDef *header, uint8_t *data);
void CO_SDO_HandleResponse(uint8_t node_id, uint8_t *data, uint8_t dlc);
void CO_NMT_HandleHeartbeat(uint8_t node_id, uint8_t *data, uint8_t dlc);


void LogMessage(const char *msg);

void COB_ProcessStateMachine(void);
COB_State COB_GetState(void);
void COB_SetState(COB_State state);

#endif /* INC_COB_DISPATCHER_H_ */
