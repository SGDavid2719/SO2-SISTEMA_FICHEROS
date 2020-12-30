//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que crea un enlace a un fichero, llamando a la función mi_link().
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion " ".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char **argv){
	if(argc != 4)
	{
		printf("Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
		return -1;
	}
	bmount(argv[1]);
	if(strlen(argv[2]) - 1 == '/')
	{
		fprintf(stderr, "(mi_link)Es un directorio, no un fichero\n");
		return -1;
	}
	if(mi_link(argv[2], argv[3]) < 0) return -1;
	bumount();
	return 0;
}