# Delta B18/I2 — mando-motor (reemplaza el contrato vigente)

Candidato de la vía rápida B18. MM1–MM4 (B08) quedan como historial.

## REMOVED Requirements

### Requirement: MM1–MM4
Motivo: salida reemplazable por puntero a función con contexto, cuatro resultados
y `SolicitarCero` aparte. El servicio pasa a llamar al dispositivo Motor.

## ADDED Requirements

### Requirement: MM5 — Mando limitado
`Servicios/Driver_Motor` MUST exponer `void Driver_Motor_Init(void)` (llama
`Motor_Init`, que arranca con mando 0) y `void Driver_Motor_Aplicar(uint16_t
por_mil)`, que entrega a `Motor_Write` el valor limitado a 1000. Sin estado.

#### Scenario: S1 — Mando limitado a 1000 (`i2.drivers`)
- Given motor simulado.
- When se inicializa y se aplican 0, 1, 500, 999, 1000, 1001, 65535 y 0.
- Then Init MUST NOT escribir mando; cada aplicación MUST producir una escritura
  con 0, 1, 500, 999, 1000, 1000, 1000 y 0, y `Motor_Write` MUST NOT recibir más de 1000.
