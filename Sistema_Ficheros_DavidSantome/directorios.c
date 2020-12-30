//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

struct UltimaEntrada UltimaEntradaLectura;
struct UltimaEntrada UltimaEntradaEscritura;

// Función que dada una cadena de caracteres camino separa su contenido en dos inicial/final. Pasamos el tipo por referencia.
// Por parametros entrada "const char *camino, char *inicial, char *final, char *tipo".
// Variables usadas en la funcion "char *caminoAux, char *aux".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    if (camino[0] != '/')
    {
        return -1;
    }
    // Para trabajar más comodamente se crea un caminoAux de medida "camino+1" para evitar problemas
    char *caminoAux = (char *)malloc(strlen(camino)+1);
    strcpy(caminoAux, camino);
    caminoAux++;
    char *aux = strchr(caminoAux, '/');
    // Si no hay segundo '/' es un fichero
    if (!aux)
    {
        *tipo = 'f';
        strcpy(inicial, caminoAux);
        *final = '\0';
        return 0;
    }
    // Sino es un directorio
    strcpy(final, aux);
    strcpy(inicial, strtok(caminoAux, "/"));
    *tipo = 'd';
    //fprintf(stderr,"inicial: %s;\n final: %s;\n caminoAux: %s;\n aux: %s;\n camino: %s\n", inicial, final, caminoAux, aux, camino);
    return 0;
}

