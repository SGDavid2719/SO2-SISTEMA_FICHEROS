//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "ficheros.h"
#include <string.h>

// pROGRAMA para ejecutar la función mi_truncar_f().
// Por parametros entrada "int argc, char *argv[]".
// Variables usadas en la funcion "unsigned int ninodo, nbytes, struct STAT stat, struct tm *ts, char atime[80], mtime[80], ctime[80]".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char *argv[])
{
    // Validación de sintaxis
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return -1;
    }

    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);
    // Montamos el dispositivo virtual
    if(bmount(argv[1])==-1){
        printf("Error (truncar.c) en montar el disco \n");
        return -1;
    }
    // Si nbytes= 0 liberar_inodo() si no mi_truncar_f() fsi
    if (nbytes == 0)
    {
        liberar_inodo(ninodo);
    }
    else
    {
        mi_truncar_f(ninodo, nbytes);        
    }
    // Llamamos a mi_stat_f() para saber como estan los metadatos
    struct STAT stat;
    mi_stat_f(ninodo, &stat);
    struct tm *ts;
    char atime[80], mtime[80], ctime[80];
    ts = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    fprintf(stderr, "\nDATOS INODO %d:\n", ninodo);
    fprintf(stderr, "tipo=%c\n", stat.tipo);
    fprintf(stderr, "permisos=%d\n", stat.permisos);
    fprintf(stderr, "atime: %s\n", atime);
    fprintf(stderr, "ctime: %s\n", ctime);
    fprintf(stderr, "mtime: %s\n", mtime);
    fprintf(stderr, "nlinks=%d\n", stat.nlinks);
    fprintf(stderr, "tamEnBytesLog=%d\n", stat.tamEnBytesLog);
    fprintf(stderr, "numBloquesOcupados=%d\n\n", stat.numBloquesOcupados);
    // Desmontamos el dispositivo virtual
    bumount();
    return 0;
}
