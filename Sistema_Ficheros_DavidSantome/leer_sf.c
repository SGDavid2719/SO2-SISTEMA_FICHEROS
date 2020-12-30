//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "directorios.h"

// Programa que muestra la metainformación.
// Por parametros entrada "int argc, const char *argv[]".
// Variables usadas en la funcion "".
// Por parametros salida " ".
//int main(int argc, const char *argv[])
//{
	//if (argc == 2)
	//{		
		//bmount(argv[1]);

		//struct superbloque SB;
		//int ninodo;
		//if (bread(0, &SB) == -1) return -1;

		//leer_datos_superbloque(SB);		
		//recorrido_lista_enlazada_inodos_libres(SB);
		//reservamos_bloque_liberamos_bloque(SB);
       	//mapa_bits_bloques_metadatos_ocupados(SB);
		//int ninodo = reservar_inodo('d', 7);
		//datos_directorio_raiz(ninodo);		
		//nivel4(ninodo);
		
		//bumount(argv[1]);
	//}
	//else
	//{
		//printf(" Ha introducido un número de argumentos inválido !\n");
		//printf(" La forma correcta es ./leer_sf <nombre del dispositivo>\n");
	//}
//}

// Utilizado en el nivel 2 y 3

// Función para leer los datos del superbloque.
// Por parametros entrada "struct superbloque SB".
// Variables usadas en la funcion " ".
// Por parametros salida " ".
void leer_datos_superbloque(struct superbloque SB)
{
	printf("\nDATOS DEL SUPERBLOQUE\n");
	printf("SB.posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
	printf("SB.posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
	printf("SB.posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
	printf("SB.posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
	printf("SB.posPrimerBloqueDatos  = %d \n", SB.posPrimerBloqueDatos);
	printf("SB.posUltimoBloqueDatos = %d \n", SB.posUltimoBloqueDatos);
	printf("SB.posInodoRaiz = %d \n", SB.posInodoRaiz);
	printf("SB.posPrimerInodoLibre = %d \n", SB.posPrimerInodoLibre);
	printf("SB.cantBloquesLibres = %d \n", SB.cantBloquesLibres);
	printf("SB.cantInodosLibres = %d \n", SB.cantInodosLibres);
	printf("SB.totBloques = %d \n", SB.totBloques);
	printf("SB.totInodos = %d \n", SB.totInodos);
	printf("\n");

	//printf("\nSizeof struct superbloque: %lu\n", sizeof(struct superbloque));
	//printf("sizeof struct inodo is: %lu\n", sizeof(struct inodo));
}

// Utilizado en el nivel 2

// Función que recorre la lista enlazada de inodos libres.
// Por parametros entrada "struct superbloque SB".
// Variables usadas en la funcion "int y, x, contador".
// Por parametros salida " ".
void recorrido_lista_enlazada_inodos_libres(struct superbloque SB)
{
	printf ("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
	int y , x , contador = 0;
   	for(x = 0 ; x<tamAI(SB.totInodos); x++ )
	{
       	y = 0;
		// Array de inodos del tamaño de un bloque.
       	struct inodo bufInodos[BLOCKSIZE/INODOSIZE]; 
       	bread(SB.posPrimerBloqueAI+x,&bufInodos);
       	while( y < BLOCKSIZE/INODOSIZE && contador<SB.totInodos)
		{
         	printf("%d ",bufInodos[y].punterosDirectos[0]);
           	y++;
         	contador++;
        }
	}
	printf("\n\n");
}

// Utilizado en el nivel 3

// Función para reservar y liberar bloques.
// Por parametros entrada "struct superbloque SB".
// Variables usadas en la funcion "int nbloque".
// Por parametros salida " ".
void reservamos_bloque_liberamos_bloque(struct superbloque SB)
{
	printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
   	int nbloque=reservar_bloque();
	// Leemos y Salvamos el bloque
	if (bread(0, &SB) == -1) return;
   	printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", SB.posPrimerBloqueDatos);  
   	printf("SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);
   	liberar_bloque(nbloque);
	// Leemos y Salvamos el bloque
	if (bread(0, &SB) == -1) return;
   	printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);
}

// Utilizado en el nivel 3

// Función para mostrar el mapa de bits y bloques ocupados.
// Por parametros entrada "struct superbloque SB".
// Variables usadas en la funcion " ".
// Por parametros salida " ".
void mapa_bits_bloques_metadatos_ocupados(struct superbloque SB)
{
	printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
	printf("posSB (bloque %d): %d\n", posSB,leer_bit(posSB));
   	printf("posPrimerBloqueMB (bloque %d): %d\n", SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
   	printf("posUltimoBloqueMB (bloque %d): %d\n", SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
   	printf("posPrimerBloqueAI (bloque %d): %d\n", SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
   	printf("posUltimoBloqueAI (bloque %d): %d\n", SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
   	printf("posPrimerBloqueDatos (bloque %d): %d\n", SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
   	printf("posUltimoBloqueDatos (bloque %d): %d\n", SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));
}

// Utilizado en el nivel 3

// Función para mostrar los datos del direcotiro raiz.
// Por parametros entrada "int ninodo".
// Variables usadas en la funcion "struct tm *ts, char atime[80], char mtime[80], char ctime[80], struct inodo inodo".
// Por parametros salida " ".
void datos_directorio_raiz(int ninodo)
{
	printf("\nDATOS DEL DIRECTORIO RAIZ\n");
	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];
	struct inodo inodo;
	if (leer_inodo(ninodo, &inodo) == -1)
	{
		printf("Error (leer_sf) . No se pudo leer el inodo\n");
		exit(1);
	}
	printf("tipo: %c\n", inodo.tipo);
	printf("permisos: %d\n", inodo.permisos);
	ts = localtime(&inodo.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodo.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodo.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	printf("atime: %s\nctime: %s\nmtime: %s\n", atime, ctime, mtime);
	printf("nlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n\n", inodo.nlinks, inodo.tamEnBytesLog, inodo.numBloquesOcupados);
}

// Utilizado en el nivel 4

// Función para mostrar los datos del inodo reservado.
// Por parametros entrada "int ninodo".
// Variables usadas en la funcion "struct tm *ts, char atime[80], char mtime[80], char ctime[80], struct inodo inodo".
// Por parametros salida " ".
void datos_inodo_reservado(int ninodo)
{
	printf("\nDATOS DEL INODO RESERVADO %d\n", ninodo);
	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];
	struct inodo inodo;
	if (leer_inodo(ninodo, &inodo) == -1)
	{
		printf("Error (leer_sf) . No se pudo leer el inodo\n");
		exit(1);
	}
	printf("tipo: %c\n", inodo.tipo);
	printf("permisos: %d\n", inodo.permisos);
	ts = localtime(&inodo.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodo.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodo.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	printf("atime: %s\nctime: %s\nmtime: %s\n", atime, ctime, mtime);
	printf("nlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n\n", inodo.nlinks, inodo.tamEnBytesLog, inodo.numBloquesOcupados);
}

// Utilizado en el nivel 4

// Función para utilizar traducir_bloque_inodo.
// Por parametros entrada "int ninodo".
// Variables usadas en la funcion " ".
// Por parametros salida " ".
void nivel4 (int ninodo)
{
	printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n\n");
	ninodo = reservar_inodo('f', 6);
	traducir_bloque_inodo(ninodo, 8, 1);
	traducir_bloque_inodo(ninodo, 204, 1);
	traducir_bloque_inodo(ninodo, 30004, 1);
	traducir_bloque_inodo(ninodo, 400004, 1);
	traducir_bloque_inodo(ninodo, 468750, 1);
	datos_inodo_reservado(ninodo);
}

// Utilizado en el nivel 7

// Programa 
// Por parametros entrada "int argc, char **argv".
// Variables usadas en la funcion "struct superbloque SB".
// Por parametros salida " ".
int main(int argc, char **argv){
    if (argc!=2) {
        fprintf(stderr, "Sintaxis: pruebas_buscar_entrada <nombre_dispositivo>\n");
        exit(-1);
    }
    // Montamos el dispositivo
    if(bmount(argv[1])<0) {
		fprintf(stderr,"Error (leer_sf.c) en montar el disco \n");
		return -1;
	}
 
	struct superbloque SB;
	if (bread(0, &SB) == -1) return -1;
	leer_datos_superbloque(SB);

    // Mostrar creación directorios y errores
    //mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    //mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    //mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    //mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    //mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    //mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    //mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1); //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    //mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    //mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    //mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    //mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    //mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
    
    bumount(argv[1]);
}

// Función para utilizar mostrat_buscar_entrada.
// Por parametros entrada "char *camino, char reservar".
// Variables usadas en la funcion "unsigned int p_inodo_dir, unsigned int p_inodo, unsigned int p_entrada, int error".
// Por parametros salida " ".
void mostrar_buscar_entrada(char *camino, char reservar){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
    return;
}
 
