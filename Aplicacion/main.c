/**
 * @file main.c
 * @brief Programa principal del tacometro: inicializacion y lazo cooperativo.
 * @details Conecta los servicios con la logica de Procesamiento. No incluye
 * encabezados de Dispositivos: todo acceso al hardware pasa por un servicio.
 * @note ESW.2.1.1. El startup inicializa .data y .bss antes de entrar aqui.
 */
#include "main.h"
#include "Comunicacion.h"
#include "Control.h"
#include "Driver_Alarmas.h"
#include "Driver_Motor.h"
#include "Mandos.h"
#include "Medicion_Velocidad.h"
#include "Procesamiento.h"
#include "Sistema.h"
#include "Temperatura.h"

/** @brief Estado de operacion vigente, actualizado una vez por periodo. */
static Procesamiento_Estado Aplicacion_Estado;
/** @brief Memoria del regulador PI de velocidad. */
static Procesamiento_Regulador Aplicacion_Regulador;

/**
 * @brief Calcular el mando del motor del periodo segun el estado vigente.
 * @details MANUAL: lectura del potenciometro. AUTO_ACTIVO: un paso del regulador
 * PI con la consigna vigente; si el regulador no entrega un mando calculado
 * (consigna cero o error) devuelve cero. AUTO_ESPERA: cero, y reinicia el PI.
 * @param[in] rpm_milesimas Velocidad medida en la ultima ventana.
 * @return Mando de 0 a 1000 por mil.
 * @note USW.2.1.1.4. Unidad interna; una llamada por periodo de 500 ms.
 */
static uint16_t Aplicacion_CalcularMando(uint32_t rpm_milesimas)
{
    uint16_t mando = 0U;
    if (Aplicacion_Estado == PROCESAMIENTO_MANUAL) {
        mando = Mandos_LeerManual();
    } else if (Aplicacion_Estado == PROCESAMIENTO_AUTO_ACTIVO) {
        const Medicion_Velocidad_Lectura medicion = {rpm_milesimas, MEDICION_VELOCIDAD_VALIDA};
        const Procesamiento_EstadoRegulador resultado = Procesamiento_Regular(
            &Aplicacion_Regulador, Procesamiento_Consigna(), &medicion, &mando);
        if (resultado != PROCESAMIENTO_REGULADOR_VALIDO &&
            resultado != PROCESAMIENTO_REGULADOR_RETENIENDO_MEDICION) {
            mando = 0U;
        }
    } else {
        (void)Procesamiento_ReiniciarRegulador(&Aplicacion_Regulador);
    }
    return mando;
}

/**
 * @brief Ejecutar el periodo de control de 500 ms.
 * @details Alarma, modo, mando y telemetria, en ese orden; ver Aplicacion_Iterar.
 * @param[in] rpm_milesimas Velocidad medida en la ventana que acaba de cerrar.
 * @note USW.2.1.1.5. Unidad interna.
 */
static void Aplicacion_Periodo(uint32_t rpm_milesimas)
{
    const int32_t temperatura = Temperatura_Leer();
    Driver_Alarmas_Aplicar(Procesamiento_EvaluarAlarma(temperatura));

    const Procesamiento_Estado anterior = Aplicacion_Estado;
    bool hay_consigna = false;
    uint32_t consigna = 0U;
    while (Comunicacion_RecibirConsigna(&consigna)) {
        hay_consigna = true;
    }
    Aplicacion_Estado = Procesamiento_ActualizarModo(Mandos_LeerModo(), hay_consigna, consigna);
    if (anterior == PROCESAMIENTO_MANUAL && Aplicacion_Estado == PROCESAMIENTO_AUTO_ESPERA) {
        Comunicacion_Descartar();
        (void)Procesamiento_ReiniciarRegulador(&Aplicacion_Regulador);
    }

    Driver_Motor_Aplicar(Aplicacion_CalcularMando(rpm_milesimas));

    (void)Comunicacion_EnviarTelemetria(rpm_milesimas, temperatura,
        Aplicacion_Estado == PROCESAMIENTO_MANUAL ? COMUNICACION_ESTADO_MANUAL :
        (Aplicacion_Estado == PROCESAMIENTO_AUTO_ESPERA ? COMUNICACION_ESTADO_AUTO_ESPERA :
                                                          COMUNICACION_ESTADO_AUTO_ACTIVO));
}

bool Aplicacion_Inicializar(void)
{
    const Procesamiento_ConfigRegulador ganancias = {
        TPI_CONTROL_KP_NUM, TPI_CONTROL_KP_DEN, TPI_CONTROL_KI_NUM, TPI_CONTROL_KI_DEN,
        TPI_CONTROL_MAX_PERIODOS_SIN_MEDICION
    };
    const bool sistema_listo = Sistema_Init();
    Driver_Motor_Init();
    Driver_Motor_Aplicar(0U);
    Driver_Alarmas_Init();
    Driver_Alarmas_Aplicar(!sistema_listo);
    if (!sistema_listo) {
        return false;
    }
    Mandos_Init();
    Comunicacion_Init();
    Aplicacion_Estado = Procesamiento_InicializarModo(Mandos_LeerModo());
    (void)Procesamiento_InicializarRegulador(&Aplicacion_Regulador, &ganancias);
    Medicion_Velocidad_Init();
    return true;
}

void Aplicacion_Iterar(void)
{
    uint32_t rpm_milesimas;
    if (Aplicacion_Estado == PROCESAMIENTO_MANUAL) {
        Driver_Motor_Aplicar(Mandos_LeerManual());
    }
    if (Medicion_Velocidad_Actualizar(&rpm_milesimas)) {
        Aplicacion_Periodo(rpm_milesimas);
    }
}

/**
 * @brief Punto de entrada: inicializar y repetir el lazo principal.
 * @details Si la inicializacion falla queda detenido, con el motor en cero y la
 * alarma encendida.
 * @return No retorna.
 * @note USW.2.1.1.1.
 */
int main(void)
{
    if (!Aplicacion_Inicializar()) {
        for (;;) {
        }
    }
    for (;;) {
        Aplicacion_Iterar();
    }
}
