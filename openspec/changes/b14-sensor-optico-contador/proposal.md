# B14 — Lectura del contador del sensor óptico

Estado: propuesta pendiente de acuerdo; no implementar todavía.

## Objetivo y origen

R02/R04/R09/R10/R11/R15: obtener conteos del sensor óptico para la medición genérica
B03, completando ESW.2.1.10 sin adelantar la integración. B13 aceptado/archivado.

## Alcance

- SensorOptico_Read(void) devuelve uint32_t de pulsos acumulados mediante
  TIM_ReadTimer(LPC_TIM1), sin conversiones a tiempo/RPM ni recorte de185.
- Proponer TIMER1 como contador externo ascendente por CAP1.0/P1.18, función11
  de PINSEL3. TIMER0 conserva base temporal de B11. Un flanco ascendente por ranura;
  factor60 permanece en configuración, fuera del dispositivo/servicio genérico.
- Lectura sin reiniciar/detener/configurar contador, sin ISR, contexto, buffers,
  estado mutable, detección de parada o lógica de ventanas.
- Documentar ESW10/USW.2.1.10.1 con Doxygen. Comprobar lectura y compatibilidad
  con medición B03 mediante pruebas host con fuente emulada y compilación ARM.

Archivos: Dispositivos/Sensor_Optico.h/.c, tests/host/test_sensor_optico.c,
CMakeLists.txt, Doxyfile, README y seguimiento SDD. No modificar Drivers, Timer,
Medicion_Velocidad, otros módulos ni archivos archivados.

Excluye PinConfig/InitCounter/Enable reales, configuración eléctrica/reloj,
sincronización de ventanas, main/servicios, regulación y ensayos físicos.

## Dependencias, pendientes y aceptación

Manual UM10360 tabla83/p119 y §21.6.3–4/p505–506 confirman entrada/contador32bits.
Biblioteca ya ofrece TIM_PinConfig, TIM_InitCounter, TIM_Enable y TIM_ReadTimer;
sólo la última se usa aquí. Pinmux/arranque y validación de señal/placa quedan al final.
Al usarlo: TIMER1 exclusivo/configurado/arrancado, CAP1.0 en modo contador sin
captura/IRQ del mismo canal ni match que reinicie/detenga; acceso serializado.

1. Lecturas devuelven conteos exactos, incluido0/MAX, y no alteran TIMER0 ni el contador.
2. Prueba de composición calcula diferencias fuera del módulo, incluidas60 cuentas
   cruzando rollover, y obtiene60 RPM con B03/factor60/1s; sin recortar mediciones.
3. Nuevo arranque exige nueva referencia; menos de2^32 pulsos entre muestras para
   diferencia modular inequívoca. No afirmar adquisición/tiempos físicos por host.
4. Host/sanitizadores, build ARM, regresiones/Doxygen pasan; bibliotecas intactas.

Riesgo: confundir TC contador con milisegundos o reiniciarlo al leer. Contrato explícito
y prueba de diferencias/medición lo detectan. Reversión: commit inverso sólo B14.
Implementar tras acuerdo, commit local al terminar, sin push.
