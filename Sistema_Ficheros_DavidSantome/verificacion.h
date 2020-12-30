//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "simulacion.h"

struct INFORMACION {
    int pid; 
    unsigned int nEscrituras; // validadas 
    struct REGISTRO PrimeraEscritura; 
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};