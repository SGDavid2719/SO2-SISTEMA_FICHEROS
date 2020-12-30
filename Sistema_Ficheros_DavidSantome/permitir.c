//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "ficheros.h"
#include <string.h>

// Programa para cambiar los permisos por consola.
// Por parametros entrada "int argc, char *argv[]".
// Variables usadas en la funcion " ".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char *argv[])
{
    // Validación de Sintaxis
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return -1;
    }
    // Montar dispositivo
    if(bmount(argv[1])==-1){
        printf("Error (permitir.c) en montar el disco \n");
        return -1;
    }
    // Llamada a mi_chmod_f() con los argumentos recibidos, convertidos a entero
    mi_chmod_f(atoi(argv[2]), atoi(argv[3]));
    // Desmontar dispositivo
    bumount();
    return 0;
}
