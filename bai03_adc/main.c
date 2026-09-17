#include "stm32f1xx_hal.h"
#include <stdint.h>

ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart1;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void Error_Handler(void);

/* Gửi chuỗi qua UART */
void UART_SendString(char *str)
{
    while (*str)
    {
        HAL_UART_Transmit(
            &huart1,
            (uint8_t *)str,
            1,
            HAL_MAX_DELAY
        );
        str++;
    }
}

/* Gửi số nguyên qua UART */
void UART_SendNumber(uint32_t number)
{
    char buf[10];
    int i = 0;

    if (number == 0)
    {
        char c = '0';

        HAL_UART_Transmit(
            &huart1,
            (uint8_t *)&c,
            1,
            HAL_MAX_DELAY
        );

        return;
    }

    while (number > 0)
    {
        buf[i++] = '0' + (number % 10);
        number /= 10;
    }

    while (i > 0)
    {
        char c = buf[--i];

        HAL_UART_Transmit(
            &huart1,
            (uint8_t *)&c,
            1,
            HAL_MAX_DELAY
        );
    }
}

/* SysTick */
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

int main(void)
{
    uint32_t adc_value;
    uint32_t voltage_mv;

    HAL_Init();

    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART1_UART_Init();

    /* Hiệu chuẩn ADC */
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    while (1)
    {
        /* Bắt đầu chuyển đổi ADC */
        HAL_ADC_Start(&hadc1);

        /* Chờ ADC hoàn thành */
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            /* Đọc giá trị ADC */
            adc_value = HAL_ADC_GetValue(&hadc1);

            /* Tính điện áp: V = ADC × 3.3 / 4095 */
            voltage_mv = (adc_value * 3300U) / 4095U;

            /* Gửi kết quả qua UART */
            UART_SendString("ADC = ");
            UART_SendNumber(adc_value);

            UART_SendString("    Voltage = ");
            UART_SendNumber(voltage_mv);

            UART_SendString(" mV\r\n");
        }

        HAL_ADC_Stop(&hadc1);

        /* Đọc mỗi 1 giây */
        HAL_Delay(1000);
    }
}

/* ================= CLOCK ================= */

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState = RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ================= ADC1 ================= */

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;

    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;

    hadc1.Init.ContinuousConvMode = DISABLE;

    hadc1.Init.DiscontinuousConvMode = DISABLE;

    hadc1.Init.ExternalTrigConv =
        ADC_SOFTWARE_START;

    hadc1.Init.DataAlign =
        ADC_DATAALIGN_RIGHT;

    hadc1.Init.NbrOfConversion = 1;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* PA0 = ADC1_IN0 */
    sConfig.Channel = ADC_CHANNEL_0;

    sConfig.Rank = ADC_REGULAR_RANK_1;

    sConfig.SamplingTime =
        ADC_SAMPLETIME_239CYCLES_5;

    if (HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ================= UART1 ================= */

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;

    huart1.Init.BaudRate = 115200;

    huart1.Init.WordLength =
        UART_WORDLENGTH_8B;

    huart1.Init.StopBits =
        UART_STOPBITS_1;

    huart1.Init.Parity =
        UART_PARITY_NONE;

    huart1.Init.Mode =
        UART_MODE_TX_RX;

    huart1.Init.HwFlowCtl =
        UART_HWCONTROL_NONE;

    huart1.Init.OverSampling =
        UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ================= GPIO ================= */

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/* ================= ADC GPIO ================= */

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hadc->Instance == ADC1)
    {
        __HAL_RCC_ADC1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA0 = Analog input */
        GPIO_InitStruct.Pin = GPIO_PIN_0;

        GPIO_InitStruct.Mode =
            GPIO_MODE_ANALOG;

        HAL_GPIO_Init(
            GPIOA,
            &GPIO_InitStruct
        );
    }
}

/* ================= UART GPIO ================= */

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA9 = TX */
        GPIO_InitStruct.Pin = GPIO_PIN_9;

        GPIO_InitStruct.Mode =
            GPIO_MODE_AF_PP;

        GPIO_InitStruct.Speed =
            GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(
            GPIOA,
            &GPIO_InitStruct
        );

        /* PA10 = RX */
        GPIO_InitStruct.Pin = GPIO_PIN_10;

        GPIO_InitStruct.Mode =
            GPIO_MODE_INPUT;

        GPIO_InitStruct.Pull =
            GPIO_NOPULL;

        HAL_GPIO_Init(
            GPIOA,
            &GPIO_InitStruct
        );
    }
}

/* ================= ERROR ================= */

static void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
