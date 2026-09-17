.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Reset_Handler

.extern SystemInit
.extern __libc_init_array
.extern main

.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss
.extern _estack

/* ================= VECTOR TABLE ================= */

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object

g_pfnVectors:

    .word _estack
    .word Reset_Handler

    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler

    .word 0
    .word 0
    .word 0
    .word 0

    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* STM32F103 IRQ */

    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler

    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler

    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_IRQHandler
    .word DMA1_Channel3_IRQHandler
    .word DMA1_Channel4_IRQHandler
    .word DMA1_Channel5_IRQHandler
    .word DMA1_Channel6_IRQHandler
    .word DMA1_Channel7_IRQHandler

    .word ADC1_2_IRQHandler

    .word USB_HP_CAN1_TX_IRQHandler
    .word USB_LP_CAN1_RX0_IRQHandler
    .word CAN1_RX1_IRQHandler
    .word CAN1_SCE_IRQHandler

    .word EXTI9_5_IRQHandler

    .word TIM1_BRK_IRQHandler
    .word TIM1_UP_IRQHandler
    .word TIM1_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler

    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word TIM4_IRQHandler

    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word I2C2_EV_IRQHandler
    .word I2C2_ER_IRQHandler

    .word SPI1_IRQHandler
    .word SPI2_IRQHandler

    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word USART3_IRQHandler

    .word EXTI15_10_IRQHandler
    .word RTCAlarm_IRQHandler
    .word USBWakeUp_IRQHandler


/* ================= RESET HANDLER ================= */

.section .text.Reset_Handler
.type Reset_Handler, %function
.thumb_func

Reset_Handler:

    /* System clock */
    bl SystemInit

    /* Copy .data from FLASH to RAM */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

1:
    cmp r1, r2
    bcs 2f

    ldr r3, [r0]
    str r3, [r1]

    adds r0, r0, #4
    adds r1, r1, #4

    b 1b

    /* Clear .bss */
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

3:
    cmp r1, r2
    bcs 4f

    str r3, [r1]

    adds r1, r1, #4

    b 3b

    /* C library initialization */
4:
    bl __libc_init_array

    /* Main */
    bl main

5:
    b 5b


/* ================= DEFAULT HANDLER ================= */

.section .text.Default_Handler,"ax",%progbits
.type Default_Handler, %function
.thumb_func

Default_Handler:
    b Default_Handler


/* ================= WEAK HANDLERS ================= */

.weak NMI_Handler
.weak HardFault_Handler
.weak MemManage_Handler
.weak BusFault_Handler
.weak UsageFault_Handler
.weak SVC_Handler
.weak DebugMon_Handler
.weak PendSV_Handler
.weak SysTick_Handler

.weak WWDG_IRQHandler
.weak PVD_IRQHandler
.weak TAMPER_IRQHandler
.weak RTC_IRQHandler
.weak FLASH_IRQHandler
.weak RCC_IRQHandler

.weak EXTI0_IRQHandler
.weak EXTI1_IRQHandler
.weak EXTI2_IRQHandler
.weak EXTI3_IRQHandler
.weak EXTI4_IRQHandler

.weak DMA1_Channel1_IRQHandler
.weak DMA1_Channel2_IRQHandler
.weak DMA1_Channel3_IRQHandler
.weak DMA1_Channel4_IRQHandler
.weak DMA1_Channel5_IRQHandler
.weak DMA1_Channel6_IRQHandler
.weak DMA1_Channel7_IRQHandler

.weak ADC1_2_IRQHandler

.weak USB_HP_CAN1_TX_IRQHandler
.weak USB_LP_CAN1_RX0_IRQHandler
.weak CAN1_RX1_IRQHandler
.weak CAN1_SCE_IRQHandler

.weak EXTI9_5_IRQHandler

.weak TIM1_BRK_IRQHandler
.weak TIM1_UP_IRQHandler
.weak TIM1_TRG_COM_IRQHandler
.weak TIM1_CC_IRQHandler

.weak TIM2_IRQHandler
.weak TIM3_IRQHandler
.weak TIM4_IRQHandler

