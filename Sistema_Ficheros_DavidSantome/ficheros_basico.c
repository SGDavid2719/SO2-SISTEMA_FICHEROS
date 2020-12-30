//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "ficheros_basico.h"

// Estructura interna del sistema de ficheros (Super-bloque, mapa de bits, array de inodos y datos)

// Funcion que calcula el tamaño del mapa de bits en bloques.
// Por parametros entrada "unsigned int nbloques".
// Variables usadas en la funcion " ".
// Por parametros salida "tamañoMapaBits".
int tamMB(unsigned int nbloques)
{	
    // (nbloques / 8bits) / BLOCKSIZE
    // Utilizaremos el operador modulo % para saber si necesitamos esa cantidad justa 
    // o si necesitamos añadir un bloque adicional
    if ((nbloques / 8) % BLOCKSIZE == 0)
    {
        return ((nbloques / 8) / BLOCKSIZE);
    }
    else
    {
        return ((nbloques / 8) / BLOCKSIZE) + 1;
    }   
}

// Funcion para devolver el tamaño del array de inodos en bloques.
// Por parametros entrada "unsigned int ninodos".
// Variables usadas en la funcion " ".
// Por parametros salida "tamañoArrayInodos".
int tamAI(unsigned int ninodos)
{
    // (ninodos * INODOSIZE) / BLOCKSIZE
    // Utilizaremos el operador modulo % para saber si necesitamos esa cantidad justa 
    // o si necesitamos añadir un bloque adicional
    if ((ninodos * INODOSIZE) % BLOCKSIZE == 0)
    {
        return ((ninodos * INODOSIZE) / BLOCKSIZE);
    }
    else
    {
        return ((ninodos * INODOSIZE) / BLOCKSIZE) + 1;
    }  
}

// Funcion que permite rellenar los datos basicos del superbloque.
// Por parametros entrada "unsigned int nbloques, unsigned int ninodos".
// Variables usadas en la funcion "struct superbloque SB".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;
    // Posición del primer bloque del mapa de bits
    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB = 1
    // Posición del último bloque del mapa de bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    // Posición del primer bloque del array de inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    // Posición del último bloque del array de inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    // Posición del primer bloque de datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    // Posición del último bloque de datos
    SB.posUltimoBloqueDatos = nbloques - 1;
    // Posición del inodo del directorio raíz en el array de inodos
    SB.posInodoRaiz = 0;
    // Posición del primer inodo libre en el array de inodos
    SB.posPrimerInodoLibre = 0;
    // Cantidad de bloques libres en el SF
    SB.cantBloquesLibres = nbloques;
    // Cantidad de inodos libres en el array de inodos
    SB.cantInodosLibres = ninodos;
    // Cantidad total de bloques
    SB.totBloques = nbloques;
    // Cantidad total de inodos
    SB.totInodos = ninodos;
    // Guardamos la informacion del superbloque en el sistema de ficheros
    if (bwrite(posSB,&SB) == -1)return -1; 
    return 0;
}

// Funcion para definir una zona de memoria con todos los bits a zero.
// Por parametros entrada " ".
// Variables usadas en la funcion "struct superbloque SB, int i".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
// Sin mejora.
int initMB()
{
    struct superbloque SB;
    // Leemos primeramente el superbloque
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en initMB -> bread");
        return -1;
    }
    // Tenemos en cuenta los bloques ocupados para el superbloque
    for (int i = posSB; i <= SB.posUltimoBloqueAI; i++)
    {
        if (escribir_bit(i, 1) == -1)
            return -1;
    }
    SB.cantBloquesLibres -= SB.posUltimoBloqueAI + 1;
    // Escribimos en los bloques correspondientes
    if (bwrite(posSB,&SB) == -1)return -1; 
    return 0;
}

