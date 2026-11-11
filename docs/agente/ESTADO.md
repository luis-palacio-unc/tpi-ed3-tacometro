# Estado — 2026-11-11

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`,main.
  HEAD de incorporación9352f0d,origin/main62eef53; consultar Git al reanudar.
  El commit de este documento entrega implementación B17 lista para auditoría.
- B00–B16/B03-R1/B07-R1/B05-R1 aceptados/archivados; B17 implementado/verificado,
  **pendiente de /auditar B17 y aceptación**. Sin cierre/sincronización/archivo/push.
  [Informe](../../openspec/changes/b17-regulador-velocidad/verify-report.md).
- Propuesta Claude r2 APROBAR; usuario autoriza tareas1.2–4.6,commit y evidencia
  workspace. m1: empate Kp=Ki=1/1000,1RPM/500milésimas→mando1 en S1/pruebas.
- USW.2.1.2.8–10, PI entero separado B01. Control.h ganancias/N,periodo500ms
  contractual; retención sin integrar y reinicio sin historia.
- B01/B06 anteriores,Servicios,Dispositivos,90 Drivers originales,main,contratos
  aceptados/archivo previo intactos. Sin inicialización o enlace productivo nuevos.

## Evidencia propia

Host33/33 y ASan/LSan/UBSan33/33,sin omisiones:8 casos PI+2 composición,
116722 pasos exactos por modalidad y25 probes. Planta: establecimiento14/14 pasos,
picos99,6/185,2RPM. ARM15/15,Doxygen46 sin avisos.
Copias externas: quitar AW falla S2/oráculo; truncar falla S1/oráculo.
Instancia ARM40bytes; recursos/pila/libgcc separados en informe.
Evidencia: build/verificacion-b17-2026-11-11,dentro del workspace,ignorada.
Reproducir `python3 scripts/verificar_b17.py`. Sin MCU/simulador CPU/placa/motor,
sintonía real,flasheo o instalaciones.

## Próxima acción

`/auditar B17`. Revisar implementación/informe/evidencia y aceptar antes de
sincronizar/archivar. No iniciar otro bloque ni hacer push.
Integración final: cadencia500ms,main/ISR/Timer/PWM,ADC/DMA/VREF/primera muestra,
ADGDR/ADDRn/erratas,sintonía/pruebas físicas,diagramas/informe.
Sensor desconectado fuera de alcance. Motor≈185RPM al100%: observación del usuario
en trabajo anterior. S8 conserva modelo200RPM al100%,sin equivalencia física asumida.
