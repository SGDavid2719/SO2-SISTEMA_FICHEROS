//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que crea un fichero, llamando a la función mi_creat().
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion "char * nombre_fichero, int permisos, char * camino".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char **argv){
    
    if (argc != 4) {
        printf("Sintaxis: ./mi_touch <disco> <permisos> </ruta>\n");
        exit(1);
    }
    
    // Diferenciamos entre fichero y directorio
    const char *camino = argv[3];
    if(camino[strlen(camino)-1] == '/') { 
        fprintf(stderr, "Error (mi_touch.c): No es un fichero");
        return -1;
    }

    char * nombre_fichero = argv[1];
    if(bmount(nombre_fichero)==-1){
        printf("Error (mi_touch.c) en montar el disco %s\n",nombre_fichero);
        exit(1);
    }
    
    int permisos = atoi(argv[2]);
    // Hay que comprobar que permisos sea un nº válido (0-7).
    if(permisos<0 || permisos>7){
        printf("Error : modo inválido: <<%d>> \n",permisos);
        exit(1);
    }
    
    if(mi_creat(camino,permisos)==-1){
        exit(1);
    }
        
    if(bumount()==-1){
        printf("Error (mi_touch.c) en desmontar el disco %s\n",nombre_fichero);
        exit(1);
    }
    return 0;
}