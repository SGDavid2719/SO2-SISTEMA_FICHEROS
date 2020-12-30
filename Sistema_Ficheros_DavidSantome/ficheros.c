//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "ficheros.h"

// Función para escribir el contenido de un buffer de memoria de tamaño nbytes en un fichero/directorio.
// Por parametros entrada "unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes".
// Variables usadas en la funcion "struct inodo inodo, int primerBLogico, ultimoBLogico, desp1, desp2, nbfisico, escritos, unsigned char buf_bloque[BLOCKSIZE], int tamEnBytesLog".
// Por parametros salida "escritos" caso SUCCESS "-1" caso FAILURE.
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;

    // Leemos el inodo correspondiente pasado por argumento
    if(leer_inodo(ninodo, &inodo) < 0)
    {
        perror("Error al leer el inodo en mi_write_f");
        return -1;
    }
    // (inodo.permisos & 2) == 2
    if((inodo.permisos & 2) == 0)
    {
        perror("Error permisos de escritura");
        return -1;
    }
    // Calculamos cuál va a ser el primer bloque lógico donde hay que escribir
    int primerBLogico = offset/BLOCKSIZE;
    // Calculamos cuál va a ser el último bloque lógico donde hay que escribir
    int ultimoBLogico = (offset + nbytes -1)/BLOCKSIZE;
    // Calculamos desplazamientos dentro de esos bloques donde cae el offset,
    // y los nbytes escritos a partir del offset
    // Calculamos el desplazamiento en el bloque para el offset
    int desp1 = offset % BLOCKSIZE;
    // Calculamos el desplazamiento en el bloque para ver donde llegan los nbytes escritos a partir del offset
    int desp2 = (offset + nbytes -1) % BLOCKSIZE;

    // Además se ha de poner una sección critica que englobe la llamada a 
    // traducir_bloque_inodo() con reservar=1 ya que eso implica reservar bloques.

    mi_waitSem(); // Niveles Semáforos

    // Obtenemos el nº de bloque físico, correspondiente a primerBLogico,
    // mediante traducir_bloque_inodo() (con reservar=1)
    int nbfisico = traducir_bloque_inodo(ninodo, primerBLogico, 1);
    
    mi_signalSem(); // Niveles Semáforos

    int escritos = 0;
    unsigned char buf_bloque[BLOCKSIZE];
    // Leemos ese bloque físico del dispositivo virtual y
    // lo almacenamos en un array de caracteres del tamaño de un bloque, buf_bloque
    memset(buf_bloque,0,BLOCKSIZE);
    if(bread(nbfisico, buf_bloque) < 0)
    {
        perror("Error al leer el bloque fisico en mi_write_f");
        return -1;
	}
    // CASO 1: Un único bloque
    if(primerBLogico == ultimoBLogico)
    {
        // Utilizamos memcpy() para escribir los nbytes (nbytes < BLOCKSIZE)
        memcpy(buf_bloque + desp1, buf_original, desp2 +1 - desp1); // nbytes 
        // Escribimos buf_bloque modificado en el nbfisico donde corresponde
        bwrite(nbfisico, buf_bloque);
           
        // Actualizamos el numero de bytes escritos
        escritos = nbytes;
    }
    else
    {// CASO 2: Más de un bloque
        // FASE 1: copiar los bytes restantes en la posición indicada por desp1
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        bwrite(nbfisico, buf_bloque);
        escritos = escritos + BLOCKSIZE - desp1;

        // FASE 2: bloques lógicos intermedios. Escribimos directamente el bloque
        
        mi_waitSem(); // Niveles Semáforos

        // Vamos a sobreescribir bloques completos por tanto no hace falta leerlos previamente ni utilizar el memcpy
        // sino escribir directamente el bloque correspondiente del buf_original.
        // Iteramos para cada bloque lógico intermedio i
        // (primerBLogico+1 nos colocamos en el primero de en medio hasta el penultimo)
        for(int i = primerBLogico +1; i < ultimoBLogico; i++)
        {
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            // Lo volcamos al dispositivo mediante un bwrite() en el bloque físico correspondiente a ese bloque lógico
            bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBLogico - 1) * BLOCKSIZE);
            escritos = escritos + BLOCKSIZE;
        }

        // FASE 3: ultimo bloque logico
        // Obtenido mediante la función traducir_bloque_inodo(ninodo, ultimoBLogico, 1)
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBLogico, 1);
        
        mi_signalSem(); // Niveles Semáforos

        // Hacemos un bread() del bloque físico correspondiente y almacenamos el resultado en el buffer
        bread(nbfisico, buf_bloque);
        // Copiamos los bytes de el byte lógico del último bloque hasta donde hemos de escribir
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 +1);
        // Contiene los nuevos datos, preservando los restantes originales
        bwrite(nbfisico, buf_bloque);
        escritos = escritos + desp2 + 1;
    }

    // Actualizamos inodo

    // Se puede hacer más granular poniendo la sección crítica en mi_write_f() 
    // pero que sólo afecte a la porcion de código donde se actualiza la información del inodo

    // Wait
    mi_waitSem(); // Niveles Semáforos 

    // Leer inodo
    if(leer_inodo(ninodo, &inodo) < 0)
    {
        perror("Error al leer el inodo en mi_write_f");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    // Actualizar ctime, mtime ( y tamEnBytesLog si ha variado el tamaño) 

    int tamEnBytesLog = escritos + offset;
    // Actualizamos solo si hemos escrito mas alla del final del fichero
    if(tamEnBytesLog > inodo.tamEnBytesLog)
    {
        inodo.tamEnBytesLog = tamEnBytesLog;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);

    // Escribir_inodo
    if(escribir_inodo(ninodo, inodo) < 0)
    {
        perror("Error al escribir el inodo en mi_write_f");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }
    
    // Signal
    mi_signalSem(); // Niveles Semáforos
    
    return escritos;
}