// Función que nos buscará una determinada entrada entre las entradas del inodo correspondiente a su directorio padre.
// Por parametros entrada "const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos".
// Variables usadas en la funcion "struct superbloque SB, char inicial, int error, struct inodo inodo_dir, struct entrada entrada, int cant_entradas_inodo, unsigned int offset".
// Por parametros salida "error" caso SUCCESS "-1" caso FAILURE.
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    // Si (es el directorio raíz) entonces
    if (!strcmp(camino_parcial, "/")) // Camino_parcial es “/”
    {
        // *p_inodo:=SB.posInodoRaiz
        *p_inodo = 0; // Nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0; // *p_entrada:=0
        return 0; // devolver 0
    }

    // inicial[sizeof(entrada.nombre)]: car
    // final[sizeof(strlen(camino_parcial))]: car
    char inicial[strlen(camino_parcial)], final[strlen(camino_parcial)], tipo;
    // extraer_camino (camino_parcial, inicial, final, &tipo)
    int error = extraer_camino(camino_parcial, inicial, final, &tipo);

    // Si error al extraer camino entonces devolver ERROR_EXTRAER_CAMINO  fsi
    if (error == -1)
    { 
        return ERROR_EXTRAER_CAMINO;
    }
    //fprintf(stderr,"[buscar_entrada() → inicial: %s, final: %s, reservar: %d]\n",inicial,final,reservar);

    // Buscamos la entrada cuyo nombre se encuentra en inicial
    struct inodo inodo_dir;
    // leer_inodo( *p_inodo_dir, &inodo_dir)
    leer_inodo(*p_inodo_dir, &inodo_dir);
    // si inodo_dir no tiene permisos de lectura entonces
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA; // devolver ERROR_PERMISO_LECTURA
    }

    // El buffer de lectura puede ser un struct tipo entrada 
    // o bien un array de las entradas que caben en un bloque, para optimizar la lectura en RAM
    struct entrada entrada;
    // Calcular cant_entradas_inodo = tamañoFicheroIndicandoElMasAlejado/tamañoEntarda
    int cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);  // Cantidad de entradas que contiene el inodo
    //  num_entrada_inodo := 0
    int num_entrada_inodo = 0;  // nº de entrada inicial
    unsigned int offset = 0;
    // Si cant_entradas_inodo > 0 entonces
    if (cant_entradas_inodo > 0)
    {
        // Leer entrada
        mi_read_f(*p_inodo_dir, &entrada, offset, sizeof(struct entrada));
        // Mientras ((num_entrada_inodo < cant_entradas_inodo) y (inicial ≠ entrada.nombre)) hacer 
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre)))
        {
            num_entrada_inodo++;
            offset += sizeof(struct entrada);
            memset(entrada.nombre, 0, sizeof(struct entrada)); // Previamente inicializar el buffer de lectura con 0s
            // Leer siguiente entrada 
            mi_read_f(*p_inodo_dir, &entrada, offset, sizeof(struct entrada));
        }
    }

    // Si (num_entrada_inodo = cant_entradas_inodo) y (inicial ≠ entrada.nombre) entonces
    if (num_entrada_inodo == cant_entradas_inodo)
    { // La entrada no existe
        switch (reservar)
        {

            case 0: // Modo consulta. Como no existe retornamos error
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:  // Modo escritura. 
                // Creamos la entrada en el directorio referenciado por *p_inodo_dir
                // Si es fichero no permitir escritura 
                if (inodo_dir.tipo == 'f') // si inodo_dir.tipo = ‘f’ entonces
                {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO; // devolver ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
                }
                // Si es directorio comprobar que tiene permiso de escritura
                if ((inodo_dir.permisos & 2) != 2) // si inodo_dir no tiene permisos de escritura entonces
                {
                    return ERROR_PERMISO_ESCRITURA; // devolver ERROR_PERMISO_ESCRITURA
                }
                else
                {
                    strcpy(entrada.nombre, inicial); // copiar inicial en el nombre de la entrada
                    if (tipo == 'd') // si tipo = 'd' entonces
                    {
                        if (!strcmp(final, "/")) // si final es igual a "/" entonces
                        {
                            // reservar un inodo como directorio y asignarlo a la entrada
                            int ninodo = reservar_inodo('d', permisos);
                            entrada.ninodo = ninodo;
                            //fprintf(stderr,"[buscar_entrada() → reservado inodo %d tipo %c con permisos %d para %s]\n",ninodo,tipo,permisos,inicial);                      
                        }
                        else // Cuelgan más diretorios o ficheros
                        {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO; // devolver ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
                        }
                    }
                    else  // Es un fichero
                    {
                        // reservar un inodo como fichero y asignarlo a la entrada
                        int ninodo = reservar_inodo('f', 6);
                        entrada.ninodo = ninodo;
                        //fprintf(stderr,"[buscar_entrada() → reservado inodo %d tipo %c con permisos %d para %s]\n",ninodo,tipo,permisos,inicial);
                    }
                    // escribir la entrada
                    error = mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada));
                    if (error == -1) // si error de escritura entonces
                    {
                        if (entrada.ninodo != -1) // si se había reservado un inodo para la entrada entonces 
                        {
                            liberar_inodo(entrada.ninodo); // liberar el inodo
                        }
                        return -1; // devolver EXIT_FAILURE
                    }
                }
        }
    }
    if ((!strcmp(final, "/")) || !(strcmp(final, "\0"))) // si hemos llegado al final del camino  entonces
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) // si (num_entrada_inodo < cant_entradas_inodo) && (reservar=1) entonces
        {
            // Modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE; // devolver ERROR_ENTRADA_YA_EXISTENTE
        }
        // Cortamos la recursividad
        *p_inodo = entrada.ninodo; //  asignar a *p_inodo el numero de inodo del directorio/fichero creado/leido
        *p_entrada = num_entrada_inodo; // asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        //fprintf(stderr,"[buscar_entrada() → creada entrada: %s, %d]\n",inicial,entrada.ninodo);
        return 0;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo; // asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada;
        // Llamada recursiva
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
}

// Función que mostrará un mensaje según el tipo de error.
// Por parametros entrada "int error".
// Variables usadas en la funcion "".
// Por parametros salida " ".
void mostrar_error_buscar_entrada(int error) {
    switch (error) {
        case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
        case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
        case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
        case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
        case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
        case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
        case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}

//      NIVEL 8

// Función que crea un fichero/directorio y su entrada de directorio.
// Por parametros entrada "const char *camino, unsigned char permisos".
// Variables usadas en la funcion "unsigned int p_inodo_dir, unsigned int p_inodo, unsigned int p_entrada, int error".
// Por parametros salida "error".
int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();   // Niveles Semáforos

    // Por simplicidad podemos suponer directamente que p_inodo_dir es 0
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;  

    // Se basa, principalmente, en la función buscar_entrada() con reservar=1
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    mi_signalSem(); // Niveles Semáforos
    
    return 0;
}