// Funcion que nos permite enlazar todos los inodos entre si.
// Por parametros entrada " ".
// Variables usadas en la funcion "struct superbloque SB, int i, struct inodo inodos[BLOCKSIZE / INODOSIZE], int variable_incremental".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int initAI()
{
    struct superbloque SB;
    // struct inodo inodos [BLOCKSIZE/INODOSIZE]
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // Leemos el superbloque para localizar array de inodos
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en initAI -> bread");
        return -1;
    }
    // contInodos := SB.posPrimerInodoLibre+1;
    int variable_incremental = SB.posPrimerInodoLibre + 1;
    // Si hemos inicializado SB.posPrimerInodoLibre = 0
    // para (i=SB.posPrimerBloqueAI; i<=SB.posUltimoBloqueAI;i++) hacer
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // para (j=0; j<BLOCKSIZE / INODOSIZE; j++) hacer
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            // inodos[j].tipo := ‘l’;
            inodos[j].tipo = 'l'; // Libre
            // si (contInodos < SB.totInodos) entonces
            if (variable_incremental < SB.totInodos)
            {
                // Iniciamos primer elemento de cada inodo con variable_incremental
                // inodos[j].punterosDirectos[0] := contInodos;
                inodos[j].punterosDirectos[0] = variable_incremental;
                // contInodos++;
                variable_incremental++;
            }
            else
            {   // si_no //hemos llegado al último inodo
                // Ultimo de la lista apunta a un numero muy grande (NULL)
                // inodos[j].punterosDirectos[0] := UINT_MAX;
                inodos[j].punterosDirectos[0] = UINT_MAX;
                // Hay que salir del bucle, el último bloque no tiene por qué estar completo
            }
        }
        // Escribir el bloque de inodos en el dispositivo virtual
        if (bwrite(i, &inodos) == -1)
        {
            return -1;
        }
    }
    return 0;
}

//      NIVEL 3

// Programacion de las funciones básicas de E/S para bits del mapa de bits y escribe el valor 0 (libre) o 1 (ocupado) en un determinado bit del mapa de bits
// Por parametros entrada "unsigned int nbloque, unsigned int bit".
// Variables usadas en la funcion "struct superbloque SB, unsigned int posbyte, unsigned int posbit, unsigned int nbloqueMB, unsigned int nbloqueabs, unsigned char mascara, unsigned char bufferMB[BLOCKSIZE]".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int escribir_bit(unsigned int nbloque, unsigned int bit) 
{
    struct superbloque SB;
    // Leemos el superbloque para obtener la localización del MB
	if(bread(posSB, &SB) < 0){
		perror("Error: bread() en escribir_bit()");
		return -1;
	}
    // Calculamos la posicion del byte, bit y bloque en el mapa de bits en el cual debemos leer
	unsigned int posbyte = nbloque / 8;
	unsigned int posbit = nbloque % 8;
    // Hemos de determinar luego en qué bloque del MB, nbloqueMB, se halla ese bit para leerlo
	unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    // Finalmente hemos de obtener en qué posición absoluta del dispositivo virtual se encuentra ese bloque
	unsigned int nbloqueabs = nbloqueMB + SB.posPrimerBloqueMB;
	unsigned char mascara = 128; // 10000000
	unsigned char bufferMB[BLOCKSIZE];
    // Leemos el bloque que vamos a modificar
	if(bread(nbloqueabs, &bufferMB) < 0){
		perror("Error: bread() en escribir_bit()");
		return -1;
	}
    // Desplazamiento de bits a la derecha
	mascara >>= posbit;
    // Necesitamos realizar la operación módulo con el tamaño de bloque para localizar su posición
	posbyte %= BLOCKSIZE;
    // Miramos si tenemos que escribir un 1 o un 0
    // Operadores AND y NOT para bits
	if(bit == 0) bufferMB[posbyte] &= ~mascara;
    // Operador OR para bits
	else if(bit == 1) bufferMB[posbyte] |= mascara;
	else{
		perror("Error: Bit pasado por parametro incorrecto");
		return -1;
	}
    // Guarda los cambios tanto en el mapa de bits como en el superbloque
	if(bwrite(nbloqueabs, &bufferMB) < 0){
		perror("Error: bwrite() en escribir_bit");
		return -1;
	}
    return 0;
}

