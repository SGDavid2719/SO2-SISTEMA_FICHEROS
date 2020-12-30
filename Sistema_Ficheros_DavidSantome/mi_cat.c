//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que muestra todo el contenido de un fichero.
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion "const unsigned int TAM_BUFFER, unsigned char buf[TAM_BUFFER], unsigned int offset, int nbytes, unsigned int contador_bytes, struct STAT stat".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main (int argc, char **argv) {
	//Comprobamos sintaxis
	if(argc != 3)
	{
		fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
		return -1;
	}
	// Utilizamos una variable TAM_BUFFER para poder cambiar el tamaño del buffer sin modificar sentencias involucradas
	const unsigned int TAM_BUFFER = 1024;
	unsigned char buf[TAM_BUFFER];
	unsigned int offset = 0;
	int nbytes;
	unsigned int contador_bytes = 0;
	struct STAT stat;
	bmount(argv[1]);
	// Comprobamos que se trate de un fichero
	if(argv[2][strlen(argv[2])-1]=='/'){ // Directorio
		fprintf(stderr, "Se ha introducido un directorio, no un fichero\n");
		return -1;
    }
	// Filtramos basura inicializando a primeramente
	memset(buf, 0, TAM_BUFFER);
	nbytes = mi_read(argv[2], buf, offset, TAM_BUFFER);
    // Mientras haya bytes por leer
	while(nbytes > 0)
	{
		write(1, buf, nbytes);
		contador_bytes += nbytes;
		offset += nbytes;
		memset(buf, 0, TAM_BUFFER);
		nbytes = mi_read(argv[2], buf, offset, TAM_BUFFER);
	}
	if(nbytes < 0)
	{
		fprintf(stderr,"(mi_cat) Error al leer\n");
	}
    mi_stat(argv[2],&stat);
	fprintf(stderr, "\n\ntotal_leidos: %d\n", contador_bytes);
	bumount(argv[1]);
	return 0;
}
