//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "verificacion.h"

// Programa que recorrera el fichero prueba.dat.
// Por parametros entrada "int argc, char *argv[]".
// Variables usadas en la funcion " ".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int main(int argc, char const *argv[])
{
    // Comprobar el nº de argumentos; // uso: ./verificacion <disco> <directorio_simulacion>
    if (argc == 3)
    {
        // Obtenemos el disco
        const char *disco = argv[1];
        // Obtenemos el directorio
        const char *dir = argv[2];

        // Montar disco.
        bmount(disco);

        struct STAT stat;
        // Realizamos stat() del directorio para obtener el tamaño y el nº de entradas.
        mi_stat(dir, &stat);
        int numEntradas = stat.tamEnBytesLog / sizeof(struct entrada);

        fprintf(stderr, "numentradas: %d    NUMPROCESOS: %d \n", numEntradas,NUMPROCESOS);
        // Si el nº de entradas != NUMPROCESOS entonces devolver -1 fsi
        if (numEntradas != NUMPROCESOS)
        {
            perror("El numero de entrada es distinto al numero de procesos");
            return -1;
        }

        // Crear el fichero informe.txt dentro del directorio de simulación.
        char fic[100];
        strcpy(fic, dir);
        strcat(fic, "informe.txt");
        if (mi_creat(fic, 6) < 0)
        {
            perror("Error (verificacion.c): no se pudo crear el fichero");
            return -1;
        }
        
        int posinforme = 0;

        // Leer la entrada (en este caso todas las entradas) del directorio
        const unsigned int TAM_BUFFER = NUMPROCESOS * sizeof(struct entrada);
        struct entrada entradas[TAM_BUFFER];
        if (mi_read(dir, entradas, 0, NUMPROCESOS * sizeof(struct entrada)) < 0)
        {
            perror("Error en la lectura de las entradas");
            return -1;
        }

        // Para cada entrada del directorio de simulación (es decir, para cada proceso) hacer
        for (int i = 0; i < NUMPROCESOS; i++)
        {
            char dirAux[100];
            struct INFORMACION info;
            
            // Extraer el PID a partir del nombre de la entrada y guardarlo en el registro info
            // Inicializamos la informacion para evitar posible basura del proceso anterior
            // Hay que limpiar el buffer de escrituras antes de cada nuevo uso!!!
            char *nombre = strchr(entradas[i].nombre,'_');
            nombre++;
            info.pid = atoi(nombre);
            info.nEscrituras = 0;
            info.PrimeraEscritura.fecha = time(NULL);
            info.PrimeraEscritura.nEscritura = NUMESCRITURAS;
            info.PrimeraEscritura.nRegistro = 0;
            info.UltimaEscritura.fecha = 0;
            info.UltimaEscritura.nEscritura = 0;
            info.UltimaEscritura.nRegistro = 0;
            info.MenorPosicion.fecha = time(NULL);
            info.MenorPosicion.nEscritura = 0;
            info.MenorPosicion.nRegistro = REGMAX;
            info.MayorPosicion.fecha = time(NULL);
            info.MayorPosicion.nEscritura = 0;
            info.MayorPosicion.nRegistro = 0;

            // Montamos el camino hacia cada prueba.dat
            // dirAux = /simul_aaaammddhhmmss/entradas[i].nombre/prueba.dat
            strcpy(dirAux, dir);
            strcat(dirAux, entradas[i].nombre);
            strcat(dirAux, "/prueba.dat");

            // Preparamos el buffer siendo multiplo de BLOCKSIZE
            int cant_registros_buffer_ecrituras = 256;
            struct REGISTRO buffer_escrituras[cant_registros_buffer_ecrituras];
            int offset = 0;
            // Leer una escritura
            int read = mi_read(dirAux,buffer_escrituras,offset, sizeof(buffer_escrituras));
            // Mientras haya escrituras en prueba.dat por hacer
            while (info.nEscrituras < NUMESCRITURAS && read > 0)
            {
                for (int j = 0; j < cant_registros_buffer_ecrituras; j++)
                {
                    // Si la escritura es válida entonces
                    if (buffer_escrituras[j].pid == info.pid)
                    {
                        // Si es la primera escritura validada entonces
                        if (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura)
                        {
                            // Inicilizar los registros significativos con los datos de esa escritura
                            info.PrimeraEscritura.fecha = buffer_escrituras[j].fecha;
                            info.PrimeraEscritura.nEscritura = buffer_escrituras[j].nEscritura;
                            info.PrimeraEscritura.nRegistro = buffer_escrituras[j].nRegistro;
                        }else{
                            // Comparar fecha, y para una misma fecha, el nº de escritura (para obtener la primera y ultima escritura)
                            // con los datos de nuestros registros y actualizar éstos si es necesario
                            double diff_t = difftime(info.UltimaEscritura.fecha,buffer_escrituras[j].fecha);
                            if(diff_t < 0){
                                info.UltimaEscritura.fecha = buffer_escrituras[j].fecha;
                                info.UltimaEscritura.nEscritura = buffer_escrituras[j].nEscritura;
                                info.UltimaEscritura.nRegistro = buffer_escrituras[j].nRegistro;
                            }else if(diff_t == 0){
                                if(buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura){
                                    info.UltimaEscritura.fecha = buffer_escrituras[j].fecha;
                                    info.UltimaEscritura.nEscritura = buffer_escrituras[j].nEscritura;
                                    info.UltimaEscritura.nRegistro = buffer_escrituras[j].nRegistro;
                                }
                            }
                        }
                        // Miramos la mayor y menor de las escrituras con nRegistro
                        if (buffer_escrituras[j].nRegistro < info.MenorPosicion.nRegistro)
                        {
                            info.MenorPosicion.fecha = buffer_escrituras[j].fecha;
                            info.MenorPosicion.nEscritura = buffer_escrituras[j].nEscritura;
                            info.MenorPosicion.nRegistro = buffer_escrituras[j].nRegistro;
                        }
                        if (buffer_escrituras[j].nRegistro > info.MayorPosicion.nRegistro)
                        {
                            info.MayorPosicion.fecha = buffer_escrituras[j].fecha;
                            info.MayorPosicion.nEscritura = buffer_escrituras[j].nEscritura;
                            info.MayorPosicion.nRegistro = buffer_escrituras[j].nRegistro;
                        }
                        info.nEscrituras++;
                    }
                }
                // Leemos siguiente escritura
                offset += read;
                memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
                read = mi_read(dirAux, buffer_escrituras, offset, sizeof(buffer_escrituras));
            }

            // Añadir la información del struct info  al fichero informe.txt por el final
            char aux1[500]; //Buffer que ira recopilando la información para luego escribirla al fichero
            char aux2[100]; //Buffer que recogerá la información del proceso y la colocara en el otro buffer
            memset(aux1, 0, 500);
            memset(aux2, 0, 100);
            sprintf(aux1, "PID: %d\n", info.pid);
            sprintf(aux2, "Numero escrituras: %d\n", info.nEscrituras);
            strcat(aux1, aux2);
            memset(aux2, 0, 100);
            sprintf(aux2, "Primera escritura\t%d\t%d\t%s", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)));
            strcat(aux1, aux2);
            memset(aux2, 0, 100);
            sprintf(aux2, "Ultima escritura\t%d\t%d\t%s", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)));
            strcat(aux1, aux2);
            memset(aux2, 0, 100);
            sprintf(aux2, "Menor Posicion\t\t%d\t%d\t%s", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)));
            strcat(aux1, aux2);
            memset(aux2, 0, 100);
            sprintf(aux2, "Mayor Posicion\t\t%d\t%d\t%s\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, asctime(localtime(&info.MayorPosicion.fecha)));
            strcat(aux1, aux2);

            posinforme += mi_write(fic, aux1, posinforme, strlen(aux1));
            printf("%d) %d escrituras validadas en %s\n", i + 1, info.nEscrituras, dirAux);
        }
        bumount();
    }
    else
    {
        printf("uso: ./verificacion <disco> <directorio_simulacion>\n");
        return -1;
    }
}