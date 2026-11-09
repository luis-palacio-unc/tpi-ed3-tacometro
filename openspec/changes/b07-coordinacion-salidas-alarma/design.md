# Diseño B07 — Driver de alarma portable

## Decisiones

B06 decide temperatura>umbral. B07 sólo envía una solicitud lógica al par LED/buzzer.
R15 exige independencia de hardware también en servicios; las salidas reales aún
no tienen pines/polaridades confirmados. La interfaz reemplazable resuelve esa necesidad
concreta sin un registro global o framework. No hay discrepancias con alcance acordado.

| Opción | Compromiso | Decisión/motivo |
|---|---|---|
| Llamar LED/Buzzer concretos / funciones externas | Acoplar servicios impide trasladar implementación | Dos funciones bool(void*,bool) inyectadas por llamada |
| Contexto compartido / por salida | Los dispositivos pueden tener configuraciones distintas | Dos contextos opacos, uno por función; NULL válido si proveedor lo admite |
| Inicialización/registro global / interfaz const explícita | Agrega ciclo de vida sin requisito | Sin estado del servicio, init, heap o caché de órdenes |
| Cortocircuitar errores / intentar ambos | Primer fallo no debe silenciar otra señal | LED primero, buzzer después siempre; resultado combinado |
| Enum B06 en servicio / solicitud bool | Incluir aplicación invierte dependencias | Servicio recibe bool; adaptación B06 sólo en test/fixture por ahora |

## API y contratos

Servicios/Driver_Alarmas.h/.c, ESW.2.1.6:
Driver_Alarmas_EscribirSalida = bool (*)(void *contexto, bool activar).
Driver_Alarmas_Salidas contiene escribir_led/contexto_led y escribir_buzzer/contexto_buzzer.
Driver_Alarmas_Aplicar(const Driver_Alarmas_Salidas*, bool activar), USW.2.1.6.1,
retorna Driver_Alarmas_Estado: ACEPTADA, ERROR_ARGUMENTO, ERROR_LED, ERROR_BUZZER,
ERROR_AMBOS. No existe salida numérica ni registro interno adicional.

Validar puntero/interfaz completa antes de cualquier llamada; no rechazar contextos
NULL sin conocer su contrato. Ejecutar ambas funciones en instrucciones separadas,
sin &&/|| que cortocircuiten su ejecución. Guardar dos bool y clasificar cuatro
combinaciones; no reintentar, deshacer ni asumir que fallo implica estado físico conocido.
El servicio no cambia interfaz ni consulta registro/contexto anterior; los proveedores
pueden modificar su contexto para aplicar/registrar órdenes. Exigir vida útil, coherencia
y serialización externa, interfaz estable durante callbacks y funciones que retornen.
Duración/pila total dependen de proveedores; no garantizar tiempo de GPIO mediante host.

## Flujo e integración

Temperatura genérica → B06 en aplicación → activar/desactivar → Driver_Alarmas →
funciones de salida → futuros LED/Buzzer. Sólo ACTIVA/INACTIVA llegan al servicio.
Ausencia/error se conserva como diagnóstico; el adaptador de prueba no emite órdenes
ni interpreta un resultado antiguo como aplicación actual. No se añade adaptador a
producción ni main; no se resuelve aquí política física ante pérdida de adquisición.
Proveedor traduce activo lógico a polaridad; servicio no conoce niveles/registros.
Sin cambios a Procesamiento/Temperatura ni dispositivos concretos.

## Archivos

- Driver_Alarmas.h/.c: tipos, única USW y documentación completa de callbacks/errores.
- tests/host/test_driver_alarmas.c: 12 casos del servicio, sólo -IServicios.
- tests/host/test_integracion_alarmas.c: cuatro secuencias B05/B06 y salidas emuladas.
- CMakeLists.txt: dos targets/CTest host y fixture B07 ARM.
- cmake/verificar_enlace_b07.cmake: API B07+B06/B05 retenida; callbacks emulados,
  ELF/GNU_STACK y ausencia de indefinidos, Debug/Release sin ejecutar MCU.
- Doxyfile: ampliar INPUT con Driver_Alarmas; una USW nueva, sin documentar proveedores
  emulados como si fueran unidades de dispositivos de producción.
- docs/agente/README y seguimiento SDD: matriz y evidencia, sin promover/archivar aún.

## Verificación y recursos

Escribir consumidores C antes de implementar API; fallo esperado por símbolos ausentes,
luego mismos archivos pasan. Pruebas de solicitudes, repetición, orden, interfaz nula,
cuatro resultados, recuperación, inmutabilidad y contextos independientes.
Integración prueba B06 evaluable, no evaluable, fallo de salida y recuperación;
última solicitud activa no se reemplaza por apagado ante lectura inválida.
Dos controles negativos temporales: buzzer siempre activo y salida temprana tras fallo LED.
Ejecutar host/sanitizadores y regresiones, ARM Debug/Release y Doxygen estricto.
Medir código/pila propios y sizeof de interfaz/enum con sonda ARM, sin fijar ABI
portable ni atribuir la pila de callbacks al servicio. Sin dudas bloqueantes/dependencias nuevas.
