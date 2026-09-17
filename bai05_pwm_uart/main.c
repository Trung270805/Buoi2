#include "stm32f1xx_hal.h"
#include <stdint.h>

TIM_HandleTypeDef htim2;

/* ================= UART ================= */

volatile uint8_t rx_byte;
volatile uint8_t rx_index = 0;
volatile uint8_t command_ready = 0;

char rx_buffer[32];

/* ================= LED ================= */

uint8_t led_on = 0;
uint32_t pwm_percent = 50;

/* ================= VECTOR TABLE ================= */

uint32_t vector_table[64]
    __attribute__((aligned(256)));

/* ================= PROTOTYPE ================= */

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_Init(void);

static void UART_SendChar(char c);
static void UART_SendString(const char *str);
static void UART_SendNumber(uint32_t number);

static void Set_PWM(uint32_t percent);
static void ProcessCommand(void);

static uint8_t IsCommand(
    const char *cmd,
    const char *target
);

static uint8_t ParsePWM(
    const char *cmd,
    uint32_t *percent
);

static void RelocateVectorTable(void);
static void Error_Handler(void);

void HAL_TIM_MspPostInit(
    TIM_HandleTypeDef *htim
);

/* ================= SYSTICK ================= */

void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

/* ================= USART1 INTERRUPT ================= */

void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE)
    {
        rx_byte = (uint8_t)USART1->DR;

        if (rx_byte == '!')
        {
            rx_buffer[rx_index] = '\0';
            command_ready = 1;
        }
        else if (rx_byte != '\r' &&
                 rx_byte != '\n')
        {
            if (rx_index < sizeof(rx_buffer) - 1)
            {
                rx_buffer[rx_index++] = rx_byte;
            }
        }
    }
}

/* ================= MAIN ================= */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_USART1_Init();

    RelocateVectorTable();

    /* LED ban đầu OFF */
    Set_PWM(0);

    if (HAL_TIM_PWM_Start(
            &htim2,
            TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    UART_SendString("READY\r\n");

    while (1)
    {
        if (command_ready)
        {
            ProcessCommand();

            rx_index = 0;
            command_ready = 0;

            rx_buffer[0] = '\0';
        }
    }
}

/* ================= VECTOR ================= */

static void RelocateVectorTable(void)
{
    uint32_t i;

    for (i = 0; i < 64; i++)
    {
        vector_table[i] =
            ((uint32_t *)0x08000000)[i];
    }

    /*
     * USART1 IRQ = 37
     * vector index = 16 + 37 = 53
     */
    vector_table[53] =
        (uint32_t)USART1_IRQHandler;

    SCB->VTOR =
        (uint32_t)vector_table;

    __DSB();
    __ISB();
}

/* ================= PROCESS COMMAND ================= */

static void ProcessCommand(void)
{
    uint32_t new_pwm;

    /* ON! */

    if (IsCommand(
            rx_buffer,
            "ON"))
    {
        led_on = 1;

        Set_PWM(pwm_percent);

        UART_SendString(
            "LED ON\r\n"
        );

        return;
    }

    /* OFF! */

    if (IsCommand(
            rx_buffer,
            "OFF"))
    {
        led_on = 0;

        Set_PWM(0);

        UART_SendString(
            "LED OFF\r\n"
        );

        return;
    }

    /* PWM:xx%! */

    if (ParsePWM(
            rx_buffer,
            &new_pwm))
    {
        pwm_percent = new_pwm;

        /*
         * ON:
         * thay đổi độ sáng ngay.
         *
         * OFF:
         * chỉ lưu PWM.
         */
        if (led_on)
        {
            Set_PWM(pwm_percent);
        }

        UART_SendString("PWM = ");

        UART_SendNumber(
            pwm_percent
        );

        UART_SendString(
            "%\r\n"
        );

        return;
    }

    /* Status! */

    if (IsCommand(
            rx_buffer,
            "Status"))
    {
        UART_SendString(
            "Status: LED="
        );

        if (led_on)
        {
            UART_SendString("ON");
        }
        else
        {
            UART_SendString("OFF");
        }

        UART_SendString(
            ", PWM="
        );

        UART_SendNumber(
            pwm_percent
        );

        UART_SendString(
            "%\r\n"
        );

        return;
    }

    /* ERROR */

    UART_SendString(
        "ERROR\r\n"
    );
}

/* ================= COMMAND COMPARE ================= */

static uint8_t IsCommand(
    const char *cmd,
    const char *target)
{
    uint8_t i = 0;

    while (target[i] != '\0')
    {
        if (cmd[i] != target[i])
        {
            return 0;
        }

        i++;
    }

    if (cmd[i] != '\0')
    {
        return 0;
    }

    return 1;
}

/* ================= PWM PARSER ================= */

static uint8_t ParsePWM(
    const char *cmd,
    uint32_t *percent)
{
    uint8_t i = 0;
    uint32_t value = 0;

    /* Phải bắt đầu bằng PWM: */

    if (cmd[0] != 'P')
        return 0;

    if (cmd[1] != 'W')
        return 0;

    if (cmd[2] != 'M')
        return 0;

    if (cmd[3] != ':')
        return 0;

    i = 4;

    /* Phải có ít nhất 1 chữ số */

    if (cmd[i] < '0' ||
        cmd[i] > '9')
    {
        return 0;
    }

    /* Đọc số phần trăm */

    while (cmd[i] >= '0' &&
           cmd[i] <= '9')
    {
        value =
            value * 10U +
            (cmd[i] - '0');

        if (value > 100)
        {
            return 0;
        }

        i++;
    }

    /* Phải có dấu % */

    if (cmd[i] != '%')
    {
        return 0;
    }

    i++;

    /* Sau % phải kết thúc lệnh */

    if (cmd[i] != '\0')
    {
        return 0;
    }

    *percent = value;

    return 1;
}

/* ================= PWM ================= */

static void Set_PWM(uint32_t percent)
{
    uint32_t compare;

    if (percent > 100)
    {
        percent = 100;
    }

    compare =
        ((htim2.Init.Period + 1U)
         * percent) / 100U;

    __HAL_TIM_SET_COMPARE(
        &htim2,
        TIM_CHANNEL_1,
        compare
    );
}

/* ================= UART SEND CHAR ================= */

static void UART_SendChar(char c)
{
    while (!(USART1->SR &
             USART_SR_TXE))
    {
    }

    USART1->DR =
        (uint8_t)c;
}

/* ================= UART SEND STRING ================= */

static void UART_SendString(
    const char *str)
{
    while (*str)
    {
        UART_SendChar(*str);

        str++;
    }
}

/* ================= UART SEND NUMBER ================= */

static void UART_SendNumber(
    uint32_t number)
{
    char buffer[10];
    uint8_t i = 0;

    if (number == 0)
    {
        UART_SendChar('0');
        return;
    }

    while (number > 0)
    {
        buffer[i++] =
            '0' + (number % 10);

        number /= 10;
    }

    while (i > 0)
    {
        UART_SendChar(
            buffer[--i]
        );
    }
}

/* ================= CLOCK ================= */

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(
            &RCC_OscInitStruct) != HAL_OK)
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

