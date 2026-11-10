# Base de tiempos — candidato B11

Origen R01/R02/R08/R09/R10/R11/R15; alcance acordado el 2026-11-09.
Contrato nuevo de soporte, todavía sin aceptación del resultado.

## Requirements

### Requirement: BT1 — Configuración explícita

MUST ofrecer inicializar/reiniciar con frecuencia CPU uint32_t Hz explícita, estable
y real; MUST admitir múltiplos de 1000 entre 2000 y 120000000 inclusive para ticks
de 1 ms. MUST rechazar el resto con ERROR_FRECUENCIA, cero ms y contador deshabilitado,
incluso tras uso válido. MUST NOT configurar PLL/relojes/flash ni asumir calibración.

#### Scenario: S1 — Frecuencias y recarga
- Given frecuencias válidas, fronteras o cero/no múltiplos/excesivas.
- When se inicializa la base.
- Then MUST producir ACTIVA/cero con período exacto representable, o ERROR_FRECUENCIA sin habilitar.

#### Scenario: S2 — Reinicialización y recuperación
- Given base activa, detenida o fallida con tiempo/eventos previos.
- When se inicializa con frecuencia válida o inválida.
- Then MUST abrir época nueva desde cero o invalidar; un reinicio válido MUST recuperar.

### Requirement: BT2 — Lectura y ciclo de vida

Antes de inicializar MUST devolver SIN_DATOS/cero. Leer MUST entregar instantánea
coherente de estado y uint32_t milisegundos; ACTIVA habilita uso temporal, incluido
cero. NULL MUST retornar ERROR_ARGUMENTO sin alterar base ni acceder a registros.
Detener MUST congelar tiempo y convertir ACTIVA en DETENIDA; repetición MUST ser
idempotente y conservar SIN_DATOS/ERROR_FRECUENCIA. Sólo reinicializar reanuda desde cero.

#### Scenario: S3 — Inicio y argumentos
- Given base sin inicializar o lectura NULL en cualquier estado.
- When se lee o se detiene antes de inicializar.
- Then MUST distinguir ausencia/cero activo, conservar diagnósticos y no mutar por NULL.

#### Scenario: S4 — Parada
- Given ticks atendidos y posibles pendientes.
- When se detiene y llegan más eventos.
- Then MUST conservar tiempo detenido sin nuevos incrementos hasta reinicio válido.

### Requirement: BT3 — Interrupciones y concurrencia

MUST contar una unidad por tick atendido estando ACTIVA, con rollover uint32_t;
ISR MUST ser breve, sin callbacks/esperas/trabajo de servicios. Inicializar/detener
MUST cancelar eventos pendientes anteriores. Operaciones públicas MUST preservar
PRIMASK previo y proteger estado/contador frente a ISR. Sólo lazo privilegiado,
serializado; propietario exclusivo de SysTick. MUST NOT prometer recuperar ticks
colapsados, tiempo en suspensión/debug ni precisión eléctrica sin evidencia física.

#### Scenario: S5 — Ticks y rollover
- Given contador activo cerca del límite uint32_t o base no activa.
- When se atienden interrupciones.
- Then MUST incrementar modularmente sólo ACTIVA y mantener diagnóstico en los demás estados.

#### Scenario: S6 — Máscara y eventos pendientes
- Given interrupciones habilitadas/enmascaradas y SysTick pendiente antiguo.
- When se lee/inicializa/detiene y se restauran interrupciones.
- Then MUST preservar máscara, obtener instantánea coherente y evitar ticks antiguos tras reinicio/parada.

### Requirement: BT4 — Adaptación y evidencia

MUST aislar registros/CMSIS dentro de Dispositivos/soporte y conservar contratos
aceptados, servicios, aplicación y main. Sólo pruebas MAY componer tiempo con B10;
MUST probar frontera exacta. MUST documentar USW y ejecutar host/sanitizadores,
regresiones y enlace ARM Debug/Release con vector/recursos revisados. Modelo de
registros MUST distinguirse de simulación MCU/placa; aceptación física pendiente.

#### Scenario: S7 — Consumo B10
- Given base activa y selector B10 con intervalo configurable.
- When llegan muestras con tiempos obtenidos antes/en la frontera o base no activa.
- Then MUST confirmar exactamente al cumplir intervalo; adaptador MUST consumir sólo ACTIVA.

#### Scenario: S8 — Puerto y verificación
- Given implementación/puerto, registros modelados y fixtures ARM.
- When se ejecutan pruebas, inspección de registros/vector/recursos y Doxygen.
- Then MUST preservar máscaras/orden/registros ajenos y registrar resultados reales por modalidad.
