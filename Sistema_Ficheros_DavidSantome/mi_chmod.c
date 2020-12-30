//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que cambia los permisos de un fichero o directorio, llamando a la función mi_chmod().
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion "unsigned int permisos".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char **argv)
{
    // Comprobamos sintaxis
	if(argc != 4)
    { 
        printf("Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n");
        return -1;
    }
    unsigned int permisos = atoi(argv[2]);
    
    // Miramos los permisos que sean validos
    if((permisos <= 7) && (permisos >=0))
    {
    	// Montamos el disco
        if(bmount(argv[1])==-1)
        {
            printf("Error (mi_chmod.c) en montar el disco \n");
            return -1;
        }
        // Cambiamos los permisos
   		int a = mi_chmod(argv[3], permisos);
        if(a < 0)
        {
            printf("Error al cambiar los permisos\n");
            return -1;
        }
        // Desmontamos el disco
        bumount();
        return 0;
    }
    printf("El valor de permisos esta fuera del rango");
    return -1;
}