//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"
#include <stdio.h>

// Programa para crear el dispositivo virtual.
// Crea el direcotiro raíz.
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion "int nbloques, int ninodos, unsigned char buffer[BLOCKSIZE]".
// Por parametros salida " ".
int main(int argc, char **argv){

    int nbloques;
    int ninodos;
    unsigned char buffer[BLOCKSIZE];
    if(argc != 3){
	    fprintf(stderr,"Sintaxis: ./mi_mkfs <nombre_dispositivo> <nbloques>\n");
	    exit(0);
    }
    nbloques = atoi(argv[2]);
    ninodos = nbloques/4;
    memset(buffer, 0, BLOCKSIZE);

    if(nbloques >= 0){ //El número de bloques pasados no puede ser negativo
        if(bmount(argv[1]) != -1){
            for (int i = 0; i < nbloques; i++){
                bwrite(i, buffer);
            }
            initSB(nbloques, ninodos);
	        initMB();
	        initAI();
            reservar_inodo ('d', 7);
            bumount();
        } else {
            fprintf(stderr,"Error (mi_mkfs.c) en montar el disco\n");
        }
    }
}

