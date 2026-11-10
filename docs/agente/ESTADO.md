# Estado — 2026-11-09

## Punto actual

- Repositorio: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD/remoto al incorporarse.
- B00–B10 y B03-R1 aceptados/archivados. B10 aceptado el 2026-11-09;
  mandos sincronizado: MA3/S6 modificado, SE1–SE4 agregados; MA1/MA2/MA4
  conservados, total ocho requisitos/13 escenarios.
- Implementación B10: 25f423ecb8311afd09e7c4ec50e0ed6bc416c3dc. Cierre/propuesta en commit
  `docs: aceptar B10 y proponer B11` (eeef43a). Push autorizado pero rechazado:
  remoto avanzó a bd7bc0b (README: repositorio público). Integración prevista sin
  conflictos, sólo esa línea; merge pendiente de autorización separada del usuario.
  Rechazo/vista previa en /tmp/ed3-cierre-b10-20261109-_2p72mp4.
- Cambio activo b11-base-tiempos-lpc1769: sólo propuesta; alcance/criterio
  pendientes de acuerdo, sin implementación ni publicación futura autorizadas.

## Evidencia aceptada y cierre

- B10: host/san 17/17, 179 C (14 selector + cuatro integración), referencia
  4096 muestras/400 rollovers, codec 5096 pares y velocidad 11199 ternas.
  Refuerzo de aserciones: selector host/san 1/1; ARM Debug 13/13 y fixture B10
  Release; Doxygen estricto 32 funciones/USW; negativos detectados.
- Matriz, recursos y límites en informe archivado B10.
- Cierre documental: merge por requisitos, hashes/manifiesto, YAML/enlaces y diff;
  sin cambios de firmware ni pruebas nuevas. Logs externos: /tmp/ed3-cierre-b10-20261109-_2p72mp4.
- Sin ejecución ARM, simulador o placa; sin flasheo/instalación/cambios sensibles MCU.

## Pendientes y próxima acción

Acordar B11 propuesto: base temporal en Dispositivos/soporte con SysTick, 1 ms y
frecuencia CPU explícita, sin PLL ni integración main. Lectura compartida/ISR,
parada/reinicio y límites de precisión deberán especificarse y probarse.
Reloj físico, GPIO/selector, ADC/DMA/captura/ventana RPM de 1 s, UART/TX,
PWM/potencia/regulación y main pendientes. Fuente temporal no es orden de eventos B01.
Política física/reentrada tras pérdida y umbral térmico TPI aún pendientes.
CMSIS-DAP integrado; revisión PCB/silicio/pines/VREF sin verificar.
Diagramas finales después de completar ESW, según CONTEXTO.
No implementar B11 ni publicar trabajo futuro sin acuerdo.
