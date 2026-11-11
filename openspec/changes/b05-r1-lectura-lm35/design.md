# Diseño propuesto B05-R1 — corregido tras revisión de Claude

## Flujo y API (H1)

ADC → DMA → ADC_DMA_LM35 → LM35_Read → LM35_Convert → centésimas Celsius.
int32_t LM35_Read(void) público devuelve2500 para25,00°C. Conversión interna
static int32_t LM35_Convert(uint16_t muestra). Sin float/double, enum, contexto,
salida por puntero, inicializador o caché de temperatura; se calcula al consultar.
Se conserva LM35_CENTESIMAS_POR_GRADO=100 porque la unidad sigue vigente.

| Decisión | Motivo |
|---|---|
| Muestra volatile uint32_t en LM35.c, extern interno | Propiedad del dispositivo como B16; destino futuro DMA con RESULT15:4. |
| ADC_GDR_RESULT original, una consulta | Reutiliza biblioteca; ignora campos ajenos sin validar lecturas. |
| int32_t centésimas | Coincide con TE2/alarma/telemetría; elimina adaptación de unidades. |
| Conversión entera con redondeo B05 | Conserva comportamiento numérico aceptado, sin auxiliares de punto flotante. |

m1: nombrar LM35_ADC_BITS/ADC_CUENTAS/MV_POR_GRADO y derivar
CENTESIMAS_POR_MV=CENTESIMAS_POR_GRADO/MV_POR_GRADO (100/10 exacto).
Usar N=(uint64_t)muestra×ADC_VREF_MV×CENTESIMAS_POR_MV; D=ADC_CUENTAS; q=N/D,r=N%D;
resultado=q+(r≥D/2). Sin redondear primero a tensión ni truncar el resultado final.
Error≤0,005°C es cuantización aritmética, no exactitud eléctrica. Con referencia
≤INT32_MAX/10 y muestra≤4095, N cabe en uint64_t y el redondeado cabe en int32_t.
El guard de configuración asegura este dominio, sin chequeos por cada lectura.

## Referencia y builds (H2/H7)

Configuracion/Analogica.h incluye stdint.h. Seguir UART.h/Timer.h:

```c
#ifndef ADC_VREF_MV
#define ADC_VREF_MV UINT32_C(2540)
#endif
#if ADC_VREF_MV <= 0 || ADC_VREF_MV > INT32_MAX / 10
#error "ADC_VREF_MV debe ser positivo y permitir salida int32_t en centesimas."
#endif
```

Configuración acordada e implementada en Analogica.h. Default2540mV provisional para
compilar firmware sin flags: el anexoA/p4 del anteproyecto rotula +2V54DC_VREF,
pero no acredita tensión/cableado de la placa. El usuario difiere el valor físico
a integración; cualquier default se documentará como software provisional.

Targets host separados pruebas_lm35_vref_1000/2000/2540/3300/5000 contienen el test
y LM35.c. Cada uno usa target_compile_definitions(... PRIVATE ADC_VREF_MV=<valor>)
para todas sus unidades; no cambiar definiciones del target firmware. Un target
sin override comprueba default2540 y lectura1024→6350. Sólo pasar -D a CMake no
define una macro C: los overrides se aplican al compilador de cada target.
Probes C11 incluyen Analogica.h con -DADC_VREF_MV=0 y=-1: fallo esperado sólo si
el diagnóstico identifica ADC_VREF_MV. Agregar caso superior INT32_MAX/10+1.
Fixtures ARM usan override explícito de prueba; build firmware tiene default.

## Pruebas y compatibilidad (H4)

S1 usa muestras0/512/2048 con palabras limpias y OR0xC700000F: DONE31, OVERRUN30,
CHN=7 y bits3:0 activos. S6 cambia0xC700200F→0xC300800F→0x00008000, repitiendo
lecturas y comprobando memoria intacta. Estos datos prueban extracción, no origen.
S3 usa Python estándar Fraction y floor(x+Fraction(1,2)) como oráculo exacto sobre4096 códigos
por cada build; la producción usa cociente/resto. Comparar enteros por igualdad,
con empate explícito VREF1000/muestra128→313; sin tolerancia float.
S7 pasa las centésimas directamente a Temperatura_Actualizar. Estados genéricos
vienen del fixture; TE2, servicios, alarma, telemetría y main permanecen intactos.

## Riesgo de integración: ADGDR y dos destinos (H3)

ADGDR almacena el último canal convertido. Un desfase de peticiones/LLI puede
escribir potenciómetro en destino LM35 y viceversa. Getters ignoran CHN; RESULT
puede parecer válido y disparar/enmascarar alarma. Dos variables no garantizan ruteo.
Evaluar ADDRn/AD0DRn por canal como origen, verificando UM10360 ADC/GPDMA y erratas
LPC176x (incluida ADC.2), disparos/coherencia y API original. La biblioteca actual
selecciona ADGDR desde GPDMA_LUTPerAddr para P2M y sobreescribe srcMemAddr: no basta
cambiar ese parámetro. Viabilidad de ADDRn no verificada; no modificar Drivers.
Resolver al integrar; revisar explícitamente contrato B16 si cambia el origen,
sin editarlo ahora ni alterar su archivo histórico. No bloquea esta revisión.

Muestra32bits alineada en .bss local, MCU activo; DMA único escritor productivo.
Una lectura volatile no garantiza edad/coherencia multicanal ni sincroniza hilos C.
Primer dato, canal/pin, ADC/DMA y revisión de memoria si Sleep quedan para el final.

## USW y archivos (H6)

Retirar USW.2.1.11.1 (init) y .11.2 (antigua API pública). No reciclar IDs:
.11.3=LM35_Read; .11.4=LM35_Convert estática nueva. Se conserva la regla matemática
B05, pero firma/visibilidad/contrato son nuevos; Doxygen EXTRACT_STATIC=YES.

m2: test_lm35.c enlaza sólo con LM35.c; ADC_DMA_LM35 tiene una única definición
y soporte no define datos. No incluir Adquisicion_ADC_DMA.c en esos targets.
Modificar LM35.h/.c, extern de soporte, nueva Analogica.h, test_lm35.c y
test_integracion_temperatura.c. Ajustar CMake/fixture ARM retenido B05 y Doxyfile
si corresponde; no exigir auxiliares retirados que el compilador ya no use.
Planificar host/san, ARM/O0/O2/headers, símbolos/recursos y Doxygen/diff/hashes.
H5 específico de float queda sin efecto. Sin ensayos físicos ni integración nueva.

m3: [resumen versionado](review-notes.md) de las revisiones externas locales;
no dependen de rutas o archivos disponibles sólo en la máquina del autor.
