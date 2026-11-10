# Diseño preliminar B12 — tres dispositivos GPIO

Usar módulos existentes del anexoB: Selector ESW13, LED ESW14 y Buzzer ESW15.
Primeras USW .13.1/.14.1/.15.1; sin nuevos módulos de abstracción ni estructuras.
Selector_Read usa GPIO_ReadValue(PORT_2), aísla bit6 y retorna true si es1.
Buzzer_Write usa GPIO_SetPinState con SET si activar, RESET si desactivar;
LED_Write hace lo inverso por el ánodo común. Puertos/pines propios de cada .c.
Ambas firmas son void(bool activar), sin contexto ni retorno: el GPIO original
void no detecta fallos eléctricos. Corrección explícita del usuario tras la revisión.
Driver_Alarmas sigue intacto: su firma bool(void*,bool) deberá revisarse en otro
bloque antes de conectar estas salidas. No agregar proveedores/adaptadores/casts.

Asignación elegida bajo autorización: selectorP2.6, buzzerP2.7, ambos altos. Evita
P2.10–13 (EINT0–3) y conserva P2.0–5 con opciones PWM. LEDP0.22 bajo se apoya en
esquema oficial OM13085 revD; verificar revisión física cuando haya ensayo.

Inicialización física, dirección, multiplexado, polarización de entrada, reloj, FIOMASK y llamada
desde main se difieren por instrucción explícita. Wrappers operan sobre GPIO ya
configurado; no se ejecutan ahora sobre la placa ni se conectan al lazo global.
Lectura no decide manual/automático; ese mapeo/antirrebote corresponde al consumidor.

Pruebas host de selección de bit, polaridad y pines vecinos con doble en API GPIO,
sin copiar/parchear el driver ni simular registros completos. Pruebas directas
de dispositivos, sin composición B07 hasta su revisión. Son pruebas de traducción lógica, no de GPIO
eléctrico. Compilar módulos con headers originales ARM, header aislado y Doxygen;
no afirmar enlace completo ni inicialización física. Firma B11 y servicios intactos.

Dispositivos es específico del hardware y puede usar los tipos y APIs de la biblioteca.
La independencia del MCU corresponde a las capas superiores. B07 sigue vigente
pero su composición con estas salidas queda pendiente de revisión. No agregar
intermediarios para ocultar hardware dentro de Dispositivos.
