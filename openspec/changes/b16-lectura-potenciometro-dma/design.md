# Diseño B16 — getter simple

## Decisión acordada

ADC convierte, DMA transfiere y uint16_t Pot_Read(void) devuelve la lectura.
Sin parámetros, salida por puntero, estados, validaciones, callbacks o contexto.
Se retira el decodificador/enum de la primera propuesta. Los contratos aceptados
de servicios no cambian; Mandos conserva la normalización posterior.

Flujo: ADC → DMA → ADC_DMA_Pot → Pot_Read → valor numérico de0..4095.

## Datos y responsabilidades

ADC_DMA_Pot es una única palabra volatile uint32_t, definida y propiedad de
Potenciometro.c. Adquisicion_ADC_DMA.h conserva sólo la declaración extern interna
para que el soporte configure su dirección como destino DMA al integrar. La palabra conserva el formato ADGDR completo;
Pot_Read aplica una vez
ADC_GDR_RESULT original a esa posición y devuelve uint16_t. RESULT ocupa15:4;
su límite4095 hace segura la conversión. No se interpretan DONE/OVERRUN/canal.
No hay función intermediaria, copia persistente o validación añadida.

La integración garantizará origen AD0.0/P0.23 función01 y destino de32 bits alineado,
sin escritores CPU concurrentes. No basta alternar canales con un destino común:
el soporte debe mantener en esta posición solamente la muestra del potenciómetro.
Secuencia multicanal/LM35, ADC.2, configuración, arranque y enlace al final.

## Memoria y concurrencia

La palabra ocupa4 bytes en .bss de SRAM local; UM10360 rev4.1 §1.9/p12 confirma
acceso CPU/DMA con el microcontrolador activo. No cambia linker/startup ni reserva
banco AHB. Si se incorpora Sleep, revisar ubicación/relojes según §4.8.1.
volatile obliga a consultar memoria; no sincroniza hilos C. En Cortex-M3, la lectura
alineada de32 bits accede a una palabra. Durante una transferencia puede devolver
el valor anterior o el nuevo; no promete antigüedad ni coherencia entre canales.
La CPU no escribe al ejecutar. Cero inicial de .bss no acredita una conversión.
Sin espera, heap, IRQ, caché, limpieza, normalización o señal de disponibilidad.

## Archivos y verificación

- Potenciometro.h/.c: getter y Doxygen ESW12/USW.2.1.12.1; header sólo stdint.h.
- soporte/Adquisicion_ADC_DMA.h/.c: declaración extern y configuración diferida;
  sin almacenamiento propio del potenciómetro.
- tests/host/test_potenciometro.c: tres casos, palabras constantes en memoria;
  referencias0/2048/4095, actualización, repetición y no escritura. Flags ignorados.
  El ejecutable enlaza sólo con Potenciometro.c; no necesita el .c de soporte.
- CMake/Doxyfile y seguimiento: se amplían pruebas/docs, conservando los controles.

Corrección: host/sanitizadores, ARM O0/O2, propiedad de símbolo y Doxygen ejecutados;
evidencia en verify-report.md. Las pruebas preparan memoria desde PC y no acreditan
conversión ADC, transferencia DMA, temporización o señal eléctrica.
Biblioteca/servicios/otros dispositivos/contratos aceptados intactos.
