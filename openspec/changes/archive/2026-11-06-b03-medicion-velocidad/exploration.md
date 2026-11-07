# Exploración B03

Medicion_Velocidad.h/.c siguen como esqueleto; el requisito principal R02 aún no tiene
cálculo implementado. B01 decide modos y B02 entrega consignas válidas con antigüedad.
El siguiente dato necesario para regulación/visualización es una velocidad con validez.

Se consideró implementar captura/timer reales, pero todavía faltan asignación de pines,
revisión física y estrategia temporal. Un bloque de cálculo con entradas de ventana
permite ejecutar casos numéricos verificables sin inventar esos datos ni precisión.

Recomendación: servicio independiente de registros, con contrato explícito de pulsos,
duración y errores. El dispositivo futuro entregará instantáneas coherentes. Los
60 pulsos/vuelta propuestos requieren un solo flanco por ranura; no contar ambos.
RPM corresponde al eje portadisco, sin asumir una reducción mecánica no informada.

No usar 185 como clamp de lectura: es el máximo de consignas aceptado en R14.
Elegir representación y redondeo antes de código; contemplar auxiliares de división
provistos por el toolchain si hacen falta. No hay contrato vigente, código ni pruebas
B03 ejecutadas; la propuesta no afirma rendimiento o medición física.
