# Estado — 2026-11-11

## Punto actual

- Repo /home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII,main.
  HEAD de incorporaciónbf975f3,origin/main62eef53;consultar Git al reanudar.
  El commit de este documento entrega corrección de auditoría B17 H1/H2.
- B00–B16/B03-R1/B07-R1/B05-R1 aceptados/archivados. B17 implementado/verificado,
  pendiente de /auditar B17 (r2) y aceptación;sin cierre/sincronización/archivo/push.
  [Informe](../../openspec/changes/b17-regulador-velocidad/verify-report.md).
- Auditoría inicial:APROBAR CON CAMBIOS,sin bloqueantes,código correcto.
  H1/H2 resueltos sólo en pruebas/infraestructura y seguimiento. Aplicacion/
  Procesamiento.c y toda producción byte a byte iguales a bf975f3.
- H1:dos configuraciones Ki>Kp y caso de ambos límites. H2:n=0 con e<0,
  integral4000 retenida y mando posterior4. Referencia Fraction y planta intactas.

## Evidencia propia de corrección

Verificar_b17 completo:host/san33/33 cada uno,ARM15/15,Doxygen46 sin avisos.
10 casos PI+2 composición,226 ejecuciones C por modalidad;8 configuraciones,
133758 pasos exactos=67966 deterministas+65536 pseudoaleatorios+256 init.
Planta14/14 pasos,picos99,6/185,2RPM;instancia40bytes,recursos repetidos.
Copia correcta pasa;mutantes de límites superior/inferior y n<0 fallan en unidad
y oráculo;AW/truncado anteriores también detectados.
Evidencia workspace build/verificacion-b17-h1-h2-2026-11-11,ignorada por Git.
Reproducir python3 scripts/verificar_b17.py --evidencia build/verificacion-b17-h1-h2-2026-11-11.
Entrega original conservada en build/verificacion-b17-2026-11-11.
Sin MCU/simulador CPU/placa/motor,flasheo,instalaciones o sintonía real.

## Próxima acción

/auditar B17 (r2). Aceptar resultado antes de sincronizar/archivar.
No iniciar otro bloque ni hacer push. Integración final pendiente:cadencia500ms,
main/ISR/Timer/PWM,ADC/DMA/VREF/primera muestra y ADGDR/ADDRn/erratas,
sintonía/pruebas físicas,diagramas/informe. Sensor desconectado fuera de alcance;
S8 conserva el modelo200RPM,distinto del dato histórico de motor≈185RPM.
