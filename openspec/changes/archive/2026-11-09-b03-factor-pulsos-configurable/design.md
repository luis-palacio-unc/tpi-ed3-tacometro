# Diseño B03-R1 — Pulsos por vuelta explícitos

## Decisión de arquitectura

El contrato aceptado B03 fija 60 en servicio. R15 aclarado por el usuario exige
independencia y su orden actual autoriza corregirlo. Se mantiene cálculo genérico
de conteo/tiempo: el factor explícito expresa pulsos por vuelta, sin identificar
modelo de sensor, placa, registros o flancos particulares. La composición externa
relacionará captura y factor coherentes; aplicación seguirá consumiendo RPM/estado.

| Opción | Compromiso | Decisión |
|---|---|---|
| Constante fija / parámetro | Fija requiere recompilar servicio y liga disco | Cuarto argumento uint32_t pulsos_por_vuelta por llamada |
| Configuración mutable global / por valor | Global añade ciclo de vida y sincronización | Por valor, igual que duración; instancias independientes |
| Fórmula específica en sensor / cálculo genérico | No hay captura real; PPR es relación genérica de conteos | Servicio recibe parámetros abstractos; Configuracion/Medicion.h fija 60 del TPI |
| Wrapper compatible / API explícita | Wrapper oculta otra vez el 60 | Migración de todos los llamadores; sin alias físico en servicio |
| uint128 / límites nuevos / cálculo acotado64 | uint128 no portable Cortex-M3; recortar rompe dominio | Cociente/resto y seis pasos binarios exactos con uint64_t |

## API, errores y flujo

Medicion_Velocidad_Actualizar(lectura,pulsos,duracion_us,pulsos_por_vuelta).
Añadir ERROR_PULSOS_POR_VUELTA al final del enum para conservar valores existentes.
NULL tiene prioridad, luego duración cero, factor cero y rango del resultado.
Salida escribible se limpia en error; fuente válida recupera sin inicialización previa.
Inicializar conserva contrato. Lectura de dos campos, sin timestamp ni estado nuevo.

Captura + configuración → servicio de cálculo → lectura RPM/estado → aplicación.
Servicio no incluye Medicion.h ni dispositivos. Configuración define
TPI_MEDICION_PULSOS_POR_VUELTA=60U (un flanco/ranura de anteproyecto); sólo consumidores
del TPI la incluyen. Fixture y regresiones usan esta constante. No integrar main real.
Contrato de acceso serializado permanece; factor no valida overflow ocurrido al capturar.

## Aritmética acotada

Resultado ideal = pulsos×60000000000/D, D=duracion_us×factor.
SEGUNDOS_POR_MINUTO=60 es conversión universal de tiempo; no son pulsos/vuelta.
Numerador directo puede exceder uint64_t; D<=18446744065119617025 sí cabe.
Usar A=pulsos×1000000000<=4294967295000000000 y dividir: q=A/D, r=A%D.
Si q>UINT32_MAX/60, resultado no representable. Obtener cociente/resto de r×60/D
procesando seis bits de 60, sin construir el producto:
doblar resto mediante comparación con D-resto; sumar r mediante comparación con D-r.
Cada resta/suma queda en 0..D-1 y cociente fraccional en 0..59.
Resultado=60q+fracción; redondear una vez si resto>=D-resto (también seguro con D impar).
Comprobar UINT32_MAX después de redondear. Sin saturación ni doble redondeo.
Son seis iteraciones acotadas, no espera; no afirmar latencia física. libgcc existente
resuelve división/modulo64; no introducir heap, libc o dependencia nueva en firmware.

## Archivos y migración

- Servicios/Medicion_Velocidad.h/.c: API, aritmética y dos USW existentes con Doxygen.
- Configuracion/Medicion.h: factor físico TPI documentado, sin funciones nuevas.
- tests/host/test_medicion_velocidad.c: 15 casos anteriores con factor60 y ocho nuevos.
- tests/host/consulta_medicion_velocidad.c y verificar_velocidad.py: consumidor C aislado,
  entradas por stdin; Python estándar usa enteros arbitrarios como oráculo independiente.
- CMakeLists.txt: target/CTest host adicional y Python ya disponible; fixture B03 recibe factor.
- Doxyfile/contexto/OpenSpec: trazabilidad nueva. No modificar specs/archives aceptados.

## Verificación y preguntas

Antes del cambio, consumidor de cuatro argumentos debe fallar al compilar.
Después compila sin configuración/dispositivo. Regresiones TPI y oráculo cubren
factores, ceros, errores combinados, extremos, límites de redondeo/rango y monotonía.
Host/sanitizadores, ARM Debug/Release, recursos/pila y Doxygen estrictos.
No hay preguntas bloqueantes: 60 del TPI confirmado por consigna; hardware físico
sigue pendiente fuera de este alcance. Resultado requiere aceptación; sin push nuevo.
