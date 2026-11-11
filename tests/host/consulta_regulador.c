#include "Procesamiento.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    Procesamiento_Regulador r;
    Procesamiento_ConfigRegulador cfg = {4U,1000U,1U,1000U,4U};
    if (!Procesamiento_InicializarRegulador(&r,&cfg)) { return 2; }
    unsigned accion;
    uint32_t consigna, valor, estado;
    int campos;
    while ((campos=scanf("%u %" SCNu32 " %" SCNu32 " %" SCNu32 " %" SCNu32 " %" SCNu32 " %" SCNu32 " %" SCNu32 " %" SCNu32,
           &accion,&consigna,&valor,&estado,&cfg.kp_num,&cfg.kp_den,&cfg.ki_num,&cfg.ki_den,&cfg.max_periodos_sin_medicion)) == 9) {
        int resultado;
        uint16_t u=0U;
        if (accion==0U) {
            resultado=Procesamiento_InicializarRegulador(&r,&cfg) ? 1 : 0;
        } else if (accion==1U) {
            resultado=Procesamiento_ReiniciarRegulador(&r) ? 1 : 0;
        } else if (accion==2U) {
            const Medicion_Velocidad_Lectura m={valor,(Medicion_Velocidad_Estado)estado};
            resultado=(int)Procesamiento_Regular(&r,consigna,&m,&u);
        } else { return 2; }
        printf("%d %" PRIu16 " %" PRId64 " %" PRIu32 " %" PRIu32 " %u %u\n",
            resultado,u,r.integral_num,r.ultima_valida,r.periodos_invalidos,
            r.tiene_medicion ? 1U : 0U,r.configurado ? 1U : 0U);
        fflush(stdout);
    }
    return campos==EOF ? 0 : 2;
}
