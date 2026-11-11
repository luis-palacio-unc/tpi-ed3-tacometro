# Archivo B15 — 2026-11-10

El usuario acepta el resultado, pide cerrar y planificar B16. Alcance aceptado:
Motor_Write de una entrada y mando directo con MR0 fijo1000. Implementación
b473d0f,10/10 tareas completas; no se aceptan aquí inicialización o ensayo físico.
No se autoriza push en este cierre; commit local según procedimiento acordado.

## Sincronización

Creado openspec/specs/motor-dispositivo/spec.md: MD1–MD4 y S1–S5 idénticos al
candidato, sólo título/procedencia reflejan aceptación. Estado actualizado antes
de mover el cambio. Propuesta/spec/diseño/tareas/informe conservan redacción
histórica, incluidas menciones a aceptación pendiente; este informe y state.yaml
registran el cierre posterior. Otros contratos y archivos archivados intactos.

## Evidencia y límites aceptados

Host/sanitizadores21/21,204 casos C+oráculo Python; cuatro casos B15 con1001 mandos
válidos y64535 rechazos. ARM13/13 y objetosO0/O2/header; Doxygen42 sin avisos.
Único externo PWM_MatchUpdateSingle; Drivers90 originales a21cc1e intactos.
Evidencia realmente ejecutada en verify-report.md y /tmp/ed3-b15-directo-2yz3lskk.
No se repiten ni declaran pruebas nuevas de firmware en el cierre documental.

MR0=1000 es configuración requerida para inicialización final, no aplicada aún.
PWM1.1/P2.0 conservado; frecuencia/prescaler/pinmux/potencia/adaptación B08/main y
validación física PWM.1 siguen pendientes. No ejecución ARM/placa, simulación,
instalación, flasheo o publicación. Las versiones anteriores con conversión,
lectura MR0 o1000→1001 quedan sólo como historial Git, no contrato actual.

Exploración/propuesta B16 no acredita adquisición ADC/DMA ni potenciómetro real.
Próxima acción: acordar B16 antes de implementar. Cierre revisa igualdad normativa,
YAML, enlaces, diff y hashes de fuentes/biblioteca/archivos previos.
Logs del cierre: /tmp/ed3-cierre-b15-propuesta-b16-zasor5qr.

## Manifest SHA256 (sin este informe)

- `design.md`: `7c1b7d737f96d467a0a11b80495ae830ac6c8260428470e8ed019da34f233f72`
- `exploration.md`: `6473800c1acefbd499a76d599fd81b6f1e8e72f5086fd5b11fea484e9c336642`
- `proposal.md`: `a549934d088bd0b099f3dea9daa4ef8e6ad9335eea3ddc2f3212460e4720d872`
- `specs/motor-dispositivo/spec.md`: `2346a919b4517111dcf85264e2fc850d2dd70c285d3004366beb8323cc64eab6`
- `state.yaml`: `17219b5dabb8c74afa83193ae3dfd19c459082d2eccc5253bff2648eb77d3a63`
- `tasks.md`: `3a683397339a1f163539465421dd168b1cc2b18e64ecf25b5cecbe06bca036c0`
- `verify-report.md`: `665f92f4475a258dc59d6e1c6dc05e11490aa2f14cc62619352b8e8902a16616`
