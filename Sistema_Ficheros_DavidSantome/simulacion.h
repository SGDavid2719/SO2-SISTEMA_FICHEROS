//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include <sys/wait.h>
#include <signal.h>
#include "directorios.h"
#define REGMAX 500000 // (((12+256+256²+256³)-1)*BLOCKSIZE) /sizeof(struct registro)
#define NUMPROCESOS 100
#define NUMESCRITURAS 50

struct REGISTRO {
    time_t fecha; //fecha de la escritura en formato epoch 
    pid_t pid; //PID del proceso que lo ha creado
    int nEscritura; //Entero con el número de escritura (de 1 a 50)
    int nRegistro; //Entero con el número del registro dentro del fichero (de 0 a REGMAX-1) 
};
