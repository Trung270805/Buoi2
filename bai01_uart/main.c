#include "stm32f1xx_hal.h"
#include <stdint.h>

UART_HandleTypeDef huart1;

#define RX_BUFFER_SIZE 100

uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;
uint8_t rx_data;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void Error_Handler(void);

/* SysTick Handler */
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

int main(void)
{
    HAL_Init();

    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    while (1)
    {
        /* Nhận từng ký tự từ PC */
        if (HAL_UART_Receive(&huart1, &rx_data, 1, HAL_MAX_DELAY) == HAL_OK)
        {
            /* Nếu gặp ký tự kết thúc '!' */
            if (rx_data == '!')
            {
                /* Gửi mã lớp và mã nhóm */
                uint8_t prefix[] = "<VM02><01>:";

                HAL_UART_Transmit(
                    &huart1,
                    prefix,
                    sizeof(prefix) - 1,
                    HAL_MAX_DELAY
                );

                /* Gửi bản tin đã nhận */
                if (rx_index > 0)
                {
                    HAL_UART_Transmit(
                        &huart1,
                        rx_buffer,
                        rx_index,
                        HAL_MAX_DELAY
                    );
                }

                /* Gửi ký tự xuống dòng theo yêu cầu */
                uint8_t newline[] = "\n\r";

                HAL_UART_Transmit(
                    &huart1,
                    newline,
                    sizeof(newline) - 1,
                    HAL_MAX_DELAY
                );

                /* Xóa bộ đệm */
                rx_index = 0;
            }
            else
            {
                /* Lưu ký tự nhận được vào bộ đệm */
                if (rx_index < RX_BUFFER_SIZE - 1)
                {
                    rx_buffer[rx_index] = rx_data;
                    rx_index++;
                }
            }
        }
    }
}

/* Cấu hình Clock */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

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

/* Cấu hình USART1 */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;

    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/* Cấu hình GPIO */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/* Cấu hình chân USART1 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA9 - USART1 TX */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* PA10 - USART1 RX */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/* Xử lý lỗi */
static void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
