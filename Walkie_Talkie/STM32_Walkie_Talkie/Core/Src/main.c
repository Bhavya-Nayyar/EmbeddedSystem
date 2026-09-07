#include "main.h"
#include <stdint.h>
#include <string.h>

#define NRF_PAYLOAD_SIZE 32
#define I2S_FRAME_SAMPLES 32

#define CE_LOW() HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET)
#define CE_HIGH() HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_SET)
#define CSN_LOW()                                                              \
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET)
#define CSN_HIGH()                                                             \
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET)

#define NRF_CONFIG 0x00
#define NRF_EN_AA 0x01
#define NRF_EN_RXADDR 0x02
#define NRF_SETUP_AW 0x03
#define NRF_SETUP_RETR 0x04
#define NRF_RF_CH 0x05
#define NRF_RF_SETUP 0x06
#define NRF_STATUS 0x07

#define NRF_RX_ADDR_P0 0x0A
#define NRF_TX_ADDR 0x10
#define NRF_RX_PW_P0 0x11

#define NRF_W_REGISTER 0x20
#define NRF_R_REGISTER 0x00
#define NRF_R_RX_PAYLOAD 0x61
#define NRF_W_TX_PAYLOAD 0xA0
#define NRF_FLUSH_TX 0xE1
#define NRF_FLUSH_RX 0xE2

#define NRF_STATUS_RX_DR (1U << 6)
#define NRF_STATUS_TX_DS (1U << 5)
#define NRF_STATUS_MAX_RT (1U << 4)

static const uint8_t NRF_ADDRESS[5] = {0xD2, 0xD2, 0xD2, 0xD2, 0xD2};

/* Explicit Peripheral Handles Required by Linker & stm32f4xx_it.c */
I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi3_rx;

/* Audio Buffers */
static uint16_t i2s_rx_dma[I2S_FRAME_SAMPLES * 2];
static uint16_t i2s_tx_dma[I2S_FRAME_SAMPLES * 2];

static uint8_t nrf_tx_payload[NRF_PAYLOAD_SIZE];
static uint8_t nrf_rx_payload[NRF_PAYLOAD_SIZE];

volatile uint8_t mic_data_ready = 0;
volatile uint8_t speaker_dma_busy = 0;

/* Function Prototypes */
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_SPI1_Init(void);
void MX_I2S2_Init(void);
void MX_I2S3_Init(void);

static void DWT_Delay_us(uint32_t us) {
  uint32_t start = DWT->CYCCNT;
  us *= (SystemCoreClock / 1000000);
  while ((DWT->CYCCNT - start) < us)
    ;
}

static void NRF24_WriteReg(uint8_t reg, uint8_t value) {
  uint8_t buf[2] = {NRF_W_REGISTER | (reg & 0x1F), value};
  CSN_LOW();
  HAL_SPI_Transmit(&hspi1, buf, 2, 10);
  CSN_HIGH();
}

static uint8_t NRF24_ReadReg(uint8_t reg) {
  uint8_t cmd = NRF_R_REGISTER | (reg & 0x1F);
  uint8_t val = 0;
  CSN_LOW();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
  HAL_SPI_Receive(&hspi1, &val, 1, 10);
  CSN_HIGH();
  return val;
}

static void NRF24_WriteRegMulti(uint8_t reg, const uint8_t *data,
                                uint8_t length) {
  uint8_t cmd = NRF_W_REGISTER | (reg & 0x1F);
  CSN_LOW();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)data, length, 10);
  CSN_HIGH();
}

static void NRF24_Command(uint8_t command) {
  CSN_LOW();
  HAL_SPI_Transmit(&hspi1, &command, 1, 10);
  CSN_HIGH();
}

static void NRF24_RxMode(void) {
  CE_LOW();
  NRF24_WriteReg(NRF_CONFIG, 0x0B);
  DWT_Delay_us(130);
  CE_HIGH();
}

static void NRF24_TxMode(void) {
  CE_LOW();
  NRF24_WriteReg(NRF_CONFIG, 0x0A);
  DWT_Delay_us(130);
}

static void NRF24_Init(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  CE_LOW();
  CSN_HIGH();
  HAL_Delay(10);

  NRF24_WriteReg(NRF_CONFIG, 0x08);
  NRF24_WriteReg(NRF_EN_AA, 0x00);
  NRF24_WriteReg(NRF_EN_RXADDR, 0x01);
  NRF24_WriteReg(NRF_SETUP_AW, 0x03);
  NRF24_WriteReg(NRF_SETUP_RETR, 0x00);
  NRF24_WriteReg(NRF_RF_CH, 76);
  NRF24_WriteReg(NRF_RF_SETUP, 0x0E);

  NRF24_WriteRegMulti(NRF_RX_ADDR_P0, NRF_ADDRESS, 5);
  NRF24_WriteRegMulti(NRF_TX_ADDR, NRF_ADDRESS, 5);
  NRF24_WriteReg(NRF_RX_PW_P0, NRF_PAYLOAD_SIZE);

  NRF24_WriteReg(NRF_STATUS,
                 NRF_STATUS_RX_DR | NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT);
  NRF24_Command(NRF_FLUSH_TX);
  NRF24_Command(NRF_FLUSH_RX);

  NRF24_RxMode();
}