// Función que pone el contenido del directorio en un buffer de memoria y devuelve el número de entradas.
// Por parametros entrada "const char *camino, char *buffer, char tipo".
// Variables usadas en la funcion "unsigned int p_inodo_dir, unsigned int p_inodo, unsigned int p_entrada, char longitud[TAMFILA], struct entrada entrada, struct inodo inodo, int error, int num_entrada_inodo, cant_entradas_inodo, struct inodo inodoAux, struct tm *tm".
// Por parametros salida "error" caso SUCCESS "-1" caso FAILURE.
int mi_dir(const char *camino, char *buffer, char tipo)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    char longitud[TAMFILA];
    struct entrada entrada;
    struct inodo inodo;

    // Buscamos la entrada para comprobar que existe y leemos su inodo (*p_inodo_dir el del padre, *p_inodo el suyo)
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) == -1)
    {
        fprintf(stderr, "Error: (mi_dir) al leer el inodo\n");
        return -1;
    }

    // Comprobamos que tiene permisos de lectura
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "Error: (mi_dir) no hay permisos de lectura\n");
        return -1;
    }

    // Comprobamos que el tipo del inodo coincida con el tipo pasado por parametros
    if (inodo.tipo != tipo)
    {
        fprintf(stderr, "Error: (mi_dir) la sintaxis no concuerda con el tipo\n");
        return -1;
    }

    int num_entrada_inodo;
    int cant_entradas_inodo;
    // Si el inodo es un directorio miramos su contenido
    if (inodo.tipo == 'd')
    {
        num_entrada_inodo = 0;
        // Calculamos el numero de entradas que hay
        cant_entradas_inodo = inodo.tamEnBytesLog / sizeof(entrada);

        while (num_entrada_inodo < cant_entradas_inodo)
        {
            // Leemos la entrada del directorio
            if (mi_read_f(p_inodo, &entrada, num_entrada_inodo * sizeof(entrada), sizeof(entrada)) < 0)
            {
                fprintf(stderr, "Error: (mi_dir) ejecutando mi_read_f");
                return -1;
            }
            // Si es valida
            if (entrada.ninodo >= 0)
            {
                // Para cada entrada concatenamos su nombre al buffer con un separador y su información
                struct inodo inodoAux;
                if (leer_inodo(entrada.ninodo, &inodoAux) == -1)
                {
                    fprintf(stderr, "Error: (mi_dir) al leer el inodoAux");
                    return -1;
                }

                // Escribimos el tipo (fichero/directorio)
                if (inodoAux.tipo == 'd')
                {
                    strcat(buffer, "d"); // Tipo directorio
                }
                else if (inodoAux.tipo == 'f')
                {
                    strcat(buffer, "f"); // Tipo fichero
                }
                strcat(buffer, "\t");

                // Escribimos los permisos del fichero/directorio
                if (inodoAux.permisos & 4) strcat(buffer, "r");
                else strcat(buffer, "-");

                if (inodoAux.permisos & 2) strcat(buffer, "w");
                else strcat(buffer, "-");

                if (inodoAux.permisos & 1) strcat(buffer, "x");
                else strcat(buffer, "-");
                
                strcat(buffer, "\t");

                // Escribimos el tiempo del fichero/directorio
                // Como trabajar la información acerca del tiempo se enseña en las transparencias
                struct tm *tm;
                char tmp[100];
                tm = localtime(&inodoAux.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);
                strcat(buffer, "\t");

                // Escribimos el tamaño del fichero/directorio
                memset(longitud, '\0', sizeof(longitud));
                sprintf(longitud, " %d bytes", inodoAux.tamEnBytesLog);
                strcat(buffer, longitud);
                strcat(buffer, "\t");

                // Escribimos el nombre del fichero
                strcat(buffer, entrada.nombre);
                strcat(buffer, "\t");
                strcat(buffer, "\n");

                num_entrada_inodo++;
            }
            else
            {
                return 0;
            }
        }
    }
    // Si es un fichero imprimimos su propia información siguiendo la misma estructura que arriba
    else if (inodo.tipo == 'f')
    {
        num_entrada_inodo = 1;
        if (mi_read_f(p_inodo, &entrada, num_entrada_inodo * sizeof(entrada), sizeof(entrada)) < 0)
        {
            fprintf(stderr, "Error: (mi_dir) ejecutando mi_read_f");
            return -1;
        }

        struct inodo inodoAux;
        if (leer_inodo(p_inodo, &inodoAux) == -1)
        {
            perror("Error al leer_inodo en mi_dir");
            return -1;
        }

        // Escribimos el tipo (fichero) 
        strcat(buffer, "f");
        strcat(buffer, "\t");

        // Escribimos los permisos del fichero/directorio
        if (inodoAux.permisos & 4) strcat(buffer, "r");
        else strcat(buffer, "-");

        if (inodoAux.permisos & 2) strcat(buffer, "w");
        else strcat(buffer, "-");

        if (inodoAux.permisos & 1) strcat(buffer, "x");
        else strcat(buffer, "-");
        
        strcat(buffer, "\t");

        // Escribimos el tiempo del fichero
        struct tm *tm;
        char tmp[100];
        tm = localtime(&inodoAux.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        // Escribimos el tamaño del fichero
        memset(longitud, '\0', sizeof(longitud));
        sprintf(longitud, " %d bytes", inodoAux.tamEnBytesLog);
        strcat(buffer, longitud);
        strcat(buffer, "\t");

        // Escribimos el nombre del fichero
        strcat(buffer, entrada.nombre);
        strcat(buffer, "\t");
        strcat(buffer, "\n");
    }

    return num_entrada_inodo;
}