// Función para leer la información de un fichero/directorio y la almacena en un buffer de memoria.
// Por parametros entrada "unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes".
// Variables usadas en la funcion "struct inodo inodo, int primerBLogico, ultimoBLogico, desp1, desp2, nbfisico, leidos, unsigned char buf_bloque[BLOCKSIZE]".
// Por parametros salida "leidos" caso SUCCESS "-1" caso FAILURE.
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;

    // Se puede granularizar poniéndola en mi_read_f() 
    // pero que sólo afecte a la porcion de código donde se actualiza la información del inodo

    // Wait
    mi_waitSem(); // Niveles Semáforos

    // Leer inodo
    if(leer_inodo(ninodo, &inodo) < 0){
        perror("Error al leer el inodo en mi_read_f");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    // Actualizar el atime 
    inodo.atime = time(NULL);

    // Escribir el inodo
    if(escribir_inodo(ninodo, inodo) < 0){
        perror("Error al escribir el inodo en mi_read_f");

        mi_signalSem(); // Niveles Semáforos

        return -1;
    }

    // Signal
    mi_signalSem(); // Niveles Semáforos

    if((inodo.permisos & 4) != 4){
        perror("No hay permisos de lectura");
        return -1;
    }
    // No puede leer más allá del tamaño en bytes lógicos del inodo (final fichero)
    // // si offset >= inodo.tamEnBytesLog entonces
    if(offset >= inodo.tamEnBytesLog)
    {
        return 0;
    }
    // Leemos sólo los bytes desde el offset hasta EOF
    // si offset + nbytes >= inodo.tamEnBytesLog
    if((offset + nbytes) >= inodo.tamEnBytesLog){
        nbytes = inodo.tamEnBytesLog -offset; // nbytes := inodo.tamEnBytesLog - offset
    } 
    // Calculos iguales que en mi_write_f()
    int primerBLogico = offset/BLOCKSIZE;
    int ultimoBLogico = (offset + nbytes -1)/BLOCKSIZE;
    int desp1 = offset%BLOCKSIZE;
    int desp2 = (offset + nbytes -1)%BLOCKSIZE;
    int nbfisico = traducir_bloque_inodo(ninodo, primerBLogico, 0);
	int leidos = 0;
    // Buffer utilizado para recoger todo lo leido en un bloque del dispositivo
	unsigned char buf_bloque[BLOCKSIZE];
    // Hay que ir construyendo buf_original utilizando primeramente bread() 
    // para leer un bloque del dispositivo y copiando la porcion correspondiente con memcpy() al buf_original.
    //CASO 1: Un solo bloque
    if(primerBLogico == ultimoBLogico){
        // Reservar = 0
		if(nbfisico != -1){ // si el bloque físico existe entonces
			if(bread(nbfisico, buf_bloque) < 0){
                perror("Error en bread del bloque fisico en mi_read_f");
                return -1;
			}
			memcpy(buf_original, buf_bloque + desp1, nbytes);
		}
		leidos = nbytes;
	} else {//CASO 2: Más de un bloque, se realiza igual que en mi_write_f() pero con bread() en vez de bwrite()
        if(nbfisico != -1){ // Bloque inicial por desp1
			if(bread(nbfisico, buf_bloque) < 0){
                perror("Error al leer el bloque fisico en mi_read_f");
                return -1;
			}
			memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
		}
		leidos =BLOCKSIZE - desp1;
		for(int i = primerBLogico + 1; i < ultimoBLogico; i++){ // Bloques intermedios
			nbfisico = traducir_bloque_inodo(ninodo, i, 0);
			if(nbfisico != -1){
				if(bread(nbfisico, buf_bloque) < 0){
                    perror("Error al leer el bloque fisico en mi_read_f");
                    return -1;
			    }
				memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBLogico - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
			}
			leidos = leidos + BLOCKSIZE;
		}
		nbfisico = traducir_bloque_inodo(ninodo, ultimoBLogico, 0);
		if(nbfisico != -1){ // Bloque final
			if(bread(nbfisico, buf_bloque) < 0){
                perror("Error al leer el bloque fisico en mi_read_f");
                return -1;
			}
			//memcpy(buf_original + (BLOCKSIZE - desp1) + (ultimoBLogico - primerBLogico - 1) * BLOCKSIZE, buf_bloque, desp2 + 1);
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
		}
		leidos += desp2 + 1;
    }

    return leidos;
}

