//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "bloques.h"

static int descriptor = 0;

// Declaración Semáforo Nivel 11
static sem_t *mutex;
static unsigned int inside_sc = 0;

//                      Función Modificada en Nivel 12
// Función para montar el fichero que usaremos como dispositivo virtual.
// Por parametros entrada "const char *camino".
// Variables usadas en la funcion "descriptor (varieable global)".
// Por parametros salida "descriptor".
int bmount(const char *camino)
{
    if (descriptor > 0) {
        close(descriptor);
    }

    if ((descriptor = open(camino, O_RDWR | O_CREAT, 0666)) == -1) {
        fprintf(stderr, "Error: bloques.c → bmount() → open()\n");
    }

    if (!mutex) { // Mutex == 0
        // El semáforo es único y sólo se ha de inicializar una vez en nuestro sistema (lo hace el padre)
        mutex = initSem(); // Lo inicializa a 1
        if (mutex == SEM_FAILED) {
            return -1;
        }
    }
    return descriptor;
}

//                      Función Modificada en Nivel 12
// Función para desmontar el fichero que usaremos como dispositivo virtual.
// Por parametros entrada " ".
// Variables usadas en la funcion "descriptor (varieable global)".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int bumount()
{
    descriptor = close(descriptor); 
    // Hay que asignar el resultado de la operación a la variable ya que bmount() la utiliza
    if (descriptor == -1) {
        fprintf(stderr, "Error: bloques.c → bumount() → close(): %d: %s\n", errno, strerror(errno));
        return -1;
    }
    deleteSem(); // Borramos semaforo 
    return 0;
}

// Función que escribe el contenido de un buffer de memoria en el bloque del dispositivo virtual especificado.
// Por parametros entrada "unsigned int nbloque, const void *buf".
// Variables usadas en la funcion "descriptor (varieable global), int desplazamiento, int bytesEscritos".
// Por parametros salida "bytesEscritos" caso SUCCESS "-1" caso FAILURE.
int bwrite(unsigned int nbloque, const void *buf)
{
    int desplazamiento = nbloque * BLOCKSIZE;
    // off_t lseek(int descriptor, off_t desplazamiento, int punto_de_referencia);
    lseek(descriptor, desplazamiento, SEEK_SET);
    // size_t write(int descriptor, const void *buf, size_t nbytes);
    int bytesEscritos = write(descriptor, buf, BLOCKSIZE);
    if (bytesEscritos != BLOCKSIZE){
	    perror("ERROR 0003 BWRITE");
        return -1;
    }
    // Se ha escrito correctamente
    // printf("Hemos escrito %i bytes", bytesEscritos);
    return bytesEscritos; 
}

// Función que lee del dispositivo virtual el bloque especificado en un buffer de memoria.
// Por parametros entrada "unsigned int nbloque, void *buf".
// Variables usadas en la funcion "descriptor (varieable global), int desplazamiento, int bytesLeidos".
// Por parametros salida "bytesLeidos" caso SUCCESS "-1" caso FAILURE.
int bread(unsigned int nbloque, void *buf)
{
    int desplazamiento = nbloque * BLOCKSIZE;
    lseek(descriptor, desplazamiento, SEEK_SET);
    // size_t read(int descriptor, void *buf, size_t nbytes);
    int bytesLeidos = read(descriptor, buf, BLOCKSIZE);
    if (bytesLeidos < 0){
	    perror("ERROR 0004 BREAD");
        return -1;
    }
    // Se ha leido correctamente
    // printf("Hemos leido %i bytes", bytesLeidos);
    return bytesLeidos; 
}

//      NIVEL 11

void mi_waitSem(){
    if(!inside_sc){
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem(){
    inside_sc--;
    if(!inside_sc){
        signalSem(mutex);
    }
}