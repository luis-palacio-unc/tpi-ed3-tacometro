# Fuentes y decisiones B05-R1 — corregidas2026-11-11

Revisión externa local leída completa: B05-R1-propuesta.md (no distribuida),
incluida «Decisiones del usuario». El usuario ordena aplicar H1/H2/H3/H4/H6 y el
mecanismo de builds de H7, sin implementar. H5 específico de float queda sin efecto.

H1: centésimas enteras int32_t, como TE2/alarma/telemetría. Se conserva redondeo
B05 al cercano/empate arriba y error≤0,005°C. Se retira propuesta float, sin introducir
adaptador de unidades o estados LM35. S3 comparará enteros exactamente en4096 códigos.

H2: el anteproyecto.pdf anexoA/p4 sí rotula [+2V54DC_VREF]; se verificó página completa
y diagrama en esta revisión. Corrige la exploración anterior que omitía ese dato.
Default provisional2540mV para build sin overrides, configurable #ifndef/#error según
UART.h/Timer.h; no confirma VREF real de placa, diferida por decisión del usuario.
TI SNIS159H §7.3.1/p13:10mV/°C. UM10360 rev4.1 §29.5.2/p589: RESULT15:4.
S2 fija overrides de compilador por target y probes no positivos; no basta -D de CMake.

H3: último canal en ADGDR más dos destinos puede mezclar potenciómetro/LM35 si se
desfasa el ruteo; getters no comprueban CHN. Evaluar ADDRn por canal al integrar,
sin dar por verificada su compatibilidad con UM10360 ADC/GPDMA/erratas. Driver
actual asocia ADC a ADGDR y P2M sustituye srcMemAddr por GPDMA_LUTPerAddr:
cambiar un parámetro de dirección no garantiza otro origen. Drivers intocables.

H4: palabras limpias y decoradas con DONE/OVERRUN/CHN/bits3:0 en S1/S6.
H6: IDs11.1/11.2 retirados, sin reciclaje. Getter11.3 y conversión interna nueva11.4;
la regla numérica se conserva aunque cambien firma/visibilidad/contrato de unidad.

TE1/TE3/TE4 candidatos sustituyen la API de dispositivo, conservando TE2. Dependencias:
LM35.h/.c, tests/host/test_lm35.c, test_integracion_temperatura.c y fixture ARM B05.
Sin llamadas productivas desde Aplicacion/Servicios; sin cambiar B16/contratos/archivo.
Propuesta aprobada por Claude r2, implementación autorizada e incorporada con m1–m3.
Verificación propia en verify-report.md; auditoría posterior del código pendiente.
[Resumen público de revisiones locales](review-notes.md). No atribuir experimentos
de Claude a la verificación propia del nuevo código; no modificar su harness.
