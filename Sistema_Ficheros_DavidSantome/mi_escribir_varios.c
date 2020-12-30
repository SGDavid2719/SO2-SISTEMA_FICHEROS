//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa para probar la caché L/E.
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion "char *buffer_texto = argv[3], int longitud, escritos, varios, char *camino, unsigned int offset".
// Por parametros salida " ".
int main(int argc, char **argv){

    // Comprobamos Sintaxis
    if(argc!=5){
        fprintf(stderr, "sintaxis: ./mi_escribir_varios <disco> </ruta_fichero> <texto> <offset>\n");
        exit(-1);
    }
    // Montamos el dispositivo
    if(bmount(argv[1])<0) return -1;
    // Obtenemos el texto y su longitud
    char *buffer_texto = argv[3];
    int longitud=strlen(buffer_texto);

    // Obtenemos la ruta y comprobamos que no se refiera a un directorio
    if(argv[2][strlen(argv[2])-1]=='/'){
        fprintf(stderr,"Error: la ruta se corresponde a un directorio");
        exit(-1);
    }
    char *camino = argv[2];
    // Obtenemos el offset
    unsigned int offset=atoi(argv[4]);
    // Escribimos el texto
    int escritos=0;
    int varios = 10;
    fprintf(stderr,"longitud texto: %d\n", longitud);
    for(int i = 0; i < varios; i++){
        // Escribimos varias veces el texto desplazado 1 bloque
        escritos += mi_write(camino,buffer_texto,offset+BLOCKSIZE*i,longitud);
    }
    fprintf(stderr, "Bytes escritos %d\n", escritos);
    bumount();
}