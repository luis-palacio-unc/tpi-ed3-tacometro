# Tareas B05-R1 — implementación acordada

## 1. Acuerdo

- [x] 1.1 Revisar correcciones con Claude y acordar ejecución; leer ESTADO/Git y preservar ediciones locales. Aprobación r2 y ejecución acordadas por usuario.
- [x] 1.2 Contrastar delta TE1/TE3/TE4 con B05, conservando redondeo y TE2; registrar default provisional y riesgo ADGDR/ADDRn.

## 2. Dispositivo

- [x] 2.1 Crear Analogica.h: #ifndef ADC_VREF_MV/default2540 y #error para ≤0 o >INT32_MAX/10; documentar que no acredita VREF real.
- [x] 2.2 Definir ADC_DMA_LM35 en LM35.c/extern interno, preservando ADC_DMA_Pot y Drivers; sin configurar ADC/DMA.
- [x] 2.3 Reemplazar enum/registro/init/API anterior por LM35_Read int32_t y conversión entera B05; documentar USW11.3/11.4, retirando11.1/11.2 sin reciclar.

## 3. Pruebas y build

- [x] 3.1 Actualizar test_lm35.c: S1/S6, palabras limpias/con DONE/OVERRUN/CHN/bits3:0, actualización/repetición y memoria intacta.
- [x] 3.2 S2: targets con compile_definitions privados1000/2000/2540/3300/5000, build default y probes negativos0/−1/límite+1 por diagnóstico específico.
- [x] 3.3 S3: oráculo Fraction/floor independiente y comparación exacta de4096 códigos por configuración; empate128@1000→313.
- [x] 3.4 S7: entregar centésimas directamente en test_integracion_temperatura.c; preservar pruebas autónomas de TE2/fuentes/estados genéricos.
- [x] 3.5 Actualizar fixture ARM B05 LM35/includes/símbolos para nueva API, sin eliminar controles; medir objetosO0/O2/recursos/headers.
- [x] 3.6 Ejecutar host/san, ARM, Doxygen y controles de configuración (S8); registrar resultados reales separados de evidencia previa/Claude.

## 4. Revisión y entrega de implementación

- [x] 4.1 Revisar diff/hashes, confirmar biblioteca/servicios/otros dispositivos/contratos aceptados intactos y actualizar seguimiento.
- [x] 4.2 Dejar implementación verificada lista para auditoría y crear commit local; sin cierre, sincronización, archivo ni push por orden del usuario.

Implementación verificada; entrega con commit autorizada, sin cierre SDD/archivo.
Auditoría inicial de Claude con cambios; corrección verificada, pendiente de reauditoría/aceptación.

## 5. Corrección de auditoría de implementación

- [x] 5.1 H1: ampliar campos/EmitirTabla a todos los bits fuera de RESULT y fixture ARM; H2: corregir entrega directa en README; conservar LM35.c.
- [x] 5.2 Probar mutante sin máscara en copia externa antes/después; ejecutar host/san, ARM y Doxygen.
- [x] 5.3 Actualizar evidencia/ESTADO/MAPA/metadatos, revisar diff y commit local sin cierre/sincronización/archivo/push.
