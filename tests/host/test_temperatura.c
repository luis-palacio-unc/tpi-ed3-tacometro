/* Pruebas host del servicio Temperatura (B18/I2) con dispositivos simulados. */
#include "Temperatura.h"
#include "fakes_dispositivos.h"

static int32_t Leer(int32_t lm35)
{
    fakes.lm35 = lm35;
    return Temperatura_Leer();
}

/* Debe ser el primer caso: observa el arranque del filtro. */
static bool S1_ArranqueConLecturasDisponibles(void)
{
    Fakes_Reiniciar();
    CHECK(Leer(2000) == 2000);
    CHECK(Leer(3000) == 2500);
    CHECK(Leer(4000) == 3000);
    CHECK(Leer(5000) == 3500);
    return true;
}

static bool S2_PromedioDeLasUltimasCuatro(void)
{
    Fakes_Reiniciar();
    CHECK(Leer(6000) == 4500);
    CHECK(Leer(6000) == 5250);
    CHECK(Leer(6000) == 5750);
    CHECK(Leer(6000) == 6000);
    CHECK(Leer(6000) == 6000);
    CHECK(Leer(2501) == 5125);
    CHECK(Leer(2501) == 4250);
    CHECK(Leer(2501) == 3375);
    CHECK(Leer(2501) == 2501);
    CHECK(Leer(0) == 1875);
    return true;
}

int main(void)
{
    const Caso casos[] = {
        {"S1 arranque con lecturas disponibles", S1_ArranqueConLecturasDisponibles},
        {"S2 promedio de las ultimas cuatro", S2_PromedioDeLasUltimasCuatro}
    };
    EJECUTAR_CASOS(casos);
}
