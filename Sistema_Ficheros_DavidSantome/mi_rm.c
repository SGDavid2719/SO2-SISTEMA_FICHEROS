//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que borra un fichero, llamando a la función mi_unlink().
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion " ".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char **argv){
	if(argc != 3)
	{
		printf("Sintaxis: ./mi_rm <disco> </ruta>\n");
		return -1;
	}
	// Diferenciamos entre fichero y directorio
    const char *camino = argv[2];
    if(camino[strlen(camino)-1] == '/') 
	{ 
        fprintf(stderr, "Error (mi_rm.c): No es un fichero");
        return -1;
    }
	bmount(argv[1]);
	if(mi_unlink(argv[2]) < 0) return -1;
	bumount();
	return 0;
}