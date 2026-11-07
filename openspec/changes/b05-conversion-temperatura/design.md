# Diseño B05 — Conversión de temperatura

## Decisiones

ESW.2.1.4 según anexo B; USW.2.1.4.1 Inicializar y .2 Actualizar. Propuesta aceptada
para conversión ideal LM35 sin offset; captura y alarma permanecen fuera del bloque.

| Opción | Compromiso | Decisión y motivo |
|---|---|---|
| VREF/resolución fijas / parámetros | Fijarlas ata cálculo a la placa | Muestra, escala_adc y referencia_uv uint32_t por llamada |
| Último código / cuentas por referencia | Son magnitudes distintas | Escala es divisor de conversión; muestra<escala. Para 12 bits, ejemplo 4096 y códigos 0..4095 |
| Volver a redondear tensión / cálculo directo | Dos redondeos pueden cambiar resultado | Una conversión racional a centésimas de °C |
| Float / entero | Float innecesario para contrato | uint64_t intermedios; uint32_t salida no negativa, modelo sin offset |
| Sumar medio divisor / cociente y resto | La suma puede superar UINT64_MAX con entradas válidas | Dividir y usar resto para redondear; sin sumar al numerador |
| Saturar o conservar viejo / invalidar | Oculta error de configuración/adquisición | Estado explícito y cero; siguiente dato válido recupera |
| Contexto y getter / salida por instancia | Getter duplica copia sin requisito | Temperatura_Lectura, dos funciones, sin estado oculto |

Escala B05 no cambia B04: Mandos usa máximo inclusivo para mapear extremos del mando;
Temperatura usa cuentas por referencia para convertir unidades. Son contratos diferentes.
NXP fig.28 muestra LSB ideal=(VREFP−VREFN)/4096; TI §7.3.1 fija 10 mV/°C. Modelo
usa código×LSB, sin agregar media cuenta ADC; ese error de cuantización no es redondeo
aritmético. No fija bits, pines ni VREF reales ni promete rango eléctrico/variante.

## API, flujo y errores

Temperatura_Lectura: uint32_t centesimas_c y Temperatura_Estado estado.
Inicializar(lectura*) devuelve bool, SIN_DATOS/cero; NULL devuelve false.
Actualizar(lectura*, uint32_t muestra, uint32_t escala_adc, uint32_t referencia_uv)
escribe todo sin leer anterior y devuelve estado. Prioridad: NULL → ERROR_ARGUMENTO;
escala=0 → ERROR_ESCALA; referencia=0 → ERROR_REFERENCIA; muestra>=escala → ERROR_MUESTRA;
resto → VALIDA. Error escribible borra valor; acceso serializado externo.
SIN_DATOS sólo tras inicio/reinicio, sin timeout. Consumir sólo VALIDA; no certifica sensor.

Futura muestra/config coherente → Temperatura → aplicación en °C/validez.
No hay dependencia hacia aplicación/dispositivos ni integración de main. Capturador
valida adquisición, desbordes y parámetros antes de llamar; no detectamos fallos
eléctricos que produzcan códigos matemáticamente admisibles.

## Aritmética y recursos

N=(uint64_t)muestra×referencia_uv; D=(uint64_t)escala_adc×100; q=N/D; r=N%D.
D es positivo y par. Si r>=D/2, incrementar q; estrechar sólo al terminar.
N<=18446744060824649730 para dominio válido; D<=429496729500. Ambos caben en uint64_t;
N+D/2 puede desbordar, por eso no se suma. Como muestra<escala, q redondeado<=42949673,
representable en uint32_t; no se necesita error de rango de salida ni saturación.
Salida de pocos bytes (medir ARM), sin buffers/heap/globals. Reutilizar libgcc existente
y -z noexecstack. Medir código y pila estática, sin atribuir tiempos reales.

## Archivos y documentación

Servicios/Temperatura.h/.c: API, estados, unidades/errores/concurrencia Doxygen; sin
comentarios dentro del cuerpo. tests/host/test_temperatura.c: referencias, redondeo,
errores, fronteras y recorridos. cmake/verificar_enlace_b05.cmake: retener ambas API
y auxiliares en Debug/Release. CMakeLists.txt y docs/doxygen/Doxyfile: agregar controles.
README, docs/agente/ y OpenSpec: estado y evidencia; conservar todo otro módulo,
drivers, contratos aceptados e historial. Sin configuración común nueva.

## Verificación y preguntas

17 casos C: incluye 1000 fronteras, 10000 ternas y 70678 puntos de recorrido.
Oráculo independiente compara intervalos de redondeo alrededor del resultado; usa
(q−1)×D acotado para evitar que q×D desborde incluso con salida correcta.
Controles negativos temporales para suma ingenua y redondeo intermedio, si reproducibles.
Host C11 estricto y sanitizadores; regresiones B00–B04, ARM -O0/-O3 y Doxygen estricto.
No hay TDD configurado: modo estándar. Sin información indispensable pendiente para
el cálculo. Montaje, VREF, variante/rango LM35 y precisión reales se verifican al integrar.

Fuentes: [TI §7.3.1](https://www.ti.com/lit/ds/symlink/lm35.pdf) y
[NXP fig.28, p.68](https://www.nxp.com/docs/en/data-sheet/LPC1769_68_67_66_65_64_63.pdf).
