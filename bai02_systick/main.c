#include "stm32f1xx_hal.h"

volatile uint32_t counter_01hz = 0;
volatile uint32_t counter_1hz  = 0;
volatile uint32_t counter_10hz = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    while (1)
    {
        /* LED được điều khiển trong ngắt SysTick */
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

void HAL_SYSTICK_Callback(void)
{
    /* 0.1 Hz: chu kỳ 10 giây -> đảo trạng thái mỗi 5 giây */
    counter_01hz++;
    if (counter_01hz >= 5000)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
        counter_01hz = 0;
    }

    /* 1 Hz: chu kỳ 1 giây -> đảo trạng thái mỗi 500 ms */
    counter_1hz++;
    if (counter_1hz >= 500)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
        counter_1hz = 0;
    }

    /* 10 Hz: chu kỳ 100 ms -> đảo trạng thái mỗi 50 ms */
    counter_10hz++;
    if (counter_10hz >= 50)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
        counter_10hz = 0;
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1) {}
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        while (1) {}
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA,
                      GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2,
                      GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
