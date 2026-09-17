#include "stm32f1xx_hal.h"

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_TIM2_Init(void);

void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_TIM2_Init();

    /* Start PWM 4 channels */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

    while (1)
    {
    }
}

/* =========================
   TIM2 PWM CONFIGURATION
   ========================= */
static void MX_TIM2_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;

    /*
     * HSI = 8 MHz
     * Prescaler = 7
     * Timer frequency = 8 MHz / (7 + 1)
     *                 = 1 MHz
     *
     * Period = 999
     * PWM frequency = 1 MHz / 1000
     *               = 1 kHz
     */
    htim2.Init.Prescaler = 7;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        while (1)
        {
        }
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    /* PA0 - TIM2_CH1 - Duty 10% */
    sConfigOC.Pulse = 100;

    if (HAL_TIM_PWM_ConfigChannel(
            &htim2,
            &sConfigOC,
            TIM_CHANNEL_1) != HAL_OK)
    {
        while (1)
        {
        }
    }

    /* PA1 - TIM2_CH2 - Duty 30% */
    sConfigOC.Pulse = 300;

    if (HAL_TIM_PWM_ConfigChannel(
            &htim2,
            &sConfigOC,
            TIM_CHANNEL_2) != HAL_OK)
    {
        while (1)
        {
        }
    }

    /* PA2 - TIM2_CH3 - Duty 50% */
    sConfigOC.Pulse = 500;

    if (HAL_TIM_PWM_ConfigChannel(
            &htim2,
            &sConfigOC,
            TIM_CHANNEL_3) != HAL_OK)
    {
        while (1)
        {
        }
    }

    /* PA3 - TIM2_CH4 - Duty 70% */
    sConfigOC.Pulse = 700;

    if (HAL_TIM_PWM_ConfigChannel(
            &htim2,
            &sConfigOC,
            TIM_CHANNEL_4) != HAL_OK)
    {
        while (1)
        {
        }
    }
}

/* =========================
   GPIO + TIM2
   ========================= */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* Enable clocks */
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /*
         * TIM2:
         * CH1 -> PA0
         * CH2 -> PA1
         * CH3 -> PA2
         * CH4 -> PA3
         */
        GPIO_InitStruct.Pin =
            GPIO_PIN_0 |
            GPIO_PIN_1 |
            GPIO_PIN_2 |
            GPIO_PIN_3;

        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/* =========================
   SYSTEM CLOCK
   ========================= */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Use HSI 8 MHz */
    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState = RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1)
        {
        }
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
        while (1)
        {
        }
    }
}