// Lee un determinado bit del mapa de bits al igual que la funcion anterior.
// Por parametros entrada "unsigned int nbloque".
// Variables usadas en la funcion "struct superbloque SB, unsigned int posbyte, unsigned int posbit, unsigned int nbloqueMB, unsigned int nbloqueabs, unsigned char mascara, unsigned char bufferMB[BLOCKSIZE]".
// Por parametros salida "mascara" caso SUCCESS "-1" caso FAILURE.
unsigned char leer_bit (unsigned int nbloque)
{
    // Se procede igual que en la función anterior para obtener el byte del dispositivo que contiene
    // el bit deseado y el bloque físico absoluto que lo contiene
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error: bread() en leer_bit");
        return -1;
    }
    unsigned int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned int bufferMB[BLOCKSIZE];
    unsigned char mascara = 128; // 10000000
    // Calculamos el bloque, byte y bit que está el bloque pasado por parámetro en el mapa de bits
    // Calculamos el número de bloque que contiene el bit
    posbyte = nbloque / 8;
    posbit = nbloque % 8;
    nbloqueMB = posbyte / BLOCKSIZE;
    nbloqueabs = nbloqueMB + SB.posPrimerBloqueMB;
    if (bread(nbloqueabs, bufferMB) == -1)
    {
        perror("Error: bread() en leer_bit");
        return -1;
    }
    // Desplazamiento de bits a la derecha
    mascara >>= posbit;    
    // Operador AND para bits
    posbyte = posbyte % BLOCKSIZE;
    mascara &= bufferMB[posbyte]; 
    // Desplazamiento de bits a la derecha
    mascara >>= (7 - posbit);
    return mascara;
}

// Funcion que encuentra el primer bloque libre y nos devuelve su posicion ocupandolo con la funcion escribir_bit.
// Por parametros entrada " ".
// Variables usadas en la funcion "struct superbloque SB, unsigned char bufferMB[BLOCKSIZE], int bloque, int posbyte, int posbit, int posBloqueMB, int nbloque, unsigned char buffAUX[BLOCKSIZE]".
// Por parametros salida "nbloque" caso SUCCESS "-1" caso FAILURE.
int reservar_bloque()
{
    struct superbloque SB;
    // Buscamos los bloques libres
    if (bread(posSB, &SB) < 0)
    {
        perror("Error en el bread() en reservar_bloque\n");
        return -1;
    }
    // Comprobamos la variable del superbloque que nos indica si quedan bloques libres.
    if (!SB.cantBloquesLibres)
    {
        perror("No hay bloques libres\n");
        return -1;
    }
    // Si aún quedan, hemos de localizar el 1er bloque libre del dispositivo virtual 
    // consultando cuál es el primer bit a 0 en el MB
    // Buscamos el primer bloque, byte y bit libre
    unsigned char bufferMB[BLOCKSIZE];
    int bloque = -1, posbyte = -1, posbit = 0;
    // Recorremos los bloques del MB (iterando con posBloqueMB) y los iremos cargando en bufferMB
    for (int posBloqueMB = SB.posPrimerBloqueMB; posBloqueMB <= SB.posUltimoBloqueMB && bloque == -1; posBloqueMB++)
    {
        // bread(posBloqueMB,bufferMB)
        if (bread(posBloqueMB, bufferMB) < 0)
        {
            perror("Error en el bread() en reservar_bloque\n");
            return -1;
        }
        for (int j = 0; j < BLOCKSIZE && bloque == -1; j++)
        {
            // Bucle para encontrar el byte en el que se encuentra el bit libre
            if (bufferMB[j] != 255)
            {
                bloque = posBloqueMB;
                posbyte = j;
                // Cálculo del primer bit a 0
                unsigned char mascara = 128; //10000000
                while (bufferMB[posbyte] & mascara)
                {
                    posbit++;
                    mascara >>= 1; // desplaz. de bits a la derecha
                }
            }
        }
    }
    if (bloque == -1)
    {
        perror("Error al reservar el bit\n");
        return -1;
    }
    // Para determinar cuál es finalmente el nº de bloque (nbloque) 
    // que podemos reservar (posición absoluta del dispositivo)
    // nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE+ posbyte) * 8 + posbit;
    int nbloque = ((bloque - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    // Utilizamos la función escribit_bit() pasándole como parámetro ese nº de bloque y un 1 
    // para indicar que el bloque está reservado
    if (escribir_bit(nbloque, 1) == -1)
    {
        perror("Error al escribir_bit en reservar_bloque\n");
        return -1;
    }
    // Decremento en cantidad de bloques libres
    SB.cantBloquesLibres--;
    // Actualizamos SB
    if (bwrite(posSB, &SB) == -1)
    {
        perror("Error bwrite en reservar_bloque\n");
        return -1;
    }
    // Buffer de zeros en nbloque por si había basura
    unsigned char buffAUX[BLOCKSIZE];
    memset(buffAUX, 0, BLOCKSIZE);
    if (bwrite(nbloque, buffAUX) == -1)
    {
        perror("Error bwrite en reservar_bloque\n");
        return -1;
    }
    return nbloque;
}

// Funcion que nos libera un bloque determinado poniendo a 0 en el mapa de bits el bloque correspondiente y actualizamos la cantidad de bloques libres en el superbloque. 
// No limpiamos el bloque en la zona de datos; se queda basura pero se interpreta como espacio libre. Finalmente devolvemos el bloque liberado.
// Por parametros entrada "unsigned int nbloque".
// Variables usadas en la funcion "struct superbloque SB".
// Por parametros salida "nbloque" caso SUCCESS "-1" caso FAILURE.
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en liberar_bloque -> bread");
        return -1;
    }
    // Ponemos a 0 el bit del MB correspondiente al bloque nbloque
    if (escribir_bit(nbloque, 0)==-1)
    {
        perror("Error en escribir_bit dentro de liberar_bloque\n");
        return -1;
    }
    // Incrementamos la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres++;
    // Actualizamos el superbloque
    if (bwrite(posSB, &SB) == -1) return -1;
    return nbloque;
}

