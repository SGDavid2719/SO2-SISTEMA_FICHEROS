//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ficheros.h"

// Definicion de errores nivel 7
#define ERROR_EXTRAER_CAMINO -1
#define ERROR_PERMISO_LECTURA -2
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -3
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -4
#define ERROR_ENTRADA_YA_EXISTENTE -6
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -7
#define ERROR_PERMISO_ESCRITURA -5

// Definicion de variables nivel 8
#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000)
// suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos

struct entrada 
{
    char nombre[60];  //En el SF ext2 la longitud del nombre es 255
    unsigned int ninodo;
};

// Definición camino asociado a un ninodo nivel 9
struct UltimaEntrada
{
    char camino[512];
    int p_inodo;
};

// Definicion de funciones nivel 7
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_error_buscar_entrada(int error);
void mostrar_buscar_entrada(char *camino, char reservar);
//int main(int argc, char **argv);

// Definicion de funciones nivel 8
int mi_creat(const char *camino,unsigned char permisos);
int mi_dir(const char *camino,char *buffer, char tipo);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);

// Definicion de funciones nivel 9
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);

// Definicion de funciones nivel 10
int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);