# B11 replanteado — Timer continuo y wrapper temporal

Estado: alcance/criterio acordados el 2026-11-10; implementación y commit local
autorizados, sin push. Resultado pendiente de aceptación. Sustituye candidato
SysTick db88286 no aceptado, conservando historia.

## Objetivo y origen

Nueva instrucción del usuario y R01/R02/R08/R09/R10/R11/R15: tiempo común producido
por un timer físico, registros sólo en dispositivos y consultas mediante wrapper.
Base inicial 1 ms configurable; conservar unidades ms de B10.

## Alcance y comportamiento

- Reservar TIMER0 exclusivo: contador TC continuo, sin ISR periódica, acumulador
  software ni reinicio por consultas/match. Inicializar, leer, detener y reiniciar.
- Accesos TC/prescaler/alimentación/modo sólo en Dispositivos/soporte/Base_Tiempos
  y adaptador LPC1769. Retirar ISR propia y controles específicos SysTick.
- Configuracion/Temporizacion.h: base predeterminada 1 ms; parámetro de inicialización
  en ms enteros positivos, fijo por época. Admitir otros valores representables
  (ejemplos 2/5 ms), rechazar cero/división inexacta/desbordes/configuración inválida.
- PCLK real explícito y estable; no asumir reloj CPU ni configurar PLL/PCLKSEL/flash.
  Dispositivo normaliza conteos a marcas uint32_t ms modulares. Cambia resolución,
  no unidades del consumidor; reiniciar/reconfigurar exige nueva época y marcas.
- Servicios/Tiempo.h/.c será el wrapper: proveedor reemplazable como B08, sin CMSIS,
  TC o modelo de MCU; obtener tiempo y calcular transcurrido desde marca propia.
  Estado/errores explícitos; sólo lectura disponible habilita consumo. Resta modular,
  misma época y ventanas <2^31 ms; sin esperas/heap. Aplicación consulta vía servicio.
- Composición con B10 sólo en pruebas/fixture, sin cambiar Mandos/main.

Excluye adquisición/ventana RPM 1 s, planificador, alarmas nuevas, GPIO/ADC/DMA/UART/
PWM/regulación, arranque integrado y flasheo. No instala dependencias.

## Dependencias y archivos

B00–B10/B03-R1 aceptados. Base_Tiempos.*, nuevos Tiempo.* y Temporizacion.h,
tests/modelos/fixture, CMake, Doxygen y seguimiento. PCLK/configuración física
pendientes de verificar; TIMER0 es reserva propuesta. API/errores/USW se detallarán
al acordar el bloque. Antecedente SysTick conserva propuesta/spec/diseño/evidencia.

## Secuencia y aceptación

1. Especificación/diseño/tareas nuevos, luego implementación y revisión del diff.
2. Probar prescaler, límites/errores, ciclo de vida y avance TC sin consultas/ISR;
   conservar registros ajenos y uso exclusivo del timer.
3. Probar consultas sin reinicio, consumidores independientes, tiempo/intervalos
   con bases 1/2/5 ms y desbordamientos TC/ms; cero válido distinto de ausencia.
4. Probar wrapper reemplazable sin MCU y B10 conservando intervalos en ms.
5. Pasar host/sanitizadores/regresiones, ARM Debug/Release y Doxygen estricto;
   documentar recursos. Precisión física queda pendiente. Commit local al terminar,
   sin push/sincronizar/archivar hasta orden.

## Riesgos y reversión

PCLK incorrecto, menor resolución o épocas mezcladas: validación y límites explícitos.
Revertir mediante commit inverso, preservando historia y trabajo ajeno.
