/* Pruebas host de Driver_Motor y Driver_Alarmas (B18/I2) con dispositivos simulados. */
#include "Driver_Alarmas.h"
#include "Driver_Motor.h"
#include "fakes_dispositivos.h"

static bool MotorS1_MandoLimitado(void)
{
    static const char *const orden[] = {"Motor_Init", NULL};
    const struct { uint16_t pedido; uint16_t aplicado; } casos[] = {
        {0U, 0U}, {1U, 1U}, {500U, 500U}, {999U, 999U}, {1000U, 1000U},
        {1001U, 1000U}, {UINT16_MAX, 1000U}, {0U, 0U}
    };
    Fakes_Reiniciar();
    Driver_Motor_Init();
    CHECK(Fakes_LlamadasSon(orden) && fakes.motor_escrituras == 0U);
    for (unsigned i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        Driver_Motor_Aplicar(casos[i].pedido);
        CHECK(fakes.motor == casos[i].aplicado && fakes.motor_escrituras == i + 1U);
    }
    CHECK(fakes.motor_rechazos == 0U);
    return true;
}

static bool AlarmasS1_LedYBuzzerJuntos(void)
{
    static const char *const orden[] = {"LED_Init", "Buzzer_Init", NULL};
    const bool pedidos[] = {true, true, false, true, false, false};
    Fakes_Reiniciar();
    Driver_Alarmas_Init();
    CHECK(Fakes_LlamadasSon(orden) && fakes.led_escrituras == 0U && fakes.buzzer_escrituras == 0U);
    for (unsigned i = 0U; i < sizeof(pedidos) / sizeof(pedidos[0]); ++i) {
        Driver_Alarmas_Aplicar(pedidos[i]);
        CHECK(fakes.led == pedidos[i] && fakes.buzzer == pedidos[i]);
        CHECK(fakes.led_escrituras == i + 1U && fakes.buzzer_escrituras == i + 1U);
    }
    return true;
}

int main(void)
{
    const Caso casos[] = {
        {"mando-motor S1 mando limitado a 1000", MotorS1_MandoLimitado},
        {"salidas-alarma S1 LED y buzzer juntos", AlarmasS1_LedYBuzzerJuntos}
    };
    EJECUTAR_CASOS(casos);
}