static uint8_t NRF24_Transmit(uint8_t *payload) {
  NRF24_TxMode();

  uint8_t cmd = NRF_W_TX_PAYLOAD;
  NRF24_WriteReg(NRF_STATUS, NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT);
  NRF24_Command(NRF_FLUSH_TX);

  CSN_LOW();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
  HAL_SPI_Transmit(&hspi1, payload, NRF_PAYLOAD_SIZE, 10);
  CSN_HIGH();

  CE_HIGH();
  DWT_Delay_us(15);
  CE_LOW();

  uint32_t timeout = 1000;
  while (timeout--) {
    uint8_t status = NRF24_ReadReg(NRF_STATUS);
    if (status & NRF_STATUS_TX_DS) {
      NRF24_WriteReg(NRF_STATUS, NRF_STATUS_TX_DS);
      NRF24_RxMode();
      return 1;
    }
    if (status & NRF_STATUS_MAX_RT) {
      NRF24_WriteReg(NRF_STATUS, NRF_STATUS_MAX_RT);
      NRF24_RxMode();
      return 0;
    }
    DWT_Delay_us(10);
  }
  NRF24_RxMode();
  return 0;
}

static uint8_t NRF24_CheckRx(void) {
  return (NRF24_ReadReg(NRF_STATUS) & NRF_STATUS_RX_DR) != 0;
}

static void NRF24_ReadPayload(uint8_t *payload) {
  uint8_t cmd = NRF_R_RX_PAYLOAD;
  CSN_LOW();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
  HAL_SPI_Receive(&hspi1, payload, NRF_PAYLOAD_SIZE, 10);
  CSN_HIGH();
  NRF24_WriteReg(NRF_STATUS, NRF_STATUS_RX_DR);
}

static int32_t I2S_ReadSample(uint16_t *buffer, uint32_t index) {
  uint32_t high = buffer[index * 2];
  uint32_t low = buffer[(index * 2) + 1];
  return (int32_t)((high << 16) | low);
}

static void I2S_WriteSample(uint16_t *buffer, uint32_t index, int32_t sample) {
  buffer[index * 2] = (uint16_t)(sample >> 16);
  buffer[(index * 2) + 1] = (uint16_t)(sample & 0xFFFF);
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == SPI3) {
    mic_data_ready = 1;
  }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == SPI2) {
    speaker_dma_busy = 0;
  }
}

int main(void) {
  HAL_Init();
  SystemClock_Config();
  PeriphCommonClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();

  NRF24_Init();

  if (HAL_I2S_Receive_DMA(&hi2s3, i2s_rx_dma, I2S_FRAME_SAMPLES * 2) !=
      HAL_OK) {
    Error_Handler();
  }

  while (1) {
    if (NRF24_CheckRx()) {
      NRF24_ReadPayload(nrf_rx_payload);
      for (int i = 0; i < I2S_FRAME_SAMPLES; i++) {
        int8_t sample8 = (int8_t)((int16_t)nrf_rx_payload[i] - 128);
        int32_t sample32 = ((int32_t)sample8) << 24;
        I2S_WriteSample(i2s_tx_dma, i, sample32);
      }

      if (!speaker_dma_busy) {
        speaker_dma_busy = 1;
        HAL_I2S_Transmit_DMA(&hi2s2, i2s_tx_dma, I2S_FRAME_SAMPLES * 2);
      }
    }

    if (mic_data_ready) {
      mic_data_ready = 0;
      for (int i = 0; i < I2S_FRAME_SAMPLES; i++) {
        int32_t sample = I2S_ReadSample(i2s_rx_dma, i);

        // Shift right by 23 to scale the signed 24-bit MSB-aligned sample to
        // 8-bit
        int8_t sample8 = (int8_t)(sample >> 23);

        // Convert signed int8 (-128 to 127) to unsigned uint8 (0 to 255) for
        // NRF24
        nrf_tx_payload[i] = (uint8_t)(sample8 + 128);
      }
      NRF24_Transmit(nrf_tx_payload);
    }
  }
}

/* System Clock & CubeMX Hardware Inits */

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
    Error_Handler();
  }
}

void PeriphCommonClock_Config(void) {
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection =
      RCC_PERIPHCLK_I2S_APB1 | RCC_PERIPHCLK_I2S_APB2;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 8;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    Error_Handler();
  }
}

void MX_SPI1_Init(void) {
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    Error_Handler();
  }
}

void MX_I2S2_Init(void) {
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK) {
    Error_Handler();
  }
}

void MX_I2S3_Init(void) {
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK) {
    Error_Handler();
  }
}

void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA1_Stream4_IRQn - SPI2_TX */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

  /* DMA1_Stream0_IRQn - SPI3_RX */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = NRF_CE_Pin | NRF_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(NRF_CE_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}