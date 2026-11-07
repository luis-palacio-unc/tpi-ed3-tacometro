# Build LPC1769 — contrato vigente

Origen: R01 (LPC1769/OM13085) y R13 (verificación SDD). Alcance autorizado el
2026-11-06. Resultado aceptado por el usuario el 2026-11-06; origen B00 archivado.

## Requirements

### Requirement B00-R1: Compilación explícita
El build ARM MUST producir ELF ejecutable Cortex-M3/Thumb, mapa y reporte de memoria
con C11 y encabezados CMSIS existentes, sin dependencias nuevas. MUST NOT sustituir
silenciosamente un toolchain ausente por el compilador host.

#### Scenario S1: ARM limpio
- GIVEN fuentes y toolchain ARM instalado
- WHEN se configura y compila en un directorio vacío
- THEN se genera ELF ARM con main, Reset_Handler, mapa y reporte de memoria.

#### Scenario S2: Toolchain inválido
- GIVEN selección ARM sin toolchain o con ruta de herramientas inválida
- WHEN se configura
- THEN falla con diagnóstico explícito y no genera una imagen host como firmware.

### Requirement B00-R2: Imagen coherente
El verificador MUST preservar contenido y fecha de modificación del ELF de entrada.
El ELF MUST ubicar vectores en flash inicial, pila alineada en SRAM local y secciones
sin superposición ni desbordes. MUST contener el checksum de boot NXP y conservar
CRP desactivado. MUST rechazar símbolos de arranque faltantes y exceso de memoria.

#### Scenario S3: Imagen válida
- GIVEN ELF recién enlazado
- WHEN se verifica estructura, símbolos, vectores, checksum y memoria
- THEN cumple el mapa LPC1769 y el punto de entrada coincide con Reset_Handler.

#### Scenario S4: Imagen corrupta
- GIVEN ELF truncado, no ARM, checksum inválido o vector fuera de flash
- WHEN se ejecuta el verificador
- THEN termina con código no cero y diagnóstico, sin aprobar la imagen.

#### Scenario S5: Límites de enlace
- GIVEN un fixture que desborda SRAM o no define main
- WHEN se enlaza con el script del proyecto
- THEN el linker rechaza el firmware.

### Requirement B00-R3: Arranque mínimo revisable
El startup MUST copiar .data de flash a RAM, borrar .bss y transferir control a main.
MUST disponer de handlers por defecto. Este bloque MUST NOT configurar periféricos,
PLL, reloj, flash timing o protección; MUST NOT flashear dispositivos.

#### Scenario S6: Ruta de arranque
- GIVEN startup y fixture con .data/.bss no vacías
- WHEN se enlaza, inspecciona y revisa su desensamblado
- THEN los límites y copia/borrado corresponden al linker y hay llamada a main;
  esto acredita revisión estática, no ejecución física de la inicialización.

### Requirement B00-R4: Host y evidencia separada
El build de objetos host MUST seguir disponible. Sus resultados MUST distinguirse
de compilación ARM, simulación y hardware; verificación del ELF no acredita arranque real.

#### Scenario S7: Perfil host
- GIVEN compilador host disponible
- WHEN se configura el perfil host y compila
- THEN produce objetos propios sin intentar enlazar startup ARM ni ejecutar main.
