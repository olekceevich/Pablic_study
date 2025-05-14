#include "CO_SYNC.h"
#include "stm32f1xx_hal.h"
#include "can.h"
#include <string.h>
#include <stdio.h>

// Настройки
#define USE_SPEED_MODE 1   // 1 = Speed Mode, 0 = Position Mode
#define NODE_ID 0x01
#define LOG_DELAY_MS 500

static uint32_t last_rpdo_log = 0;
uint8_t sync_enabled = 0;
  // Счётчик шагов таймера

uint16_t sync_step_counter = 0;  // ✅ просто global, без static
void CO_SYNC_ResetStep(void) {
    sync_step_counter = 0;
}

extern void LogMessage(const char *);
void LogMessage_Once(const char *msg);

void CO_SYNC_SetEnabled(uint8_t enabled) {
    sync_enabled = enabled;
}

uint8_t CO_SYNC_IsEnabled(void) {
    return sync_enabled;
}

void CO_SYNC_Send(CAN_HandleTypeDef *hcan) {
    if (!sync_enabled) return;

    CAN_TxHeaderTypeDef header;
    uint8_t data[1] = {0};
    uint32_t mailbox;

    header.StdId = 0x080;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 0;

    if (HAL_CAN_AddTxMessage(hcan, &header, data, &mailbox) != HAL_OK) {
        LogMessage("SYNC send failed");
    } else {
        LogMessage("SYNC sent");
    }
}

void CO_SYNC_SendRPDOAndSYNC(CAN_HandleTypeDef *hcan, uint8_t node_id, uint16_t controlword, int32_t value) {
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
    uint8_t data[8] = {0};

    // Controlword (0x6040)
    data[0] = controlword & 0xFF;
    data[1] = (controlword >> 8) & 0xFF;

#if USE_SPEED_MODE
    // Target speed (0x60FF)
    data[2] = value & 0xFF;
    data[3] = (value >> 8) & 0xFF;
    data[4] = (value >> 16) & 0xFF;
    data[5] = (value >> 24) & 0xFF;
    LogMessage("RPDO (6040 + 60FF) sent");
#else
    // Target position (0x607A)
    data[2] = value & 0xFF;
    data[3] = (value >> 8) & 0xFF;
    data[4] = (value >> 16) & 0xFF;
    data[5] = (value >> 24) & 0xFF;
    LogMessage("RPDO (6040 + 607A) sent");
#endif

    data[6] = 0;
    data[7] = 0;

    txHeader.StdId = 0x200 + node_id;  // RPDO1
    txHeader.DLC = 8;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;

    if (HAL_CAN_AddTxMessage(hcan, &txHeader, data, &txMailbox) != HAL_OK) {
        LogMessage("RPDO send failed");
        return;
    }

    if (sync_enabled) {
        CO_SYNC_Send(hcan);
    } else {
        LogMessage("SYNC skipped (disabled)");
    }
}


/*
 *
 * Настроить TPDO1 для передачи данных (например, скорости двигателя из 0x3000) в асинхронном режиме, то есть без привязки к SYNC-сообщениям, а по внутреннему таймеру события (Event Timer).

🔧 Шаги конфигурации:

Шаг	Действие	Описание
1	Отключить TPDO	Записывается 0x80 в 0x1800:01, чтобы временно отключить PDO
2	Установить тип передачи = FF	0x1800:02 = 0xFF — асинхронная передача
3	Настроить минимальный интервал между отправками	0x1800:03 = 0x03E8 = 1000 (100 мс)
4	Установить Event Timer = 1000 мс	0x1800:05 = 0x03E8 = 1000 (1 секунда)
5	Очистить количество маппингов	0x1A00:00 = 0
6	Добавить маппинг объекта 0x3000 (2 байта)	0x1A00:01 = 0x100030
7	Установить количество маппингов = 1	0x1A00:00 = 1
8	Включить TPDO обратно	0x1800:01 = 0x0180 + node_id (без флага 0x80)
🧠 Важные замечания:
Асинхронный режим (transmission type = FF) — это значит, что устройство будет само по себе передавать TPDO каждые Event Timer мс (например, 1000 мс).

Inhibit Time (время запрета) задаёт минимальный интервал между двумя отправками, он не должен быть меньше 10 мс — иначе возможна перегрузка CAN-сети.

Mapping 0x3000 — означает, что мы передаём, к примеру, скорость двигателя.
 *
 *
 *
 * # 1. Отключаем TPDO1 (Communication parameter)
self._set_sdo_uint32(0x1800, 0x01, 0x80000180)  # 0x180 | NodeID + disable flag (0x80)

# 2. Тип передачи = 0xFF (асинхронный)
self._set_sdo_uint8(0x1800, 0x02, 0xFF)

# 3. Inhibit Time (в мс, например 100 мс)
self._set_sdo_uint16(0x1800, 0x03, 100)

# 4. Event Timer = 1000 мс
self._set_sdo_uint16(0x1800, 0x05, 1000)

# 5. Очистка маппинга
self._set_sdo_uint8(0x1A00, 0x00, 0)

# 6. Добавляем объект, например 0x3000:00 (скорость)
self._set_sdo_uint32(0x1A00, 0x01, 0x30000010)  # 0x3000:00, длина = 16 бит

# 7. Количество маппингов
self._set_sdo_uint8(0x1A00, 0x00, 1)

# 8. Включаем TPDO1 обратно
self._set_sdo_uint32(0x1800, 0x01, 0x00000180)  # 0x180 + NodeID без флага 0x80
 *
 *
 *
 *
 *
 *
 *
 *
 */














