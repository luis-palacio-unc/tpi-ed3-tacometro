# Tareas propuestas B16 — potenciómetro y dato ADC/DMA

## 1. Acuerdo y fuentes

- [ ] 1.1 Acordar alcance/APIs/estados y AD0.0/P0.23; releer ESTADO/Git y preservar biblioteca/módulos aceptados.
- [ ] 1.2 Confirmar formato ADGDR/macros y pendientes ADC.2, sin configurar periféricos.

## 2. Implementación y pruebas

- [ ] 2.1 Implementar ADC_Estado/ADC_Decode en soporte/Adquisicion_ADC_DMA.h/.c; Doxygen ESW18/USW18.1.
- [ ] 2.2 Implementar Pot_Read en Potenciometro.h/.c con canal0 y Doxygen ESW12/USW12.1.
- [ ] 2.3 Crear test_adc_datos.c para S1/S2: muestra/canal/flags, NULL, prioridades y recuperación.
- [ ] 2.4 Crear test_potenciometro.c para S3/S4 y composición con Mandos, sin cambiar servicios.
- [ ] 2.5 Ampliar CMake/Doxyfile conservando controles/18 fuentes y ejecutar pruebas individuales.

## 3. Verificación y cierre

- [ ] 3.1 Ejecutar host/san/regresiones y ARM/header/recursos/símbolos (S5), distinguiendo modalidad.
- [ ] 3.2 Ejecutar Doxygen y revisar diff/hashes de Drivers y contratos/módulos aceptados.
- [ ] 3.3 Actualizar informe/contexto/mapa/README; commit local según autorización, sin push.
- [ ] 3.4 Dejar resultado sin aceptar/sincronizar/archivar; no iniciar adquisición/integración física.
