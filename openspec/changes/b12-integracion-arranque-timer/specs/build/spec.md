# Build — delta candidato B12

## MODIFIED Requirements

### Requirement B00-R3: Arranque revisable
Startup MUST copiar .data, borrar .bss y luego actualizar la variable RAM de
frecuencia mediante la función CMSIS existente antes de main. MUST preservar
handlers por defecto. MUST NOT invocar SystemInit ni configurar PLL/reloj/flash
timing/protección desde esa actualización. Main MAY iniciar TIMER0 mediante la
API B11 en el alcance B12. MUST NOT flashear dispositivos sin autorización específica.

#### Scenario S6: Ruta de arranque
- Given startup y fixture con .data/.bss no vacías.
- When se enlazan e inspeccionan orden y desensamblado.
- Then MUST verificar memoria→actualización de información→main→Init→Start,
  conservando límites/vectores/checksum/CRP y distinguiendo revisión de ejecución física.
