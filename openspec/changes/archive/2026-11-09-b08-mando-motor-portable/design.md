# Diseño B08 — Driver_Motor portable

## Decisiones y contratos

Alcance acordado; sin discrepancia entre requisitos y contratos B01/B04.
El comentario del esqueleto «limita» se concreta como validación/rechazo del rango
acordado, no saturación. R15 exige que la salida sea reemplazable sin MCU concreto.

| Opción | Compromiso | Decisión/motivo |
|---|---|---|
| PWM concreto / función externa | Pines/frecuencia sin confirmar y portabilidad requerida | Función bool(void*,uint16_t) por llamada; mismo patrón B07 |
| uint16_t / uint32_t para mando | B04/B01 ya usan uint16_t; 1000 cabe con margen | uint16_t y rechazo 1001..UINT16_MAX; el llamador valida valores más amplios antes de convertir |
| Saturar / rechazar | Saturación oculta entradas erróneas | ERROR_RANGO sin orden ni parada implícita |
| Registro global/init / interfaz const | No hay estado propio que inicializar | Función y contexto externos explícitos, sin heap/cache |
| Detener / solicitar cero | Inercia/etapa de potencia impiden confirmar detención | Nombre SolicitarCero, misma validación y reporte que Aplicar |
| RPM como PWM / regulación posterior | Son magnitudes distintas | AUTOMATICA no emite desde adaptador de prueba; regulador pendiente |

Servicios/Driver_Motor.h/.c, ESW.2.1.8:
- Driver_Motor_EscribirSalida: bool (*)(void *contexto, uint16_t mando_por_mil).
- Driver_Motor_Salida: escribir y contexto; el servicio no copia/retiene/modifica interfaz.
- DRIVER_MOTOR_MANDO_MAX alias de TPI_MANDO_MANUAL_MAX en Limites.h (dominio, no hardware).
- Driver_Motor_Estado: ACEPTADA, ERROR_ARGUMENTO, ERROR_RANGO, ERROR_SALIDA.
- Driver_Motor_Aplicar(const Driver_Motor_Salida*, uint16_t), USW.2.1.8.1.
- Driver_Motor_SolicitarCero(const Driver_Motor_Salida*), USW.2.1.8.2, delega Aplicar con 0.

Validar puntero/función, luego rango y finalmente invocar proveedor una vez.
Traducir true/false al enum de esta llamada. No tocar el contexto opaco; el proveedor
sí puede modificarlo. NULL como contexto depende de su contrato. No deshacer, reintentar
o convertir un fallo en apagado. ACEPTADA sólo acredita aceptación del proveedor.
El enum no se guarda internamente: aplicación debe consumir el retorno.
Interfaz/contextos legibles/vivos/estables; serialización externa también ante ISR.
const no valida punteros ni garantiza atomicidad. Función sin esperas activas;
tiempo/pila total y efecto físico dependen del proveedor, no se fijan timeouts aquí.

## Composición y archivos

Mandos B04 → Procesamiento B01 → mando lógico/CERO → Driver_Motor → función externa.
Esta composición se implementa sólo en pruebas/fixture; main y dispositivos intactos.
MANUAL consume mando B04 sólo con VALIDO; CERO solicita cero aunque procesamiento
indique error; AUTOMATICA no produce orden hasta incorporar regulador en otro bloque.
No se propone política nueva de parada por ausencia/error de adquisición.

| Archivo | Cambio |
|---|---|
| Servicios/Driver_Motor.h/.c | API anterior, dos USW y Doxygen completo |
| tests/host/test_driver_motor.c | Rango uint16_t completo, repeticiones/cero, errores, contexto e interfaces |
| tests/host/test_integracion_motor.c | Manual B04/B01, transición/espera, consigna automática, validez, fallo observable |
| CMakeLists.txt | Dos targets/CTest; servicio aislado con Servicios y Configuracion únicamente |
| cmake/verificar_enlace_b08.cmake | Fixture generado B08+B04/B01, símbolos retenidos, ELF/indefinidos/GNU_STACK |
| docs/doxygen/Doxyfile y README | Incorporar módulo/USW sin relajar avisos estrictos |
| docs/agente/, README, openspec/ | Contexto/estado/matriz y evidencia; contratos vigentes/archivos inmutables |

## Verificación, recursos y despliegue

Consumidores host primero: RED por API ausente; mismos archivos GREEN después de
implementar servicio. Dos controles negativos temporales (saturar inválidos y cero
equivocado) deben fallar; no se guardan mutantes en Git.
Pruebas host/sanitizadores y regresiones completas una vez; ARM Debug completo y
fixture B08 Release, sin ejecución. Doxygen HTML/XML estricto y comprobar IDs de ambas USW.
Medir .text/.data/.bss, pila propia y tamaño de interfaz/enum con toolchain ARM actual,
sin fijar ABI portátil ni atribuir al servicio recursos de callbacks o integración.
No se necesita migración, instalación ni pregunta bloqueante. Pines/PWM/potencia,
arranque y tiempos físicos siguen pendientes. Sin push autorizado; contrato candidato
no se promueve ni archiva hasta aceptación del resultado.
