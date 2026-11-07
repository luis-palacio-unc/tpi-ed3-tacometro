# Diseño B04 — Normalización del mando manual

## Enfoque y decisiones

Implementar sólo la conversión lógica de Mandos, ESW.2.1.5 según anexo B del
anteproyecto (página 5 verificada en texto e imagen). Dos USW: Inicializar .1 y
NormalizarManual .2. El selector y adquisición física permanecen pendientes.

| Opción | Compromiso | Decisión y motivo |
|---|---|---|
| Escala fija del ADC / parámetro | Fijarla ata servicio a una resolución | Escala uint32_t explícita; aplicación mantiene 0..1000 |
| Entradas uint16_t / uint32_t | 16 limita resoluciones sin requisito | uint32_t para lectura/escala; salida uint16_t compatible con B01 |
| float / entero escalado | float agrega operaciones y redondeos | Enteros, cercano/empate arriba; error <=0,5 por mil |
| Producto uint32_t / uint64_t | 1000×UINT32_MAX excede 32 bits | Temporal uint64_t y libgcc ya integrado por B03 |
| Saturar / informar error | Saturar oculta fallos de adquisición/escala | Invalidar con cero y estado explícito; próxima entrada válida recupera |
| Contexto+getter / registro de salida | Getter duplica copia sin requisito | Mandos_MandoManual explícito, sin estado oculto |
| Límite duplicado / configuración común | Duplicar permite rangos distintos | TPI_MANDO_MANUAL_MAX=1000 en Limites.h; aliases públicos B01/Mandos |

## API, flujo y errores

Mandos_MandoManual: uint16_t por_mil y Mandos_Estado estado. Inicializar(mando*)
devuelve bool y escribe SIN_DATOS/cero; NULL devuelve false. NormalizarManual(
mando*, uint32_t lectura, uint32_t fondo_escala) devuelve estado y reemplaza ambos
campos sin leer contenido anterior. NULL: ERROR_ARGUMENTO; escala cero: ERROR_ESCALA;
lectura>escala: ERROR_LECTURA; resto: VALIDO. Prioridad en ese orden.
SIN_DATOS sólo tras inicio/reinicio; cero válido distinto de ausencia. Sin timeout.
El integrador consume sólo VALIDO; no confundir cero de error con nueva orden válida.

Captura futura coherente lectura/escala → Mandos → adaptación en aplicación → B01.
Capturador debe validar adquisición, overflow previo y correspondencia de escala;
el normalizador no detecta un ADC defectuoso que entrega valores dentro de rango.
Serializar escritura/lectura externa; no compartir directamente entre ISR y lazo.
No se integra main. Servicio incluye sólo Limites.h/tipos estándar, nunca aplicación.

## Aritmética y recursos

Tras validar: n=(uint64_t)lectura×1000; q=(n+escala/2)/escala; estrechar a uint16_t.
Numerador con redondeo <=4 297 114 778 647 <UINT64_MAX para todo uint32_t.
lectura<=escala garantiza 0<=q<=1000; cabe en uint16_t. No saturación ni división
por cero, ni recorte de precisión antes del producto. Factor 1000 tiene fuente única.
Pocos bytes por salida (medir ABI ARM), sin buffers, heap o globals mutables.
Reutilizar enlace -lgcc/-z noexecstack y controles ELF B00; medir código y pila estática
incluido auxiliar visible. No afirmar tiempos ni profundidad real de pila.

## Archivos

| Archivo | Cambio |
|---|---|
| Servicios/Mandos.h/.c | API y dos USW con contratos Doxygen, sin comentarios dentro del cuerpo |
| Configuracion/Limites.h | Límite manual común; TPI_RPM_MAX intacto |
| Aplicacion/Procesamiento.h | PROCESAMIENTO_MANDO_MAX pasa a alias, mismo valor/API/comportamiento |
| tests/host/test_mandos.c | Escalas, redondeo, fallos, recorrido, fronteras y adaptación B01 |
| cmake/verificar_enlace_b04.cmake | Retener API Mandos/B01 y auxiliares a -O0/-O3 |
| CMakeLists.txt, docs/doxygen/Doxyfile | Integrar host/ARM/sanitizadores y documentación incremental |
| README, docs/agente/, openspec/ | Estado y evidencia; sin promover specs ni editar archivos aceptados |

## Verificación

Casos de referencia independientes, recorrido completo de escalas 1023/4095 y otras,
prueba de monotonía/límites, fronteras hasta UINT32_MAX y barrido determinista con
oráculo de distancias enteras. Sin recalcular el cociente de producción en el oráculo.
Adaptador de test/fixture a Procesamiento manual conserva el mando; sin añadir API allí.
Host estricto C11 y ASan/LeakSanitizer/UBSan fuera de ptrace; regresiones completas.
Fixture ARM Debug/Release, símbolo de división resuelto y ELF sin indefinidos.
Doxygen estricto HTML/XML: comprobar USW.2.1.5.1/.2 y alias/constante común.
Sin TDD configurado: modo estándar. Sin preguntas indispensables, instalaciones ni
ejecución ARM/simulador/placa. Contrato vigente B01 conserva comportamiento y valor 1000.