// Funcion para escribir el contenido de una variable del tipo INODO en un determinado inodo del array de inodos.
// Por parametros entrada "unsigned int nbloque".
// Variables usadas en la funcion "struct superbloque SB, struct inodo inodos[BLOCKSIZE / INODOSIZE]".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{ 
    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en escribir_inodo -> bread");
        return -1;
    }
    // Empleamos un array de inodos, del tamaño de la cantidad de inodos que caben en un bloque
    // struct inodo inodos[BLOCKSIZE/INODOSIZE]
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // Nos colocamos en el nº de bloque del array de inodos y lo obtenemos 
    if (bread(SB.posPrimerBloqueAI + ninodo / (BLOCKSIZE/INODOSIZE) , inodos) < 0)
    {
        perror("Error bread en escribir_inodo\n");
        return -1;
    }
    // Escribimos en el lugar correspondiente del array
    // ninodo%(BLOCKSIZE/INODOSIZE)
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = inodo;
    // Escribimos el inodo en el bloque que corresponde
    if (bwrite(SB.posPrimerBloqueAI + ninodo / (BLOCKSIZE/INODOSIZE), inodos) < 0)
    {
        perror("Error bwirte en escribir_inodo\n");
        return -1;
    }
    return 0;
}

// Funcion que nos lee un determinado inodo del array de inodos para volcarlo en una variable de tipo INODO.
// Por parametros entrada "unsigned int ninodo, struct inodo *inodo".
// Variables usadas en la funcion "struct superbloque SB, struct inodo inodos[BLOCKSIZE / INODOSIZE]".
// Por parametros salida "0" caso SUCCESS "-1" caso FAILURE.
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    // Leemos el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en leer_inodo -> bread");
        return -1;
    }
    // Empleamos un array de inodos, del tamaño de la cantidad de inodos
    // struct inodo inodos[BLOCKSIZE/INODOSIZE]
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    // Obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado
    if (bread(SB.posPrimerBloqueAI + ninodo / (BLOCKSIZE/INODOSIZE), inodos) < 0)
    {
        perror("Error bread en escribir_inodo\n");
        return -1;
    }
    // El inodo solicitado está en la posición ninodo%(BLOCKSIZE/INODOSIZE)
    *inodo = inodos[ninodo % (BLOCKSIZE/INODOSIZE)];
    // Si ha ido todo bien devolvemos 0
    return 0;
}

