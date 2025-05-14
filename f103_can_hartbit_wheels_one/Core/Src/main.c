/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "can.h"
#include "CO_NMT.h"
#include "eds_data.h"
#include <stdio.h>
#include "CO_SDO.h"
#include "COB_Dispatcher.h"
#define DEBUG_UART_LOG
#include "CO_PDO.h"
#include "CO_SYNC.h"
#include <string.h>
#include <stdio.h>
#define WHEEL_NODE_1 0x03
#define WHEEL_NODE_2 0x02
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
CAN_HandleTypeDef hcan;
extern volatile uint8_t canopen_confirmation_received;
extern void CO_PDO_DebugPrint(uint8_t node_id);

int32_t target_speed = 0;//500000;

volatile uint8_t controlword_stage = 0;
//ControlMode active_mode = MODE_POSITION;
ControlMode active_mode = MODE_SPEED;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  LogMessage("=== System Init ===");

  // 1. Инициализация CAN
  CAN_Init();
  HAL_Delay(500);

  // Сброс
  CO_NMT_Send(&hcan, NMT_RESET_NODE, WHEEL_NODE_1);
  CO_NMT_Send(&hcan, NMT_RESET_NODE, WHEEL_NODE_2);
  HAL_Delay(300);

  // Старт
  CO_NMT_Send(&hcan, NMT_START, WHEEL_NODE_1);
  CO_NMT_Send(&hcan, NMT_START, WHEEL_NODE_2);
  HAL_Delay(300);

  // Активация
  CAN_SendActivationCommand(&hcan, WHEEL_NODE_1);
  CAN_SendActivationCommand(&hcan, WHEEL_NODE_2);
  HAL_Delay(100);

  // Снятие с тормоза
  CO_NMT_ReleaseBrake(&hcan, WHEEL_NODE_1);
  CO_NMT_ReleaseBrake(&hcan, WHEEL_NODE_2);
  HAL_Delay(100);


  // Инициализация режима скорости через SDO
  CO_SDO_Init_SpeedMode(&hcan, WHEEL_NODE_1);
  CO_SDO_Init_SpeedMode(&hcan, WHEEL_NODE_2);
  HAL_Delay(100);

  // ⚠️ ПЕРЕХОД в Operational только теперь
  CO_NMT_Send(&hcan, NMT_START, WHEEL_NODE_1);
  CO_NMT_Send(&hcan, NMT_START, WHEEL_NODE_2);
  HAL_Delay(100);

  // Настройка RPDO
  CO_PDO_ConfigForSpeedMode(&hcan, WHEEL_NODE_1);
  CO_PDO_ConfigForSpeedMode(&hcan, WHEEL_NODE_2);

  CO_PDO_StartupSequence_SpeedMode(&hcan, WHEEL_NODE_1, 2000);
  CO_PDO_StartupSequence_SpeedMode(&hcan, WHEEL_NODE_2, 2000);





  // Первый запуск
  CO_SYNC_ResetStep();
  CO_SYNC_SetEnabled(1);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_Delay(8000); // подождать пока мотор закончит ( это 30 сек = 3000 шагов, можно подкорректировать)
  HAL_TIM_Base_Stop_IT(&htim2);
  CO_SYNC_SetEnabled(0);

  HAL_Delay(3000);

  // Второй запуск
  CO_SYNC_ResetStep();
  CO_SYNC_SetEnabled(1);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_Delay(5000);
  HAL_TIM_Base_Stop_IT(&htim2);
  CO_SYNC_SetEnabled(0);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {



	  COB_ProcessStateMachine();  // Всё управление через FSM
	  HAL_Delay(10);



	  /*
	      if (CO_PDO_IsReceived(WHEEL_NODE_ID)) {
	          LogMessage("✅ PDO received");
	          CO_PDO_DebugPrint(WHEEL_NODE_ID);
	          HAL_Delay(500);
	      }
	      // Отладочное чтение параметров
	      uint32_t val = 0;

	      if (CO_SDO_Read(&hcan, WHEEL_NODE_ID, 0x6061, 0x00, &val) == HAL_OK) {
	          char dbg[64];
	          sprintf(dbg, "Mode Display (0x6061) = 0x%02lX", val);
	          LogMessage(dbg);
	      }

	      if (CO_SDO_Read(&hcan, WHEEL_NODE_ID, 0x6041, 0x00, &val) == HAL_OK) {
	          char dbg[64];
	          sprintf(dbg, "Statusword (0x6041) = 0x%04lX", val);
	          LogMessage(dbg);
	      }

	      if (CO_SDO_Read(&hcan, WHEEL_NODE_ID, 0x606C, 0x00, &val) == HAL_OK) {
	          char dbg[64];
	          sprintf(dbg, "Actual Speed (0x606C) = %ld", (int32_t)val);
	          LogMessage(dbg);
	      }
	   */

  }
  /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 18;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */



  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

 // скорость в ticks/sec


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && CO_SYNC_IsEnabled()) {
        int32_t target_speed = (sync_step_counter < 80) ? 9000 : 0;

        CO_SYNC_SendRPDOAndSYNC(&hcan, WHEEL_NODE_1, 0x000F, target_speed);
        CO_SYNC_SendRPDOAndSYNC(&hcan, WHEEL_NODE_2, 0x000F, target_speed);

        if (sync_step_counter == 80) {
            CO_SYNC_SetEnabled(0);
            HAL_TIM_Base_Stop_IT(htim);
            LogMessage("STEP: Stop motion");
        }

        sync_step_counter++;
    }
}


/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && CO_SYNC_IsEnabled()) {
        static uint32_t last_change = 0;
        uint32_t now = HAL_GetTick();

        if (now - last_change >= 500) { // интервал
            last_change = now;

            LogMessage("STEP: Sending target speed via RPDO...");
            CO_SYNC_SendRPDOAndSYNC(&hcan, WHEEL_NODE_ID, 0x000F, target_speed);
        }
    }
}
*/

/*
uint8_t sync_counter = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && CO_SYNC_IsEnabled()) {
        int32_t pos = (sync_counter++ % 2 == 0) ? 32768 * 10 : -32768 * 10;

        // Отправка RPDO + SYNC
        CO_SYNC_SendRPDOAndSYNC(&hcan, 0x03, 0x000F, pos);
    }
}
*/

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
