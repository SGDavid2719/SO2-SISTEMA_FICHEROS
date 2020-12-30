//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "ficheros.h"
#include <string.h>

// Programa que escribirá texto en uno o varios inodos.
// Por parametros entrada "int argc, char *argv[]".
// Variables usadas en la funcion "unsigned int ninodo, nbytes, offset[5], int escritos, struct STAT stat, int diferentes_inodos, char *buffe, int i, struct tm *ts".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <"
                        "'$(cat fichero)'"
                        "> <diferentes_inodos>\nOffsets: 9.000, 209.000, 30.725.000, 409.605.000, 480.000.000\nSi diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
        return -1;
    }
    unsigned int ninodo, nbytes, offset[5] = {9000, 209000, 30725000, 409605000, 480000000};
    int escritos;
    struct STAT stat;
    int diferentes_inodos = atoi(argv[3]);
    if(bmount(argv[1])==-1){
        printf("Error (escribir.c) en montar el disco \n");
        return -1;
    }
    // Pasamos como argumento escribiéndolo en consola y calculamos su longitud
    nbytes = strlen(argv[2]);
    printf("\n");
    fprintf(stderr,"Offsets: 9.000, 209.000, 30.725.000, 409.605.000, 480.000.000\n");
    fprintf(stderr, "longitud texto: %d\n\n", nbytes);
    char *buffer =argv[2];
    for (int i = 0; i < 5; i++)
    {
        // Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets
        if (diferentes_inodos == 0 || i == 0){
            ninodo = reservar_inodo('f', 6);
            fprintf(stderr, "Nº inodo reservado: %d\n", ninodo); fflush(stdout);
            fprintf(stderr, "offset: %d\n", offset[i]); fflush(stdout);
            escritos = mi_write_f(ninodo, buffer, offset[i], nbytes);
            // Obtenemos los datos llamando a la función mi_stat_f()
            mi_stat_f(ninodo, &stat);
            fprintf(stderr, "Bytes escritos: %d\n\n", escritos);
            // Sólo a partir de script script3e1.sh
            struct tm *ts;
            char atime[80], mtime[80], ctime[80];
            ts = localtime(&stat.atime);
            strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
            ts = localtime(&stat.mtime);
            strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
            ts = localtime(&stat.ctime);
            strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
            // Imprimir campos del inodo utilizando la salida estandar de errores (stderr)
            fprintf(stderr, "DATOS INODO %d:\n", ninodo);
            fprintf(stderr, "tipo=%c\n", stat.tipo);
            fprintf(stderr, "permisos=%d\n", stat.permisos);
            fprintf(stderr, "atime: %s\n", atime);
            fprintf(stderr, "ctime: %s\n", ctime);
            fprintf(stderr, "mtime: %s\n", mtime);
            fprintf(stderr, "nlinks=%d\n", stat.nlinks);
            fprintf(stderr, "tamEnBytesLog=%d\n", stat.tamEnBytesLog);
            fprintf(stderr, "numBloquesOcupados=%d\n\n", stat.numBloquesOcupados);
        }
    }
    // Al acabar debemos desmontar el disco
    bumount();
    return 0;
}
