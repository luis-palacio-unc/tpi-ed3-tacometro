# Diseño candidato B07-R1

## Base real y decisiones

Servicios/Driver_Alarmas.h/.c mantiene una interfaz const de cuatro campos y
cinco estados. LED/Buzzer.h/.c aceptados en B12 exponen void(bool); no hay unión
directa actualmente. Consumidores reales: dos pruebas host B07 y el fixture C
generado en cmake/verificar_enlace_b07.cmake. main aún no consume el servicio.

| Decisión | Motivo |
|---|---|
| Callback void(bool), dos punteros sin contextos | Coincide exactamente con B12 y elimina datos/retornos sin uso real. |
| Mantener interfaz explícita y Aplicar | Conserva reemplazo de salidas y pruebas sin hardware, sin estado/init nuevos. |
| Conservar ACEPTADA/ERROR_ARGUMENTO | La validación previa evita una actuación parcial por interfaz incompleta; este error sí es observable. |
| Retirar errores LED/buzzer/ambos | El proveedor no informa fallos; no atribuir diagnóstico físico al servicio. |
| No incluir dispositivos en el servicio | El integrador elige las funciones; servicios permanece independiente del MCU. |

No crear funciones puente, casts de punteros de función, proveedor GPIO genérico o
compatibilidad con firmas descartadas. No renombrar la API ni agregar inicializador.

## Contrato y flujo

Driver_Alarmas_EscribirSalida pasa a void (*)(bool). Driver_Alarmas_Salidas contiene
escribir_led/escribir_buzzer. Driver_Alarmas_Aplicar conserva argumentos/retorno enum.
Validar interfaz completa primero; luego invocar LED(activar), buzzer(activar) y
devolver ACEPTADA. No sumar consultas eléctricas ni política térmica.
Funciones/interfaz deben permanecer válidas/estables durante la llamada; uso
serializado y callbacks que retornen. No se garantiza atomicidad/tiempo físico.

Ejemplo de composición a demostrar sólo en prueba: interfaz {LED_Write,Buzzer_Write}.
La adaptación B06 externa seguirá emitiendo sólo decisiones evaluables; un dato de
temperatura inválido no se convertirá en una orden de apagado. El retorno del
servicio informa argumento/despacho, no el estado del LED o del buzzer.

## Archivos y migración prevista

| Archivos | Cambio al implementar |
|---|---|
| Servicios/Driver_Alarmas.h/.c | Callback/estructura/enum y cuerpo, Doxygen ESW6/USW6.1. |
| tests/host/test_driver_alarmas.c | Salidas void separadas; orden, repetición, validación, recuperación e interfaces independientes. |
| tests/host/test_integracion_alarmas.c | Migrar interfaz, conservar B06 y reemplazar escenarios de fallo de salida por argumento inválido/recuperación. |
| tests/host/test_dispositivos_gpio.c | Añadir composición real B07-R1+B12 usando doble GPIO existente. |
| CMakeLists.txt | Enlazar servicio en target GPIO para esa composición; conservar controles existentes. |
| cmake/verificar_enlace_b07.cmake | Migrar callbacks/interfaz y controles de símbolos del fixture ARM. |
| README, docs/doxygen/README, docs/agente y cambio SDD | Documentar contrato/evidencia real; Doxyfile sólo si resulta necesario. |

No modificar código de B06/B12/Drivers ni historial. No quedan consumidores a
migrar en main. La sincronización del contrato vigente se hará sólo tras aceptación.

## Verificación prevista

Host: pruebas de secuencia/valores con registro independiente de callbacks, nulos
en cada posición, dos interfaces sin cruce e interfaz const. Integración térmica
existente y GPIO B12 directo sobre patrones/vecinos, sin mocks de Driver_Alarmas.
ASan/LSan/UBSan y regresiones. ARM: build y fixture adaptado, no ejecución.
Header/servicio aislado con sólo include de Servicios; Doxygen HTML/XML estricto,
hashes de biblioteca/B12/B06 y revisión de memoria/pila apropiada al código final.

Implementado según este diseño y verificado tras acuerdo2026-11-10; evidencia
real en verify-report.md. Configuración/enlace global y comprobación eléctrica
siguen para integración final. Resultado pendiente de aceptación.