.weak I2C1_EV_IRQHandler
.weak I2C1_ER_IRQHandler
.weak I2C2_EV_IRQHandler
.weak I2C2_ER_IRQHandler

.weak SPI1_IRQHandler
.weak SPI2_IRQHandler

.weak USART1_IRQHandler
.weak USART2_IRQHandler
.weak USART3_IRQHandler

.weak EXTI15_10_IRQHandler
.weak RTCAlarm_IRQHandler
.weak USBWakeUp_IRQHandler


/* ================= WEAK ALIASES ================= */

.thumb_set NMI_Handler, Default_Handler
.thumb_set HardFault_Handler, Default_Handler
.thumb_set MemManage_Handler, Default_Handler
.thumb_set BusFault_Handler, Default_Handler
.thumb_set UsageFault_Handler, Default_Handler

.thumb_set SVC_Handler, Default_Handler
.thumb_set DebugMon_Handler, Default_Handler
.thumb_set PendSV_Handler, Default_Handler
.thumb_set SysTick_Handler, Default_Handler

.thumb_set WWDG_IRQHandler, Default_Handler
.thumb_set PVD_IRQHandler, Default_Handler
.thumb_set TAMPER_IRQHandler, Default_Handler
.thumb_set RTC_IRQHandler, Default_Handler
.thumb_set FLASH_IRQHandler, Default_Handler
.thumb_set RCC_IRQHandler, Default_Handler

.thumb_set EXTI0_IRQHandler, Default_Handler
.thumb_set EXTI1_IRQHandler, Default_Handler
.thumb_set EXTI2_IRQHandler, Default_Handler
.thumb_set EXTI3_IRQHandler, Default_Handler
.thumb_set EXTI4_IRQHandler, Default_Handler

.thumb_set DMA1_Channel1_IRQHandler, Default_Handler
.thumb_set DMA1_Channel2_IRQHandler, Default_Handler
.thumb_set DMA1_Channel3_IRQHandler, Default_Handler
.thumb_set DMA1_Channel4_IRQHandler, Default_Handler
.thumb_set DMA1_Channel5_IRQHandler, Default_Handler
.thumb_set DMA1_Channel6_IRQHandler, Default_Handler
.thumb_set DMA1_Channel7_IRQHandler, Default_Handler

.thumb_set ADC1_2_IRQHandler, Default_Handler

.thumb_set USB_HP_CAN1_TX_IRQHandler, Default_Handler
.thumb_set USB_LP_CAN1_RX0_IRQHandler, Default_Handler
.thumb_set CAN1_RX1_IRQHandler, Default_Handler
.thumb_set CAN1_SCE_IRQHandler, Default_Handler

.thumb_set EXTI9_5_IRQHandler, Default_Handler

.thumb_set TIM1_BRK_IRQHandler, Default_Handler
.thumb_set TIM1_UP_IRQHandler, Default_Handler
.thumb_set TIM1_TRG_COM_IRQHandler, Default_Handler
.thumb_set TIM1_CC_IRQHandler, Default_Handler

.thumb_set TIM2_IRQHandler, Default_Handler
.thumb_set TIM3_IRQHandler, Default_Handler
.thumb_set TIM4_IRQHandler, Default_Handler

.thumb_set I2C1_EV_IRQHandler, Default_Handler
.thumb_set I2C1_ER_IRQHandler, Default_Handler
.thumb_set I2C2_EV_IRQHandler, Default_Handler
.thumb_set I2C2_ER_IRQHandler, Default_Handler

.thumb_set SPI1_IRQHandler, Default_Handler
.thumb_set SPI2_IRQHandler, Default_Handler

.thumb_set USART1_IRQHandler, Default_Handler
.thumb_set USART2_IRQHandler, Default_Handler
.thumb_set USART3_IRQHandler, Default_Handler

.thumb_set EXTI15_10_IRQHandler, Default_Handler
.thumb_set RTCAlarm_IRQHandler, Default_Handler
.thumb_set USBWakeUp_IRQHandler, Default_Handler
