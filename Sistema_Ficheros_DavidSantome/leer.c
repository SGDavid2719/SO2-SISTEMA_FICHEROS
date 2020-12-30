//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "ficheros.h"

// Programa que le pasaremos por línea de comandos un nº de inodo obtenido con el programa escribir.c.
// Por parametros entrada "int argc, char *argv[]".
// Variables usadas en la funcion "const unsigned int buffer_texto_SIZE, unsigned char buffer_texto[buffer_texto_SIZE], unsigned int offset, unsigned int ninodo, int leidos, unsigned int contador_bytes, struct STAT stat".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char *argv[])
{
    // Comprobamos sintaxis
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: leer <nombre_dispositivo> <numero_inodo>\n");
        return -1;
    }
    // Modificable. No ha de ser múltiplo del tamaño del bloque
    const unsigned int buffer_texto_SIZE = 1500;
    unsigned char buffer_texto[buffer_texto_SIZE];
    unsigned int offset = 0;
    unsigned int ninodo = atoi(argv[2]);
    int leidos;
    unsigned int contador_bytes = 0;
    struct STAT stat;
    if(bmount(argv[1])==-1){
        printf("Error (leer.c) en montar el disco \n");
        return -1;
    }
    // Filtra basura la consola
    memset(buffer_texto, 0, buffer_texto_SIZE);
    leidos = mi_read_f(ninodo, buffer_texto, offset, buffer_texto_SIZE);
    // La lectura del inodo no se puede hacer de todo el fichero de golpe con mi_read_f()
    // ya que nuestro sistema permite ficheros de hasta 16GB y eso no cabría en un buffer de memoria
    // Mientras haya bytes por leer
    while (leidos > 0)
    {
        // Se utiliza el write de sistema para mostrar los resultados
        write(1, buffer_texto, leidos);
        contador_bytes += leidos;
        offset += buffer_texto_SIZE;
        memset(buffer_texto, 0, buffer_texto_SIZE);
        leidos = mi_read_f(ninodo, buffer_texto, offset, buffer_texto_SIZE);
    }
    mi_stat_f(ninodo, &stat);
    fprintf(stderr, "\n\ntotal_leidos: %d\n", contador_bytes);
    fprintf(stderr, "tamEnBytesLog: %d\n", stat.tamEnBytesLog);
    bumount(argv[1]);
    return 0;
}
