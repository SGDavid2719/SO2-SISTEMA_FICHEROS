//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que lista el contenido de un directorio llamando a la función mi_dir().
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion "char tipo, const char *camino, char buffer[TAMBUFFER], int dir".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char **argv){
    
    if(argc < 3){
        printf("Sintaxis: ./mi_ls <disco> </ruta_directorio> \n");
        return -1;
    }
   
    char tipo;
    const char *camino = argv[2];
    if(camino[strlen(camino)-1] == '/') { // es un directorio
        tipo = 'd';
    }else{
        tipo = 'f';
    }

    if(bmount(argv[1])==-1){
        printf("Error (mi_ls.c) en montar el disco \n");
        return -1;
    }
    
    char buffer[TAMBUFFER];
    memset(buffer, 0, sizeof(buffer));
    int dir = mi_dir(camino, buffer, tipo);
    if(dir < 0){
        return -1;
    } else{
        if(tipo == 'd'){
            printf("Total entradas: %d\n\n", dir);
        }
        printf("Tipo    Permisos    mTime                Tamaño         Nombre\n");
        printf("---------------------------------------------------------------\n");
        printf("%s\n", buffer);
    }
    bumount();
    return 0;   
}