//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que permite cambiar el nombre de un fichero o un directorio.
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion " ".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char **argv){
    
    if (argc != 4) {
        printf("Sintaxis: ./mi_rn <disco> <fuente> <destino>\n");
        return -1;
    }

    char * nombre_fichero = argv[1];
    if(bmount(nombre_fichero)==-1){
        printf("Error (mi_rn.c) en montar el disco %s\n",nombre_fichero);
        return -1;
    }
    
    ///////
        
    if(bumount()==-1){
        printf("Error (mi_rn.c) en desmontar el disco %s\n",nombre_fichero);
        exit(1);
    }
    return 0;
}