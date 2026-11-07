# B05 — Temperatura independiente del sensor

Estado: alcance acordado y aclarado por el usuario el 2026-11-07. Corrección de B05
en curso; commit autorizado, sin push. Resultado pendiente de aceptación.

## Objetivo y origen

R07/R09 requieren temperatura y validez; R15 exige abstracción de hardware también
en servicios. El usuario confirmó LM35 y aportó Datasheet_LM35_TI.pdf.
La versión fcd4392 colocaba su curva en Temperatura: se corrige esa responsabilidad.

## Alcance y comportamiento

- Dispositivos/LM35 convierte muestra ADC, cuentas por referencia y referencia µV
  a centésimas Celsius con estado. Aquí quedan ganancia nominal, validaciones eléctricas
  numéricas y redondeo. Sin programar ADC/DMA ni asumir variante/VREF reales.
- Servicios/Temperatura recibe temperatura física int32_t ya expresada en centésimas
  y estado genérico de fuente (válida, sin datos o error). No conoce LM35, ADC, VREF
  ni ganancias; entrega lectura/estado genéricos a aplicación.
- Fuente válida conserva el valor, incluido cero o signo negativo de otra fuente.
  Ausencia/error borra dato anterior; fuente válida posterior recupera.
- Inicialización, NULL, estado de fuente inválido y errores de conversión son explícitos.
- Documentar cuatro USW de servicio/dispositivo con Doxygen. Probar conversión,
  servicio sin enlazar LM35 e integración por adaptación de datos en pruebas.

Excluye adquisición física, pines, calibración, acondicionamiento LM35 para negativos,
filtrado, frescura, umbral/alarma, UART y main. Admitir Celsius negativos en el servicio
no implementa adquisición negativa con LM35. Conservar B00–B04 y sus archivos.

## Dependencias y archivos

B00–B04 aceptados. Modificar Temperatura.h/.c y LM35.h/.c; pruebas host, fixtures ARM,
CMake/Doxyfile, docs/agente/ y OpenSpec. No añadir callbacks o un framework para la
adquisición aún inexistente: el contrato actual intercambia magnitudes físicas/estado.
La composición con dispositivos reales se resolverá al integrar adquisición.

## Criterio de aceptación

1. LM35 mantiene referencias 250 mV →2500 y 1000 mV →10000 centésimas, escalas
   equivalentes, redondeo único, límites y errores verificables.
2. Temperatura acepta fuentes ya convertidas sin incluir/enlazar LM35 ni recibir
   parámetros ADC; conserva valores Celsius, diferencia ausencia/cero/error y recupera.
3. Integración propaga válida/sin datos/error sin filtrar errores específicos a aplicación.
4. Host/sanitizadores, enlace ARM, regresiones y Doxygen pasan; evidencia real por escenario.

## Riesgos y reversión

Riesgo: mover archivos y mantener el acoplamiento en la API. Verificar consumidor genérico
y enlaces separados. Corrección local adicional, sin reescribir fcd4392 ni publicar.
Revertir sólo su diff, preservando trabajo ajeno. VREF, variante y conexión siguen pendientes.
