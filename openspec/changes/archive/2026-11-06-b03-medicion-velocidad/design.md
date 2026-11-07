# Diseño B03 — Medicion_Velocidad

## Enfoque y decisiones

R02 requiere cálculo para disco de 60 ranuras; R04/R09 consumirán la lectura.
R14 sólo restringe consignas. La captura futura suministra pulsos/tiempo coherentes.

| Opción | Compromiso | Decisión y motivo |
|---|---|---|
| RPM entero / float / milésimas enteras | Entero pierde fracciones; float añade redondeos/runtime | uint32_t milésimas: resolución explícita de 0,001 RPM, sin flotantes |
| Truncar / redondear | Truncado sesga hacia abajo | Cercano, empate arriba; error <=0,0005 RPM |
| Saturar / error de rango | Saturar oculta entradas imposibles | Error y cero; aceptación sólo si resultado redondeado cabe en uint32_t |
| Contexto+getter / lectura explícita | Getter duplica una copia sin requisito | Registro de salida por instancia; dos funciones, sin estado oculto |
| Conservar lectura vieja / invalidar | Lectura vieja puede parecer actual | Cada actualización reemplaza valor/estado; error borra RPM |
| División de 64 bits / reducción de tipos | Reducir antes pierde precisión o rango | uint64_t y libgcc existente; sin libc ni dependencias nuevas |

## Contrato y aritmética

Medicion_Velocidad_Lectura contiene rpm_milesimas (uint32_t) y estado (enum).
Inicializar(lectura*) devuelve bool y fija SIN_DATOS/cero; NULL devuelve false.
Actualizar(lectura*, uint32_t pulsos, uint32_t duracion_us) devuelve estado,
escribe todos los campos y no necesita leer valores anteriores. NULL devuelve
ERROR_ARGUMENTO; duración cero devuelve ERROR_DURACION; exceso, ERROR_RANGO.
Salida legible sólo tras inicializar o actualizar. No copiar lecturas concurrentemente.

Con 60 pulsos/vuelta: RPM = pulsos × 60 000 000 / (60 × us).
Milésimas = pulsos × 1 000 000 000 / us. Calcular en uint64_t:
(numerador + us/2) / us; validar antes de estrechar a uint32_t.
Para todos los uint32_t, numerador <=4 294 967 295 000 000 000 y suma
<=4 294 967 297 147 483 647 <UINT64_MAX. La división sólo ocurre con us>0.
Las duraciones representan 1..4 294 967 295 us; no hay contador ni rollover aquí.
Una medición pequeña puede redondear a cero válido. SIN_DATOS sólo representa
ausencia tras inicialización/reinicio, no un timeout; la frescura requiere diseño futuro.

## Flujo, concurrencia y recursos

Futura captura → instantánea pulsos/us → Actualizar → lectura para aplicación.
No se llama Sensor_Optico ni se integra main. El capturador debe comprobar overflow,
un flanco por ranura y eje portadisco; no hay ratio de reductora asumido.
Serialización externa de captura y lectura/actualización, sin ISR en B03.
Dos campos por lectura (tamaño ARM a comprobar), sin buffer, heap ni global mutable.
División uint64_t puede emitir __aeabi_uldivmod; enlazar gcc después de objetos
en firmware y fixture. El libgcc instalado carece de nota GNU-stack en _dvmd_tls.o:
declarar -z noexecstack y comprobar GNU_STACK RW en fixture, manteniendo
--fatal-warnings. Es metadata ELF, no configuración del MPU ni garantía física.
Medir código y pila estática, incluido auxiliar si visible;
no atribuir esos tamaños a una ejecución física.

## Archivos y documentación

| Archivo | Cambio |
|---|---|
| Servicios/Medicion_Velocidad.h/.c | API, tipos/estados, dos USW.2.1.3.1/.2 y ESW.2.1.3 |
| tests/host/test_medicion_velocidad.c | Referencias, errores, redondeo, extremos y barrido aritmético |
| cmake/verificar_enlace_b03.cmake | API retenida en ELF ARM Debug/Release, auxiliares resueltos |
| CMakeLists.txt | Test host/ARM, sanitizadores y runtime gcc existente |
| docs/doxygen/Doxyfile | Agregar .h/.c de servicio a generación estricta |
| README, docs/agente/, openspec/ | Uso, trazabilidad, estado y evidencia |

Doxygen documenta unidades, errores, precondiciones y concurrencia. Sin comentarios
dentro del cuerpo de las funciones de producción, según guía. No tocar otros
módulos, drivers, contratos vigentes ni archivos archivados.

## Verificación y límites

Casos nominales y fronteras independientes de la fórmula del código. Barrido
determinista verifica desigualdad de error usando enteros, sin repetir el algoritmo.
Host con advertencias estrictas; ASan/LeakSanitizer/UBSan fuera de ptrace.
Regresiones B00–B02, fixture API B03 a -O0 y -O3; ELF sin símbolos pendientes.
Doxygen HTML/XML sin avisos; verificar ambas USW. Pruebas físicas, reloj,
adquisición, selección de ventana y precisión eléctrica pendientes y fuera de B03.
Sin preguntas indispensables para el cálculo lógico ni instalación requerida.

Referencias: [GCC: -nostdlib/-lgcc](https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html)
y [GNU ld: noexecstack](https://sourceware.org/binutils/docs/ld/Options.html).
