# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  Base cf17359; publicado62eef53. Consultar HEAD/Git al reanudar; sin push autorizado.
- B00–B14/B03-R1/B07-R1 aceptados/archivados. Activo B15 simplificado por usuario,
  implementado/verificado, sin aceptar. No sincronizar/archivar ni iniciar otro bloque.
- MR0 fijo1000 ticks como configuración requerida para inicialización final.
  bool Motor_Write(uint16_t mando_por_mil) valida0..1000 y pasa el valor exacto
  a PWM_MatchUpdateSingle(PWM_MATCH_1,mando_por_mil), incluido1000 sin cambio.
  Sin lectura/cálculo de match/conversión/redondeo/64bits/init/estado propios.
  false rechaza >1000 sin llamadas; true significa despacho lógico.
- Modelo RAM/preinclude de pruebas anterior retirado. Doxygen USW9.1 actualizado.
  Drivers90 originales a21cc1e, servicios/otros dispositivos y contratos/archivos
  aceptados intactos. Commit local al terminar, sin push/merge/reescritura.

## Evidencia actual ejecutada

| Modalidad | Resultado |
|---|---|
| Host/san | 21/21 cada uno,204 casos C+oráculo Python; sin hallazgos |
| B15 | Cuatro casos,1001 mandos válidos exactos/64535 rechazos y recuperación |
| Regresiones | 11199 ternas velocidad,5096 pares codec,4096 muestras/400 rollovers selector |
| ARM | Build/controles13/13, MotorO0/O2/headerC11/C++17; no ejecución MCU |
| Doxygen | 42 funciones HTML/XML sin avisos; un parámetro en Motor_Write |
| Negativos | Detectan canal equivocado,ajuste1000→1001 y omisión del rechazo |
| Recursos propios | .text42-O0/24-O2,.data/.bss0,marco16/8 bytes; no cota total |
| Preservación | 270 archivos protegidos,90 Drivers originales,firmware18 objetos |

[Informe actual](../../openspec/changes/b15-dispositivo-motor-pwm/verify-report.md).
Logs: /tmp/ed3-b15-directo-2yz3lskk. Evidencia anterior de conversión/períodos variables queda en Git,
no demuestra contrato actual. Dobles sólo de API de escritura, sin biblioteca/
CPU/registro/latch/señal/motor físicos. Sin flasheo/instalación/publicación.

## Próxima acción y límites

Revisar/aceptar B15 directo antes de sincronizar/archivar. Configurar MR0=1000
con drivers al integrar; PWM1.1/P2.0 single-edge alto, MR0 efectivo/estable,
reset por MR0 sin otros matches que alteren TC y acceso exclusivo serializado.
Función no detecta configuración incorrecta. Fuente/corrección100 % y PWM.1
registradas en exploración; validación física de PWM.1 pendiente en integración.
Frecuencia/reloj/prescaler/pinmux/arranque/potencia/adaptación B08/main al final.
TIMER0/base y TIMER1/CAP1.0/P1.18 del XD-51 conservados; señal/ventana1s pendientes.
Quedan Potenciometro, ADC-DMA/LM35, regulación e integración física final.
No retomar B12 diferido; diagramas del informe al terminar todos los ESW.
