# Diseño B09 — Telemetría portable

## Decisiones

Formato/alcance acordados, incluida ampliación de exclusión original B02-R5.
Delta candidato mantiene la recepción; contrato vigente sólo se sincronizará al aceptar resultado.

| Opción | Compromiso | Decisión/motivo |
|---|---|---|
| Tipos de aplicación / instantánea propia | Dependencia inversa y acoplamiento de enums | DTO lógico propio, adaptación explícita sólo en pruebas/fixture |
| snprintf / conversión entera acotada | libc/formato variádico aumenta dependencia ARM | Tres helpers pequeños, sin flotantes/libc nuevos |
| Escribir parcialmente / trama local | Fragmentos truncados pueden parecer telemetría válida | Preparar hasta 54 bytes locales y copiar sólo si entra íntegra |
| Reusar resultado RX / resultado propio | Nuevos códigos podrían alterar consumidores existentes | Enum propio; estructuras/enums/cuerpos B02 preservados |
| Valor por defecto / ND y ERR | Cero válido debe distinguirse de ausencia/fallo | Validez separada por medición, campos numéricos ignorados si no válidos |
| Negar int32_t / magnitud unsigned | -INT32_MIN es UB | Magnitud negativa por 0u-(uint32_t)valor, módulo 2^32 definido |

## API

Comunicacion.h/.c, ESW.2.1.7:
- Comunicacion_ValidezDato: DATO_VALIDO, DATO_SIN_DATOS, DATO_ERROR.
- Comunicacion_EstadoOperacion: OPERACION_MANUAL, OPERACION_AUTO_ESPERA, OPERACION_AUTO_ACTIVO.
- Comunicacion_Telemetria: rpm_milesimas uint32_t, temperatura_centesimas_c int32_t,
  validez_rpm/validez_temperatura y estado_operacion (enums propios).
- Comunicacion_ResultadoTelemetria: TELEMETRIA_OK, TELEMETRIA_ERROR_ARGUMENTO,
  TELEMETRIA_ERROR_ESTADO, TELEMETRIA_ERROR_CAPACIDAD.
- Comunicacion_CodificarTelemetria(const Comunicacion_Telemetria*, char*, size_t,
  size_t*), USW.2.1.7.7. Buffer del llamador; no retiene punteros ni necesita contexto RX/init.
- COMUNICACION_TELEMETRIA_CAPACIDAD=54 incluye NUL. Máximo: RPM= + 11 + ;TEMP=
  + 12 + ;ESTADO= + 11 + LF = 53. Sin truncado, NUL no es byte de transporte.

Primero limpiar longitud si existe y buffer[0] si escribible; validar punteros,
validez/operación y luego capacidad. Estados válidos de ambas mediciones se comprueban
antes de usar números. Capacidad cero no autoriza tocar buffer. No inferir tamaño
real de memoria por un puntero; capacidad debe corresponder al espacio escribible.
Datos, buffer y longitud no solapados; memoria viva/coherente/serializada, const no da atomicidad.

Helpers privados (USW.2.1.7.8/.9/.10): AgregarTexto, AgregarEntero y AgregarFijo.
Texto sólo usa literales internos; entero usa 10 dígitos invertidos (uint32_t).
Fijo recibe escala interna 100/1000, parte entera y fracción con 2/3 dígitos.
Longitudes acumuladas están acotadas por 53, sin sumar capacidad arbitraria;
copiar NUL sólo si capacidad>longitud. Bucles propios acotados; sin esperas/estado mutable global.
Preservar cero/negativos/subunidad y todo dominio uint32_t/int32_t, sin recorte 185.

## Composición y archivos

La aplicación futura reunirá magnitudes B03/B05 y estado B01 y llamará al servicio;
este bloque demuestra composición sólo en pruebas. Sin cambios a Procesamiento/main
o dispositivos. Estado operativo es MANUAL/AUTO_ESPERA/AUTO_ACTIVO; no añade aviso
térmico opcional ni confirma estado de actuadores. Adaptación mapea SIN_DATOS a ND,
errores de lectura a ERR y sólo usa magnitud válida. Recepción RX queda independiente.

| Archivo | Cambio |
|---|---|
| Servicios/Comunicacion.h/.c | Agregar API/helpers y cuatro USW documentadas; conservar B02 |
| tests/host/test_telemetria.c | Vectores exactos, estados, errores/capacidad/canarios, independencia y oráculo snprintf sólo host |
| tests/host/test_integracion_telemetria.c | B01/B03/B05, transiciones y recepción parcial/FIFO intercalada |
| CMakeLists.txt, cmake/verificar_enlace_b09.cmake | Dos targets/CTest y fixture ARM con API retenida, ELF/indefinidos/GNU_STACK |
| docs/doxygen/README.md | Doxyfile ya incluye Comunicacion; cuatro funciones nuevas, sin relajar controles |
| docs/agente/, README, openspec/ | Estado/matriz/evidencia y candidato; archivos aceptados inmutables |

## Verificación y riesgos

Consumidores primero RED por API ausente, luego mismos archivos GREEN. Oráculo host
usa snprintf/enteros amplios, algoritmo distinto a los dígitos manuales de producción.
Probar todas las fracciones RPM y temperaturas ±centésimas, fronteras y casos aleatorios
deterministas. Negativos temporales: eliminar padding decimal y error de capacidad NUL.
Host/sanitizadores (sin ptrace para LeakSanitizer), regresiones, ARM Debug completo,
fixture B09 Release y Doxygen estricto; medir código/pila/datos y ABI actual sin fijarlo.
No hay dudas bloqueantes ni migración/instalación. UART/envío/periodicidad, main,
hardware y diagramas finales siguen pendientes. Commit local autorizado, sin push.