// Encuentra el primer inodo libre, lo reserva con la funcion escribir inodo y devuelve su numero.
// Por parametros entrada "unsigned char tipo, unsigned char permisos".
// Variables usadas en la funcion "struct superbloque SB, struct inodo inodo, int posInodoReservado".
// Por parametros salida "posInodoReservado" caso SUCCESS "-1" caso FAILURE.
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{ 
    struct superbloque SB;
    // Leemos el Superbloque
    if (bread(posSB, &SB) == -1)
    {
        perror("Error en reservar_inodo -> bread");
        return -1;
    }
    // Comprobar si hay inodos libres y si no hay inodos libres indicar error y salir
    if (SB.cantInodosLibres == 0)
    {
        perror("No hay inodos libres");
        return -1;
    }
    struct inodo inodo;
    // Leemos el primer inodo libre para luego poder inicializarlo y posteriormente guardarlo
    if (leer_inodo(SB.posPrimerInodoLibre, &inodo) < 0)
    {
        perror("Error al leer_inodo en reservar_inodo");
        return -1;
    }
    // Guardamos la información de la posición y del primer inodo libre
    int posInodoReservado = SB.posPrimerInodoLibre;
    // Primeramente actualizar la lista enlazada de inodos libres 
    // de tal manera que el superbloque apunte al siguiente de la lista
    // Actualizamos el nuevo PrimerInodoLibre
    SB.posPrimerInodoLibre = inodo.punterosDirectos[0];
    // Actualizar la cantidad de inodos libres
    SB.cantInodosLibres --;
    bwrite(posSB, &SB);
    // Inicializamos todos los campos del inodo al que apuntaba inicialmente el superbloque
    // tipo (pasado como argumento)
    inodo.tipo = tipo;
    // permisos (pasados como argumento)
    inodo.permisos = permisos;
    // cantidad de enlaces de entradas en directorio: 1
    inodo.nlinks = 1;
    // tamaño en bytes lógicos: 0
    inodo.tamEnBytesLog = 0;
    // timestamp de creación para todos los campos de fecha y hora: time(NULL)
    inodo.atime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    // cantidad de bloques ocupados en la zona de datos: 0
    inodo.numBloquesOcupados = 0;
    // punteros a bloques directos: 0 (el valor 0 indica que no apuntan a nada)
    for (int i = 0; i < 12; i++)
    {
        inodo.punterosDirectos[i] = 0;
    }
    // punteros a bloques indirectos. 0 (el valor 0 indica que no apuntan a nada)
    for (int i = 0; i < 3; i++)
    {
        inodo.punterosIndirectos[i] = 0;
    }
    // Escribir el inodo inicializado en la posición del que era el primer inodo libre
    if (escribir_inodo(posInodoReservado, inodo) < 0)
    {
        perror("Error al escribir_inodo en reservar_inodo\n");
        return -1;
    }
    // Devolver posInodoReservado
    return posInodoReservado;
}

//      NIVEL 4

