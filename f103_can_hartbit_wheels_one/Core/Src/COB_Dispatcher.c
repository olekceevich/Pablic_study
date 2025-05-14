#include "COB_Dispatcher.h"
#include "CO_SDO.h"
#include "CO_NMT.h"
#include "CO_PDO.h"
#include "CO_SYNC.h"
#include <stdio.h>
#include <string.h>

extern uint8_t sync_enabled;  // внешний флаг из CO_SYNC
extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart2;

#define NODE_ID 0x01  // <-- Вынесено в константу

static COB_State current_state = COB_STATE_INIT;


void LogMessage(const char *msg) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s\r\n", msg);
    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void LogMessage_Once(const char *msg) {
    static char last_msg[128] = "";
    if (strcmp(last_msg, msg) != 0) {
        LogMessage(msg);
        strncpy(last_msg, msg, sizeof(last_msg));
    }
}

void COB_ProcessStateMachine(void) {
    switch (current_state) {
        case COB_STATE_INIT:
            current_state = COB_STATE_NMT_SENT;
            LogMessage("[COB] State: INIT → NMT_SENT");
            break;

        case COB_STATE_NMT_CONFIRMED:
            LogMessage("[COB] Waiting for external SDO Init (main.c)");
            break;

        case COB_STATE_SDO_CONFIRMED:
            current_state = COB_STATE_PDO_ACTIVE;
            LogMessage("[COB] State: SDO_CONFIRMED → PDO_ACTIVE");
            break;

        case COB_STATE_ERROR:
            LogMessage("[COB] State: ERROR");
            break;

        default:
            break;
    }
}

void COB_Dispatch(CAN_RxHeaderTypeDef *header, uint8_t *data) {
    uint16_t cob_id = header->StdId;

    char raw[64];
    snprintf(raw, sizeof(raw), "COB_Dispatch: ID=0x%03X", cob_id);
    LogMessage(raw);

    if (cob_id == 0x080) {
        LogMessage("SYNC frame received");
    } else if (cob_id >= 0x180 && cob_id <= 0x4FF) {
        CO_PDO_Handle(cob_id, data, header->DLC);
    } else if (cob_id >= 0x580 && cob_id <= 0x5FF) {
        LogMessage("TxSDO received");
        CO_SDO_HandleResponse(cob_id & 0x7F, data, header->DLC);

        if (current_state == COB_STATE_SDO_SENT) {
            current_state = COB_STATE_SDO_CONFIRMED;
            LogMessage("[COB] SDO confirmed → SDO_CONFIRMED");
        }
    } else if (cob_id >= 0x700 && cob_id <= 0x7FF) {
        CO_NMT_HandleMessage(cob_id, data, header->DLC);
    } else {
        LogMessage("Unknown COB-ID");
    }

    // ⚠ УБИРАЕМ автоматическую инициализацию SDO Init Phase (теперь только из main.c)
    // оставляем только управление SYNC если нужно
    if (current_state == COB_STATE_SDO_CONFIRMED && !sync_enabled) {
        LogMessage("[COB] Enabling SYNC after SDO Init...");
        CO_SYNC_SetEnabled(1);
        sync_enabled = 1;
        current_state = COB_STATE_PDO_ACTIVE;
        LogMessage("[COB] System is now OPERATIONAL");
    }
}

void handle_heartbeat(uint8_t node_id, uint8_t *data, uint8_t dlc) {
    if (dlc >= 1) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Heartbeat from Node %u: state 0x%02X", node_id, data[0]);
        LogMessage(msg);

        if (node_id == NODE_ID && data[0] == 0x7F) {
            if (current_state == COB_STATE_NMT_SENT) {
                current_state = COB_STATE_NMT_CONFIRMED;
                LogMessage("[COB] Heartbeat OK → NMT_CONFIRMED");
            }
        }
    }
}

void COB_SetState(COB_State state) {
    char msg[64];
    snprintf(msg, sizeof(msg), "State changed: %d -> %d", current_state, state);
    LogMessage(msg);
    current_state = state;
}
/*to do
 *
 *
 * uint8_t Motor_IsReady() {
    return (current_statusword & 0x0040) != 0;  // Example bit: Ready to Switch On
}

uint8_t Motor_IsEnabled() {
    return (current_statusword & 0x0200) != 0;  // Example bit: Operation Enabled
}

uint8_t Motor_IsFault() {
    return (current_statusword & 0x0008) != 0;  // Example bit: Fault
}
 *
 *
 *⚙️ Что такое Statusword (0x6041)?
Это один из самых главных статусных регистров у CANopen-устройства, обычно:

16-битный (2 байта).

Показывает текущее состояние привода/контроллера:

готов ли он принимать команды,

активен ли он,

есть ли ошибки,

находится ли в режиме включения/выключения.

Он состоит из набора битов, например:

Бит	Значение (пример)
Bit 0	Ready to switch on
Bit 1	Switched on
Bit 2	Operation enabled
Bit 3	Fault
Bit 4	Voltage enabled
Bit 5	Quick stop
Bit 6	Switch on disabled
Bit 7+	... (vendor-specific or standard)

Каждый бит сигнализирует о каком-то конкретном внутреннем состоянии.
 *
 *
 *
 *
 */