// Función que cambia los permisos de un fichero.
// Por parametros entrada "const char *camino, unsigned char permisos".
// Variables usadas en la funcion "unsigned int p_inodo_dir, unsigned int p_inodo, unsigned int p_entrada, int entrada".
// Por parametros salida "permisos" caso SUCCESS "-1" caso FAILURE.
int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    // Obtenemos el p_inodo
    int entrada = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (entrada < 0)
    {
        mostrar_error_buscar_entrada(entrada);
        return -1;
    }
    // Si la entrada existe llamamos a la funcion correspondiente, mi_chmod_f()
    return mi_chmod_f(p_inodo, permisos);
}

// Función que muestra la información acerca del inodo de un fichero o directorio.
// Por parametros entrada "const char *camino, struct STAT *p_stat".
// Variables usadas en la funcion "unsigned int p_inodo_dir, unsigned int p_inodo, unsigned int p_entrada, int entrada".
// Por parametros salida "p_inodo" caso SUCCESS "-1" caso FAILURE.
int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    // Obtenemos el p_inodo
    int entrada = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (entrada < 0)
    {
        mostrar_error_buscar_entrada(entrada);
        return -1;
    }
    // Si la entrada existe llamamos a la funcion correspondiente, mi_stat_f()
    if (mi_stat_f(p_inodo, p_stat) < 0)
    {
        perror("Error en mi_stat_f, en mi_stat");
        return -1;
    }
    return p_inodo;
}

//      NIVEL 9

// Función para leer contenido de un fichero.
// Por parametros entrada "const char *camino, void *buf, unsigned int offset, unsigned int nbytes".
// Variables usadas en la funcion "unsigned int p_inodo, p_inodo_dir = 0, p_entrada, int error, struct inodo inodo, int bytesLeidos".
// Por parametros salida "bytesLeidos" caso SUCCESS "-1" caso FAILURE.
// Usamos caché de directorios.
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo, p_inodo_dir = 0, p_entrada = 0;

    // Comprobaríamos si la escritura es sobre el mismo inodo sino
    if (!strcmp(camino, UltimaEntradaLectura.camino))
    {
        p_inodo = UltimaEntradaLectura.p_inodo;
    }
    else
    {
        // Buscamos la entrada para obtener el p_inodo
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        struct inodo inodo;
        if (leer_inodo(p_inodo, &inodo) < 0)
        {
            perror("Error al leer_inodo en mi_read");
            return -1;
        }
        // Actualizamos los campos de la ultimaEntradaEscritura
        UltimaEntradaLectura.p_inodo = p_inodo;
        memset(UltimaEntradaLectura.camino, 0, 512);
        // Inicializamos a para posible basura longitud camino[512]
        strcpy(UltimaEntradaLectura.camino, camino);
    }
    // Si existe llamamos a la función para escribir en el fichero
    int bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
    return bytesLeidos;
}