// Función que se encarga de obtener el nº  de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
// Por parametros entrada "unsigned int ninodo, unsigned int nblogico, char reservar".
// Variables usadas en la funcion "struct inodo inodo, unsigned int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice, unsigned int buffer[NPUNTEROS]".
// Por parametros salida "ptr" caso SUCCESS "-1" caso FAILURE.
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar)
{
    // var
    // inodo: estructura inodo
    struct inodo inodo;
    //  ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice: ent
    unsigned int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    // buffer[NPUNTEROS]: ent
    unsigned int buffer[NPUNTEROS];
    // fvar
    leer_inodo(ninodo, &inodo); // leer_inodo (ninodo, &inodo)
    // ptr := 0, ptr_ant := 0, salvar_inodo := 0
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;
    // nRangoBL := obtener_nRangoBL(inodo, nblogico, &ptr);
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    // nivel_punteros := nRangoBL
    nivel_punteros = nRangoBL;                          // El nivel_punteros +alto es el que cuelga del inodo
    // mientras nivel_punteros>0 hacer
    while (nivel_punteros > 0)
    { // Iterar para cada nivel de indirectos)
        // si ptr=0 entonces
        if (ptr == 0)
        {
            if (reservar == 0) // si reservar=0 entonces devolver -1
            {
                return -1; // Error lectura bloque inexistente
            }
            else // si_no
            {
                // salvar_inodo := 1
                salvar_inodo = 1;
                // ptr := reservar_bloque()
                ptr = reservar_bloque(); // De punteros
                // inodo.numBloquesOcupados++
                inodo.numBloquesOcupados++;
                // inodo.ctime = time(NULL)
                inodo.ctime = time(NULL); // Fecha actual
                // Reservar bloques punteros y crear enlaces desde inodo hasta datos
                // si nivel_punteros = nRangoBL entonces
                if (nivel_punteros == nRangoBL)
                {                                                 // El bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr; // (imprimirlo)
                    //fprintf(stderr,"[traducir_bloque_inodo() -> inodo.punterosIndirectos[%d] = %d]\n", nRangoBL-1, ptr);
                }
                else // si_no
                {
                    // buffer[indice] := ptr
                    buffer[indice] = ptr; // (imprimirlo)
                    //fprintf(stderr,"[traducir_bloque_inodo() -> punteros_nivel%d[%d] = %d]\n", nivel_punteros+1, indice, ptr);
                    // bwrite(ptr_ant, buffer)
                    bwrite(ptr_ant, buffer);
                } // El bloque cuelga de otro bloque de punteros
            }     // No cuelgan bloques de punteros
        }
        // bread(ptr, buffer)
        bread(ptr, buffer);
        // indice := obtener_indice(nblogico, nivel_punteros)
        indice = obtener_indice(nblogico, nivel_punteros);
        // ptr_ant := ptr
        ptr_ant = ptr; // Guardamos el puntero
        // ptr := buffer[indice]
        ptr = buffer[indice];
        // Y lo desplazamos al siguiente nivel
        // nivel_punteros--
        nivel_punteros--;
    } // Al salir de este bucle ya estamos al nivel de datos

    // si ptr=0
    if (ptr == 0)
    { // No existe bloque de datos
        // si reservar=0 entonces devolver -1
        if (reservar == 0)
            return -1; // Error lectura ∄ bloque
        else // si_no
        {
            // salvar_inodo := 1
            salvar_inodo = 1;
            // ptr = reservar_bloque()
            ptr = reservar_bloque(); // De datos
            // inodo.numBloquesOcupados++
            inodo.numBloquesOcupados++;
            // inodo.ctime = time(NULL)
            inodo.ctime = time(NULL);
            // si nRangoBL=0 entonces
            if (nRangoBL == 0)
            {
                // inodo.punterosDirectos[nblogico] := ptr
                inodo.punterosDirectos[nblogico] = ptr; // (imprimirlo)
                //fprintf(stderr,"[traducir_bloque_inodo() -> inodo.punterosDirectos[%d] = %d]\n", nblogico, ptr);
            }
            else // si_no
            {
                // buffer[indice] := ptr
                buffer[indice] = ptr; // (imprimirlo)
                //fprintf(stderr,"[traducir_bloque_inodo() -> punteros_nivel%d[%d] = %d]\n", nivel_punteros+1, indice, ptr);
                // bwrite(ptr_ant, buffer)
                bwrite(ptr_ant, buffer);
            }
        }
    }

    // si salvar_inodo=1 entonces
    if (salvar_inodo == 1)
    {
        // escribir_inodo(ninodo, inodo)
        escribir_inodo(ninodo, inodo); // Sólo si lo hemos actualizado
    }
    return ptr; // devolver ptr
}

// Función que se encarga de la obtención de los índices de los bloques de punteros.
// (%) numero de puntero apuntado por (/) numero de puntero apuntado por punterosIndirectos[] 
// Por parametros entrada "unsigned int nblogico, unsigned int nivel_punteros".
// Variables usadas en la funcion " ".
// Por parametros salida "nblogico" caso SUCCESS "-1" caso FAILURE.
int obtener_indice(unsigned int nblogico, unsigned int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    {
        return nblogico; //ej nblogico=8
    }
    else
    {
        if (nblogico < INDIRECTOS0)
        {
            return (nblogico - DIRECTOS); //ej nblogico=204
        }
        else
        {
            if (nblogico < INDIRECTOS1) //ej nblogico=30.004
            {
                if (nivel_punteros == 2)
                {
                    return ((nblogico - INDIRECTOS0) / NPUNTEROS);
                }
                else
                {
                    if (nivel_punteros == 1)
                    {
                        return ((nblogico - INDIRECTOS0) % NPUNTEROS);
                    }
                }
            }
            else
            {
                if (nblogico < INDIRECTOS2) //ej nblogico=400.004
                {
                    if (nivel_punteros == 3)
                    {
                        return ((nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS));
                    }
                    else
                    {
                        if (nivel_punteros == 2)
                        {
                            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS);
                        }
                        else
                        {
                            if (nivel_punteros == 1)
                            {
                                return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS);
                            }
                        }
                    }
                }
                else
                {
                    perror("Error en la obtención del índice");
                    return -1;
                }
            }
        }
    }
    return 0;
}

