/* Startup minimo LPC1769, GNU assembler. No configura relojes/perifericos. */
.syntax unified
.cpu cortex-m3
.thumb
.section .isr_vector,"a",%progbits
.balign 4
.global __vector_table
__vector_table:
.word __stack_top__
.word Reset_Handler
.word NMI_Handler
.word HardFault_Handler
.word MemManage_Handler
.word BusFault_Handler
.word UsageFault_Handler
.word __vector_checksum__
.word 0
.word 0
.word 0
.word SVC_Handler
.word DebugMon_Handler
.word 0
.word PendSV_Handler
.word SysTick_Handler
.word WDT_IRQHandler
.word TIMER0_IRQHandler
.word TIMER1_IRQHandler
.word TIMER2_IRQHandler
.word TIMER3_IRQHandler
.word UART0_IRQHandler
.word UART1_IRQHandler
.word UART2_IRQHandler
.word UART3_IRQHandler
.word PWM1_IRQHandler
.word I2C0_IRQHandler
.word I2C1_IRQHandler
.word I2C2_IRQHandler
.word SPI_IRQHandler
.word SSP0_IRQHandler
.word SSP1_IRQHandler
.word PLL0_IRQHandler
.word RTC_IRQHandler
.word EINT0_IRQHandler
.word EINT1_IRQHandler
.word EINT2_IRQHandler
.word EINT3_IRQHandler
.word ADC_IRQHandler
.word BOD_IRQHandler
.word USB_IRQHandler
.word CAN_IRQHandler
.word DMA_IRQHandler
.word I2S_IRQHandler
.word ENET_IRQHandler
.word RIT_IRQHandler
.word MCPWM_IRQHandler
.word QEI_IRQHandler
.word PLL1_IRQHandler
.word USBActivity_IRQHandler
.word CANActivity_IRQHandler

.section .text.Reset_Handler,"ax",%progbits
.balign 4
.global Reset_Handler
.type Reset_Handler,%function
.thumb_func
Reset_Handler:
    ldr r0, =__data_load__
    ldr r1, =__data_start__
    ldr r2, =__data_end__
1:
    cmp r1, r2
    bhs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__
    movs r3, #0
3:
    cmp r1, r2
    bhs 4f
    str r3, [r1], #4
    b 3b
4:
    bl main
5:
    b 5b
.size Reset_Handler, . - Reset_Handler
.ltorg

.section .text.Default_Handler,"ax",%progbits
.global Default_Handler
.type Default_Handler,%function
.thumb_func
Default_Handler:
    b .
.size Default_Handler, . - Default_Handler

.macro default_handler name
    .weak \name
    .thumb_set \name, Default_Handler
.endm
default_handler NMI_Handler
default_handler HardFault_Handler
default_handler MemManage_Handler
default_handler BusFault_Handler
default_handler UsageFault_Handler
default_handler SVC_Handler
default_handler DebugMon_Handler
default_handler PendSV_Handler
default_handler SysTick_Handler
default_handler WDT_IRQHandler
default_handler TIMER0_IRQHandler
default_handler TIMER1_IRQHandler
default_handler TIMER2_IRQHandler
default_handler TIMER3_IRQHandler
default_handler UART0_IRQHandler
default_handler UART1_IRQHandler
default_handler UART2_IRQHandler
default_handler UART3_IRQHandler
default_handler PWM1_IRQHandler
default_handler I2C0_IRQHandler
default_handler I2C1_IRQHandler
default_handler I2C2_IRQHandler
default_handler SPI_IRQHandler
default_handler SSP0_IRQHandler
default_handler SSP1_IRQHandler
default_handler PLL0_IRQHandler
default_handler RTC_IRQHandler
default_handler EINT0_IRQHandler
default_handler EINT1_IRQHandler
default_handler EINT2_IRQHandler
default_handler EINT3_IRQHandler
default_handler ADC_IRQHandler
default_handler BOD_IRQHandler
default_handler USB_IRQHandler
default_handler CAN_IRQHandler
default_handler DMA_IRQHandler
default_handler I2S_IRQHandler
default_handler ENET_IRQHandler
default_handler RIT_IRQHandler
default_handler MCPWM_IRQHandler
default_handler QEI_IRQHandler
default_handler PLL1_IRQHandler
default_handler USBActivity_IRQHandler
default_handler CANActivity_IRQHandler

.section .note.GNU-stack,"",%progbits
