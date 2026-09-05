/* USER CODE BEGIN Header */

#include "main.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include "string.h"

/* Private variables ---------------------------------------------------------*/

CAN_HandleTypeDef hcan1;

UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */

static void CAN_Filter_Config(void);
static void CAN_Send_Test_Message(uint8_t counter);
static void CAN_Process_Received_Message(void);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

int _write(int file, char *ptr, int len) {
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);

  return len;
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
  printf("CAN ERROR CALLBACK: 0x%08lX\r\n", hcan->ErrorCode);
}

static void CAN_Filter_Config(void) {
  CAN_FilterTypeDef filter = {0};

  filter.FilterBank = 0;

  filter.FilterMode = CAN_FILTERMODE_IDMASK;

  filter.FilterScale = CAN_FILTERSCALE_32BIT;

  filter.FilterIdHigh = 0x0000;
  filter.FilterIdLow = 0x0000;

  filter.FilterMaskIdHigh = 0x0000;
  filter.FilterMaskIdLow = 0x0000;

  filter.FilterFIFOAssignment = CAN_RX_FIFO0;

  filter.FilterActivation = ENABLE;

  filter.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
    Error_Handler();
  }
}

static void CAN_Send_Test_Message(uint8_t counter) {
  CAN_TxHeaderTypeDef tx_header = {0};

  uint8_t tx_data[8];

  uint32_t tx_mailbox;

  tx_header.StdId = 0x101;

  tx_header.ExtId = 0;

  tx_header.IDE = CAN_ID_STD;

  tx_header.RTR = CAN_RTR_DATA;

  tx_header.DLC = 8;

  tx_header.TransmitGlobalTime = DISABLE;

  tx_data[0] = counter;
  tx_data[1] = 0xAA;
  tx_data[2] = 0xBB;
  tx_data[3] = 0xCC;
  tx_data[4] = 0xDD;
  tx_data[5] = 0xEE;
  tx_data[6] = 0x11;
  tx_data[7] = 0x22;

  HAL_StatusTypeDef result =
      HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox);

  if (result != HAL_OK) {
    printf("STM32 CAN TX ERROR\r\n");

    printf("HAL State: %lu\r\n", hcan1.State);

    printf("CAN ErrorCode: 0x%08lX\r\n", hcan1.ErrorCode);

    printf("Free TX Mailboxes: %lu\r\n",
           HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));

    return;
  }

  printf("STM32 TX: ID=0x101 DATA=");

  for (int i = 0; i < 8; i++) {
    printf("%02X ", tx_data[i]);
  }

  printf("MAILBOX=%lu\r\n", tx_mailbox);
}

static void CAN_Process_Received_Message(void) {
  CAN_RxHeaderTypeDef rx_header;

  uint8_t rx_data[8];

  if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0) {
    return;
  }

  if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
    printf("STM32 CAN RX ERROR\r\n");

    printf("CAN ErrorCode: 0x%08lX\r\n", hcan1.ErrorCode);

    return;
  }

  printf("STM32 RX: ID=0x%03lX DLC=%lu DATA=", rx_header.StdId, rx_header.DLC);

  for (uint32_t i = 0; i < rx_header.DLC; i++) {
    printf("%02X ", rx_data[i]);
  }

  printf("\r\n");

  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

/* USER CODE END 0 */

int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  MX_CAN1_Init();

  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */

  CAN_Filter_Config();

  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
    printf("CAN START ERROR\r\n");

    Error_Handler();
  }

  printf("CAN started successfully\r\n");

  printf("Bitrate: 250000 bit/s\r\n");

  printf("RX ID: 0x100\r\n");

  printf("TX ID: 0x101\r\n");

  printf("CAN State: %lu\r\n", HAL_CAN_GetState(&hcan1));

  printf("CAN Error: 0x%08lX\r\n", HAL_CAN_GetError(&hcan1));

  printf("Free TX Mailboxes: %lu\r\n", HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));

  /* USER CODE END 2 */

  uint8_t counter = 0;

  uint32_t last_tx_time = HAL_GetTick();

  while (1) {
    CAN_Process_Received_Message();

    if ((HAL_GetTick() - last_tx_time) >= 1000) {
      last_tx_time = HAL_GetTick();

      CAN_Send_Test_Message(counter);

      counter++;
    }
  }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;

  RCC_OscInitStruct.HSIState = RCC_HSI_ON;

  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;

  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

  RCC_OscInitStruct.PLL.PLLM = 16;

  RCC_OscInitStruct.PLL.PLLN = 336;

  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;

  RCC_OscInitStruct.PLL.PLLQ = 2;

  RCC_OscInitStruct.PLL.PLLR = 2;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;

  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_CAN1_Init(void) {
  hcan1.Instance = CAN1;

  hcan1.Init.Prescaler = 12;

  hcan1.Init.Mode = CAN_MODE_NORMAL;

  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;

  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;

  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;

  hcan1.Init.TimeTriggeredMode = DISABLE;

  hcan1.Init.AutoBusOff = DISABLE;

  hcan1.Init.AutoWakeUp = DISABLE;

  hcan1.Init.AutoRetransmission = DISABLE;

  hcan1.Init.ReceiveFifoLocked = DISABLE;

  hcan1.Init.TransmitFifoPriority = DISABLE;

  if (HAL_CAN_Init(&hcan1) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void) {
  huart2.Instance = USART2;

  huart2.Init.BaudRate = 115200;

  huart2.Init.WordLength = UART_WORDLENGTH_8B;

  huart2.Init.StopBits = UART_STOPBITS_1;

  huart2.Init.Parity = UART_PARITY_NONE;

  huart2.Init.Mode = UART_MODE_TX_RX;

  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;

  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();

  __HAL_RCC_GPIOH_CLK_ENABLE();

  __HAL_RCC_GPIOA_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD2_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void) {
  __disable_irq();

  while (1) {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

    HAL_Delay(100);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line) {
  /*
   * User can add implementation here.
   */
}

#endif