// Función para obtener el rango de punteros en el que se sitúa el bloque lógico que buscamos y la dirección almacenada en el puntero del inodo.
// Por parametros entrada "struct inodo inodo, unsigned int nblogico, unsigned int *ptr".
// Variables usadas en la funcion " ".
// Por parametros salida "nRangoBL" caso SUCCESS "-1" caso FAILURE.
int obtener_nRangoBL(struct inodo inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo.punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo.punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo.punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo.punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        perror("Bloque lógico fuera de rango");
        return -1;
    }
}

//      NIVEL 6

// Función que aumenta el numero de inodos libres y libera todo aquello que apunte a esos bloques.
// Por parametros entrada "unsigned int ninodo".
// Variables usadas en la funcion "struct superbloque SB, struct inodo inodo, int bloqlib".
// Por parametros salida "ninodo" caso SUCCESS "-1" caso FAILURE.
int liberar_inodo(unsigned int ninodo)
{
    struct superbloque SB;
    struct inodo inodo;
    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) < 0)
    {
        perror("Error leer_inodo en liberar_inodo");
        return -1;
    }
    // Llamamos a la función auxiliar para liberar todos los bloques del inodo
    // El argumento primerBL que le pasamos, valdrá 0 cuando la llamamos desde esta función, 
    // ya que si liberamos el inodo hemos de liberar también TODOS los bloques ocupados
    int bloqlib = liberar_bloques_inodo(0, &inodo);
    if(bloqlib < 0){
        perror("Error al liberar_bloques_inodo en liberar_inodo");
        return -1;
    }     
    // Se actualiza la cantidad de bloques ocupados
    inodo.numBloquesOcupados -= bloqlib;
    // Marcamos el inodo como liberado y tamEnBytesLog = 0
    inodo.tipo = 'l'; 
    inodo.tamEnBytesLog = 0;
    // Leemos el superbloque para saber cuál es el primer inodo libre
    if (bread(posSB, &SB) < 0)
    {
        perror("Error al leer el superbloque en liberar_inodo");
        return -1;
    }
    // Incluimos el inodo a la lista de inodos libres
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre; 
    SB.posPrimerInodoLibre = ninodo;
    // Incrementamos la cantidad de inodos libres
    SB.cantInodosLibres++;
    // Escribimos el inodo
    if (escribir_inodo(ninodo, inodo) < 0)
    {
        perror("Error leer_inodo en liberar_inodo");
        return -1;
    }
    // Escribimos el superbloque
    if (bwrite(posSB, &SB) < 0)
    {
        perror("Error al escribir en el superbloque en liberar_inodo");
        return -1;
    }
    // Devolvemos el nº del inodo liberado
    return ninodo;
}

