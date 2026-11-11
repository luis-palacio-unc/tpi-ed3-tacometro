# Exploración B13 — UART

Lectura de2026-11-10, sin modificar fuentes ni ejecutar periféricos.

- UART.h/.c son esqueletos. Anteproyecto §1 exige UART PC bidireccional; anexoB
  asigna ESW.2.1.16. No fija periférico, baudios, formato ni buffers por IRQ.
- Motor exige decidir PWM; Sensor_Optico, conteo/captura; Potenciometro depende de
  adquisición ADC-DMA compartida pendiente. UART permite otro wrapper acotado con
  operaciones ya disponibles, sin adelantar inicialización global.
- Drivers/CMSISv2p00_LPC17xx/Drivers/src/lpc17xx_uart.c: UART_Send consulta THRE,
  llena hasta UART_TX_FIFO_SIZE=16 y retorna cantidad; UART_Receive consulta RDR
  y retorna cantidad. NONE_BLOCKING sale si no hay capacidad/datos. Reutilizarlo;
  no duplicar sus bucles, registros o timeouts. FIFO debe estar habilitada antes de TX.
- Driver/header ya admiten const uint8_t* en TX y uint8_t* en RX, uint32_t tamaños.
  UART_Read/UART_Write no colisionan con nombres existentes. UART0 macro del driver
  queda privada en .c. No se impone nueva capa genérica en Dispositivos.
- Manual NXP aportado UM10360 rev4.1: tabla80/PINSEL0, página PDF118, P0.2 TXD0/
  P0.3 RXD0. Tabla281/LSR, página317: RDR datos; THRE transmisor disponible;
  TEMT implica vaciado del transmisor. Contar escrituras a FIFO no prueba recepción PC.
- Lectura de documento de referencia, no instrucción del usuario. No se selecciona
  todavía cableado/conversor ni se presupone puerto serie por CMSIS-DAP integrado.

Recomendación: UART0 como recurso propuesto y dos funciones de transferencia sin
esperas añadidas. Son operaciones síncronas de duración dependiente del tamaño,
no un sistema de comunicación completo ni una garantía temporal. Diagnóstico de
overrun/paridad/framing y política de colas/servicio quedan pendientes de integración.

Acuerdo2026-11-10: UART0 confirmado; usuario pide pines del manual y baudios
configurables. Tabla80/p118 renderizada e inspeccionada: P0.2 TXD0/P0.3 RXD0,
función01. Driver UART_PinConfig ya contempla esos pines; reservar su uso para
inicialización final. UART_BAUDRATE default115200 modificable en UART.h; sin init
nuevo ni afirmar baudios aplicados. Render Poppler produjo avisos de caché fontconfig
por falta de ruta escribible; imagen generada/inspeccionada legible, sin ocultarlos.