// Función para escribir contenido en un fichero.
// Por parametros entrada "const char *camino, const void *buf, unsigned int offset, unsigned int nbytes".
// Variables usadas en la funcion "unsigned int p_inodo, p_inodo_dir = 0, p_entrada, int error, struct inodo inodo, int bytesEscritos".
// Por parametros salida "bytesEscritoss" caso SUCCESS "-1" caso FAILURE.
// Usamos caché de directorios.
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo, p_inodo_dir = 0, p_entrada = 0;
    struct inodo inodo;

    // Comprobaríamos si la escritura es sobre el mismo inodo sino
    if (!strcmp(camino, UltimaEntradaEscritura.camino))
    {
        p_inodo = UltimaEntradaEscritura.p_inodo; // Actualizamos el p_inodo de la ultima entrada
    }
    else
    {
        // Buscamos la entrada para obtener el p_inodo
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        // Actualizamos los campos de la ultimaEntradaEscritura
        UltimaEntradaEscritura.p_inodo = p_inodo;
        // Inicializamos a para posible basura longitud camino[512]
        memset(UltimaEntradaEscritura.camino, 0, 512);
        strcpy(UltimaEntradaEscritura.camino, camino);
    }
    if (leer_inodo(p_inodo, &inodo) < 0)
    {
        perror("Error al leer_inodo en mi_write");
        return -1;
    }
    if (inodo.tipo != 'f')
    {
        perror("En mi_write, el inodo no es un fichero");
        return -1;
    }
    if ((inodo.permisos & 2) != 2)
    {
        return ERROR_PERMISO_ESCRITURA;
    }
    // Si existe llamamos a la función para escribir en el fichero
    int bytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);

    return bytesEscritos;
}

//      NIVEL 10

