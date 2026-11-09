# Diseño B06 — Decisión térmica en aplicación

## Fuentes y decisiones

R07 pide temperatura>umbral y alarma visual/acústica; B06 sólo evalúa la condición.
Usuario acordó alcance y umbral como parámetro, sin valor fijo. B05 ya entrega Celsius
y estado genéricos. No hay discrepancias nuevas; los contratos B01/B05 se preservan.

| Opción | Compromiso | Decisión y motivo |
|---|---|---|
| Decidir en servicio / aplicación | Driver_Alarmas recibe solicitudes de Procesamiento | Decisión en Procesamiento; servicio/actuadores se integrarán después |
| Copiar enum de fuente / usar contrato B05 | Enum duplicado exige adaptación sin necesidad | Incluir Temperatura.h desde aplicación: dependencia descendente permitida, sin dispositivo |
| Salida bool / estado completo | bool confunde ausencia/error con temperatura normal | Enum de cinco decisiones: SIN_DATOS, INACTIVA, ACTIVA, ERROR_FUENTE, ERROR_ARGUMENTO |
| Contexto propio / función sin memoria | No hay histéresis, retención o temporización acordadas | Retorno por valor, sin inicialización ni estructura de salida innecesarias |
| Umbral global/constante / argumento | No se eligió valor y debe ser configuración | int32_t umbral_centesimas_c explícito; ningún default o rango físico inventado |

## Interfaz y comportamiento

Añadir Procesamiento_EstadoAlarma y
Procesamiento_EvaluarAlarmaTemperatura(const Temperatura_Lectura *lectura,
int32_t umbral_centesimas_c), USW.2.1.2.7. IDs .1–.6 y funciones B01 conservados.
El retorno es toda la decisión: el llamador lo asigna, sin resultado numérico adicional.
No pasar contexto B01, modo, salida de motor o dispositivos a esta función.

NULL→ERROR_ARGUMENTO sin leer memoria. Switch del estado antes de leer Celsius:
VALIDA compara con >, SIN_DATOS/ERROR_FUENTE/ERROR_ARGUMENTO preservan su categoría;
enum desconocido→ERROR_ARGUMENTO. Comparar int32_t directamente, sin T−umbral,
evita overflow en extremos. No escribe la entrada const ni consulta datos anteriores.
El integrador sólo interpreta ACTIVA/INACTIVA como evaluación disponible; otros
estados no significan orden de apagar/encender LED o motor. Fuente nueva recupera
porque la siguiente llamada no depende de la anterior. const exige aún captura
coherente, vida útil de lectura y serialización externa, sin garantías de frescura.

Servicio Temperatura → lectura física/validez → Procesamiento + configuración → decisión.
No convertir ADC ni conocer LM35. Driver_Alarmas/LED/Buzzer y main permanecen intactos.
No implementar patrón físico de fallo, prioridad temporal, parada, umbral por defecto,
histéresis, timeout o enclavamiento; quedan para bloques acordados posteriores.

## Archivos y compatibilidad

- Aplicacion/Procesamiento.h/.c: añadir enum/función y actualizar descripción del módulo.
  Mantener cuerpos, tipos y firmas B01 byte por byte salvo adiciones nuevas.
- tests/host/test_alarma_temperatura.c: 13 casos de frontera, errores, recuperación,
  inmutabilidad, integración B05 y tres estados de gestión de modos.
- CMakeLists.txt: nuevo target/CTest host y fixture B06 ARM.
- cmake/verificar_enlace_b06.cmake: retener nueva API + B01/Temperatura sin libc,
  parámetros volátiles, ELF y GNU_STACK RW; ejecutar sólo compilación/enlace.
- cmake/verificar_enlace_b01.cmake: agregar -IServicios por dependencia pública
  de tipos hacia Temperatura.h; no cambiar lógica/controles existentes.
- Doxyfile existente ya cubre Procesamiento y Temperatura: no requiere expansión.
  Doxygen estricto debe validar nueva USW y mantener todas las anteriores.
- docs/agente, README, docs/doxygen/README y seguimiento activo: evidencia actual.
  No modificar contratos principales ni archivos aceptados antes de aceptación B06.

## Verificación y recursos

Escribir tests antes de API, comprobar fallo por símbolos ausentes y luego ejecutar
exactamente el mismo consumidor. Regresiones B01–B05/B03-R1; host/sanitizadores;
integración B05 sin dispositivo; ARM Debug y B06 Release; documentación HTML/XML.
Controles negativos temporales comprueban detección de >= y ausencia tomada como normal.
Inspeccionar código/pila de nueva función y tamaño de enum ARM; sin suponer latencia
o ejecución física. Nuevos cálculos sólo comparan int32_t, sin auxiliares aritméticos.
No hay preguntas bloqueantes ni dependencias instalables nuevas.