/* ================= TIM2 ================= */

static void MX_TIM2_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;

    /* 8 MHz / 8 = 1 MHz */

    htim2.Init.Prescaler = 7;

    /* 1 MHz / 1000 = 1 kHz */

    htim2.Init.Period = 999;

    htim2.Init.CounterMode =
        TIM_COUNTERMODE_UP;

    htim2.Init.ClockDivision =
        TIM_CLOCKDIVISION_DIV1;

    htim2.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_PWM_Init(
            &htim2) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode =
        TIM_OCMODE_PWM1;

    sConfigOC.Pulse = 0;

    sConfigOC.OCPolarity =
        TIM_OCPOLARITY_HIGH;

    sConfigOC.OCFastMode =
        TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(
            &htim2,
            &sConfigOC,
            TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_MspPostInit(
        &htim2
    );
}

/* ================= USART1 ================= */

static void MX_USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    /* PA9 TX */

    GPIO_InitStruct.Pin =
        GPIO_PIN_9;

    GPIO_InitStruct.Mode =
        GPIO_MODE_AF_PP;

    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );

    /* PA10 RX */

    GPIO_InitStruct.Pin =
        GPIO_PIN_10;

    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;

    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );

    /*
     * 8 MHz / 115200
     * BRR = 69
     */

    USART1->BRR = 69;

    USART1->CR1 =
        USART_CR1_TE |
        USART_CR1_RE |
        USART_CR1_RXNEIE;

    USART1->CR2 = 0;
    USART1->CR3 = 0;

    HAL_NVIC_SetPriority(
        USART1_IRQn,
        0,
        0
    );

    HAL_NVIC_EnableIRQ(
        USART1_IRQn
    );

    USART1->CR1 |=
        USART_CR1_UE;
}

/* ================= GPIO ================= */

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/* ================= TIM2 MSP ================= */

void HAL_TIM_PWM_MspInit(
    TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

/* ================= PA0 PWM ================= */

void HAL_TIM_MspPostInit(
    TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM2)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin =
            GPIO_PIN_0;

        GPIO_InitStruct.Mode =
            GPIO_MODE_AF_PP;

        GPIO_InitStruct.Speed =
            GPIO_SPEED_FREQ_HIGH;

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