// Función que crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1.
// Por parametros entrada "const char *camino1, const char *camino2".
// Variables usadas en la funcion "unsigned int p_inodo_dir1, p_inodo1, p_entrada1, p_inodo_dir2, p_inodo2, p_entrada2, int error, int ninodo1, struct inodo inodo, struct entrada entrada".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int mi_link(const char *camino1, const char *camino2)
{
    mi_waitSem(); // Niveles Semáforos

    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;	
    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;

    // Comprobamos primera entrada
    int error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    // Comprobamos segunda entrada
    // En caso de no existir la creamos mediante la funcion buscar_entrada permisos 6
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);   
        
        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    int ninodo1 = p_inodo1;
    struct inodo inodo;
    if (leer_inodo(ninodo1, &inodo) == -1)
    {
        printf("Error (mi_link) . No se pudo leer el inodo\n");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    // Comprobamos que la ruta sea a un fichero, no se crean enlaces a directorios
    if (inodo.tipo != 'f')
    {
        printf("Error (mi_link) no es un fichero\n");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    struct entrada entrada;
    // Leemos la entrada correspondiente a camino2
    if (mi_read_f (p_inodo_dir2, &entrada, p_entrada2 * sizeof(entrada), sizeof(entrada)) == -1)
    {
		printf("Error (mi_link) ejecutando mi_read_f()\n");

        mi_signalSem(); // Niveles Semáforos

        return -1;
	}

    // Liberamos el inodo que se ha asociado a la entrada creada
	liberar_inodo (p_inodo2);

    // Creamos el enlaze, asociamos a esta entrada el mismo inodo que el asociado a ala entrada camino1
    entrada.ninodo = p_inodo1;
    
    // Escribimos la entrada modificada
	if (mi_write_f (p_inodo_dir2, &entrada, p_entrada2 * sizeof(entrada), sizeof(entrada)) == -1)
    {
		printf("Error (mi_link) ejecutando mi_write_f()\n");

        mi_signalSem(); // Niveles Semáforos

        return -1;
	}

    if (leer_inodo(ninodo1, &inodo) == -1)
    {
        printf("Error (mi_link) . No se pudo leer el inodo\n");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }
    
    // Incrementamos la cantidad de enlaces 
    inodo.nlinks++;
    // Actualizamos el ctime
    inodo.ctime = time(NULL);

    // Salvamos el inodo
    if (escribir_inodo(ninodo1, inodo) == -1)
    {
        printf("Error (mi_link) en escribir el inodo\n");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }
    
    mi_signalSem(); // Niveles Semáforos

    return 0;
}

// Función que borra la entrada de directorio especificada y, en caso de que fuera el último enlace existente, borrar el propio fichero/directorio.
// Por parametros entrada "const char *camino".
// Variables usadas en la funcion "unsigned int p_inodo, p_inodo_dir = 0, p_entrada, int error, struct inodo inodo_1, int nentradas, int val, struct inodo inodo_2".
// Por parametros salida "0" caso SUCCESS "error" caso FAILURE.
int mi_unlink(const char *camino)
{
    mi_waitSem(); // Niveles Semáforos
    
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    
    // Comprobamos que la entrada camino exista (p_inodo_dir)
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);

    if (error == 0)
    {
        // Si existe la entrada obtenemos su numero leyendo el inodo 
	    struct inodo inodo_2;
        if (leer_inodo(p_inodo, &inodo_2) == -1)
        {
            printf("Error (mi_unlink) . No se pudo leer el inodo\n");

            mi_signalSem(); // Niveles Semáforos

            return -1;
        }
	    // Leemos el inodo que esta asociado al directorio
    	struct inodo inodo_1;
        if (leer_inodo(p_inodo_dir, &inodo_1) == -1)
        {
            printf("Error (mi_unlink) . No se pudo leer el inodo\n");

            mi_signalSem(); // Niveles Semáforos

            return -1;
        }

        // Obtenemos el numero de entradas del inodo
	    int nentradas = inodo_1.tamEnBytesLog / sizeof (struct entrada);

        if (inodo_2.tamEnBytesLog != 0 && inodo_2.tipo == 'd')
        {
		    printf("Error: El directorio %s no está vacío\n",camino);

            mi_signalSem(); // Niveles Semáforos

		    return -1;
	    }

        // Si la entrada a eliminar es la ultima bastan con truncar el inodo con su tamaño menos el tamaño de una entrada
	    if (p_entrada != nentradas - 1){
		    struct entrada entrada;
		    // Leemos la ultima entrada
		    if (mi_read_f (p_inodo_dir, &entrada, inodo_1.tamEnBytesLog - sizeof(struct entrada), sizeof(struct entrada)) == -1)
            {
			    printf("Error (mi_unlink) ejecutando mi_read_f()\n");

                mi_signalSem(); // Niveles Semáforos

                return -1;
		    }
		    // Colocamos en la posicion de entrada lo que queremos eliminar
		    if (mi_write_f (p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == -1)
            {
			    printf("Error (mi_unlink) ejecutando mi_write_f()\n");

                mi_signalSem(); // Niveles Semáforos

                return -1;
		    }
        }

	    // Truncamos el inodo a su tamaño menos el tamaño de una entrada
	    mi_truncar_f (p_inodo_dir, inodo_1.tamEnBytesLog - sizeof (struct entrada));
	    // Leemos el inodo asociado a la entrada eliminada 
	    if (leer_inodo(p_inodo, &inodo_1) == -1)
        {
            printf("Error (mi_unlink) . No se pudo leer el inodo\n");

            mi_signalSem(); // Niveles Semáforos

            return -1;
        }
	    // Decrementamos el numero de enlaces
	    inodo_1.nlinks--;         
    	// Si no quedan enlaces, liberamos el inodo
	    if (inodo_1.nlinks == 0)
        {
		    liberar_inodo (p_inodo);
	    }
        else
        {
		    // Sino actualizamos ctime y el inodo
		    inodo_1.ctime = time (NULL);
		    if (escribir_inodo (p_inodo, inodo_1) == -1)
            {
                printf("Error (mi_unlink) . No se pudo escribir en el inodo\n");

                mi_signalSem(); // Niveles Semáforos

                return -1;
            }
	    }
	}  
    else
    {
        mostrar_error_buscar_entrada(error);

        mi_signalSem(); // Niveles Semáforos
        
        return -1;
    }
    
    mi_signalSem(); // Niveles Semáforos

    return 0;
}