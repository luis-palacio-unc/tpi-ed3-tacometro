/**
 * @file Medicion.h
 * @brief Configuracion fisica de medicion del TPI, externa al servicio generico.
 * @details Anteproyecto: disco de 60 ranuras; la captura contara un flanco por ranura.
 * El integrador entrega este factor junto con conteo/duracion coherentes al servicio.
 * @note R02/R15. No configura captura ni valida la senal electrica.
 */
#ifndef TPI_CONFIGURACION_MEDICION_H
#define TPI_CONFIGURACION_MEDICION_H

/** @brief Pulsos por vuelta del eje portadisco con un flanco por cada ranura. */
#define TPI_MEDICION_PULSOS_POR_VUELTA 60U

#endif /* TPI_CONFIGURACION_MEDICION_H */