// Función que devuelve la metainformación de un fihcero/directorio. No devuelve los punteros.
// Por parametros entrada "unsigned int ninodo, struct STAT *p_STAT".
// Variables usadas en la funcion "struct inodo inodo".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int mi_stat_f(unsigned int ninodo, struct STAT *p_STAT)
{
    struct inodo inodo;
	if(leer_inodo(ninodo, &inodo) < 0){
        perror("Error al leer_inodo en mi_stat_f");
		return -1;
	}
    // Al pasarse por referencia se utiliza ->
	p_STAT->tipo = inodo.tipo;
	p_STAT->permisos = inodo.permisos;
	p_STAT->atime = inodo.atime;
	p_STAT->mtime = inodo.mtime;
	p_STAT->ctime = inodo.ctime;
	p_STAT->nlinks = inodo.nlinks;
	p_STAT->tamEnBytesLog = inodo.tamEnBytesLog;
	p_STAT->numBloquesOcupados = inodo.numBloquesOcupados;
	return 0;
}

// Función que cambia los permisos de un fichero/directorio.
// Por parametros entrada "unsigned int ninodo, unsigned char permisos".
// Variables usadas en la funcion "struct inodo inodo".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
	struct inodo inodo;

    mi_waitSem(); // Niveles Semáforos

	if(leer_inodo(ninodo, &inodo) < 0){
        perror("Error al leer el inodo");

        mi_signalSem(); // Niveles Semáforos

		return -1;
	}
	inodo.permisos = permisos;
    // Actualizamos el ctime
	inodo.ctime = time(NULL);
	if(escribir_inodo(ninodo, inodo) < 0){
        perror("Error al escribir el inodo");

        mi_signalSem(); // Niveles Semáforos

		return -1;
	}

    mi_signalSem(); // Niveles Semáforos

	return 0;
}

//      Nivel 6

// Función para truncar un fichero/directorio a los bytes indicados, liberando los bloques necesarios.
// Por parametros entrada "unsigned int ninodo, unsigned int nbytes".
// Variables usadas en la funcion "struct inodo inodo, int primerBL, int liberados".
// Por parametros salida "liberados" caso SUCCESS "-1" caso FAILURE.
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
	struct inodo inodo;
    // Leemos el inodo
	if(leer_inodo(ninodo, &inodo) < 0)
    {
        return -1;
    } 
    // Comprobamos que el inodo tenga permisos de escritura
	if((inodo.permisos & 2) == 0)
    {
        perror("Error permisos de escritura");
        return -1;
    }
    // No se puede truncar mas alla del tamaño en bytes logicos del fichero/directorio
    if (nbytes > inodo.tamEnBytesLog)
    {
        perror("Error: más bytes de los disponibles");
        return -1;
    }
    // Calculamos el primer bloque lógico a liberar
	int primerBL;
    // si nbytes % BLOCKSIZE = 0  entonces primerBL := nbytes/BLOCKSIZE
	if(nbytes % BLOCKSIZE == 0)
    {
        primerBL = nbytes / BLOCKSIZE;  
    } 
	else // si_no primerBL := nbytes/BLOCKSIZE + 1
    {
        primerBL = (nbytes / BLOCKSIZE) + 1;
    } 
    // Liberamos con liberar_bloques_inodo
	int liberados = liberar_bloques_inodo(primerBL, &inodo);
	if(liberados < 0)
    {
        perror("Error en liberar_bloques_inodo en mi_truncar_f");
        return -1;
    }
    // Actualizamos el inodo
	if(leer_inodo(ninodo, &inodo) < 0)
    {
        perror("Error al leer el inodo en mi_truncar_f");
        return -1;
    }

    // Marcar los punteros del inodo como "null" antes de liberar los bloques
    // Actualizamos mtime, ctime
    inodo.mtime = time(NULL);
	inodo.ctime = time(NULL);
	inodo.numBloquesOcupados -= liberados;
    // Actualizamos tamEnBytesLog. Pasará a ser igual a nbytes
	inodo.tamEnBytesLog = nbytes;
    // Guardamos el inodo
	if(escribir_inodo(ninodo, inodo) < 0){
        perror("Error al escribir el inodo en mi_truncar_f");
        return -1;
    }
    // Devolvemos la cantidad de bloques liberados
	return liberados;
}