// Función que libera todos los bloques ocupados con ayuda de liberar_bloque().
// Por parametros entrada "unsigned int primerBL, struct inodo *inodo".
// Variables usadas en la funcion "unsigned int nRangoBL, nivel_punteros, indice, ptr, nBL, ultimoBL, int bloques_punteros[3][NPUNTEROS], int ptr_nivel[3], int indices[3], int liberados, unsigned char buffAux_punteros[BLOCKSIZE]".
// Por parametros salida "liberados" caso SUCCESS "-1" caso FAILURE.
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    // var
    unsigned int nRangoBL, nivel_punteros, indice, ptr, nBL, ultimoBL;
    int bloques_punteros[3][NPUNTEROS]; // Array de bloques de punteros
    int ptr_nivel[3];                   // Punteros a bloques de punteros de cada nivel
    int indices[3];                     // Indices de cada nivel
    int liberados = 0;                  // Nº de bloques liberados
    unsigned char buffAux_punteros[BLOCKSIZE]; // 1024 bytes
    // fvar

    // Hay que comprobar cuando pasemos por un bloque de punteros, si no le quedan ya punteros ocupados, 
    // puesto que en tal caso también habría que liberar ese bloque de punteros. Para eso usamos buffAux.
    memset(buffAux_punteros, 0, BLOCKSIZE);

    // si inodo->tamEnBytesLog = 0 entonces devolver 0 fsi
    if (inodo->tamEnBytesLog == 0)
    {
        return 0; 
        // El fichero vacío
        // Obtenemos el último bloque lógico del inodo
    }

    // Si inodo->tamEnBytesLog % BLOCKSIZE = 0 entonces
    if ((inodo->tamEnBytesLog % BLOCKSIZE) == 0)
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1; // ultimoBL := inodo->tamEnBytesLog / BLOCKSIZE - 1
    }
    else // Si_no ultimoBL := inodo->tamEnBytesLog / BLOCKSIZE
    {
        ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE);
    }
    //fprintf(stderr, "\n[liberar_bloques_inodo() -> primer BL: %d, ultimo BL: %d]\n",primerBL, ultimoBL);

    ptr = 0; // ptr:= 0
    //  Para nBL := primerBL hasta nBL = ultimoBL paso 1 hacer
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    { // Recorrido BLs
        // nRangoBL := obtener_nrangoBL(*inodo, nBL, &ptr)
        nRangoBL = obtener_nRangoBL(*inodo, nBL, &ptr);  // 0:D, 1:I0, 2:I1, 3:I2
        // Si nRangoBL < 0 entonces devolver ERROR fsi
        if (nRangoBL < 0)
        {
            perror("nRangoBL en liberar_bloques_inodo");
            return -1;
        }
        // nivel_punteros := nRangoBL
        nivel_punteros = nRangoBL; // El nivel_punteros +alto cuelga del inodo

        //  Mientras (ptr > 0 && nivel_punteros > 0) hacer
        while (ptr > 0 && nivel_punteros > 0)
        { // Cuelgan bloques de punteros
            // indice := obtener_indice(nBL, nivel_punteros)
            indice = obtener_indice(nBL, nivel_punteros);
            // Si indice=0  o nBL=primerBL entonces
            if (indice == 0 || nBL == primerBL){
                // Solo leemos del dispositivo si no está ya cargado en un buffer
                bread(ptr, bloques_punteros[nivel_punteros - 1]);
            }
            // ptr_nivel[nivel_punteros-1] := ptr
            ptr_nivel[nivel_punteros - 1] = ptr;
            // indices[nivel_punteros-1] := indice
            indices[nivel_punteros - 1] = indice;
            // ptr := bloques_punteros[nivel_punteros-1][indice]
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            // nivel_punteros--
            nivel_punteros--;
        }

        // si ptr > 0 entonces
        if (ptr > 0)
        { // Si existe bloque de datos
            //fprintf(stderr, "[liberar_bloques_inodo() -> liberado BF %d de datos para BL %d]\n",ptr,nBL);
            liberar_bloque(ptr);
            // liberados++
            liberados++;
            // Si nRangoBL = 0 entonces
            if (nRangoBL == 0)
            { // Es un puntero Directo
                // inodo->punterosDirectos[nBL]:= 0
                inodo->punterosDirectos[nBL] = 0;
                //printf("[liberar_bloques_inodo() -> liberado BF %d de punteros_nivel %d correspondiente al BL %d]\n",ptr, nivel_punteros, nBL);
            }
            else
            {
                // Mientras nivel_punteros < nRangoBL hacer
                while (nivel_punteros < nRangoBL)
                { 
                    // indice := indices[nivel_punteros]
                    indice = indices[nivel_punteros];
                    // bloques_punteros[nivel_punteros][indice] := 0
                    bloques_punteros[nivel_punteros][indice] = 0;
                    // ptr := ptr_nivel [nivel_punteros]
                    ptr = ptr_nivel[nivel_punteros];
                    // si bloques_punteros[nivel_punteros] = 0  entonces
                    if(memcmp(bloques_punteros[nivel_punteros], buffAux_punteros,BLOCKSIZE)== 0)
                    {
                        // No cuelgan bloques ocupados, hay que liberar el bloque de punteros
                        //fprintf(stderr, "[liberar_bloques_inodo() -> liberado BF %d de datos para BL %d]\n",ptr,nBL);
                        liberar_bloque(ptr);
                        liberados++;
                        nivel_punteros++;
                        // Si nivel_punteros = nRangoBL entonces
                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0; // inodo->punterosIndirectos[nRangoBL-1] := 0
                            //printf("[liberar_bloques_inodo() -> liberado BF %d de punteros_nivel %d correspondiente al BL %d]\n",ptr, nivel_punteros, nBL);
                        }
                    }
                    else
                    { // Escribimos en el dispositivo el bloque de punteros modificado
                        bwrite(ptr, bloques_punteros[nivel_punteros]);
                        nivel_punteros = nRangoBL; // Para salir del bucle ya que no habrá que liberar los bloques de niveles
                    }
                }
            }
        }
    }
    //fprintf(stderr, "[liberar_bloques_inodo() -> total bloques liberados: %d]\n",liberados);
    return liberados;
}