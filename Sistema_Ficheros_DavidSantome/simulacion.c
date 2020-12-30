//                                                        SO2: SISTEMA DE FICHEROS BÁSICO

#include "simulacion.h"
#include <stdio.h>


struct tm *tm;
time_t tiempo;
static char dir [100];
static char dir1[100];
static int acabados = 0;

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        acabados++;
        // Podemos testear qué procesos van acabando:
        //fprintf(stderr, "Acabado Proceso con PID: %d, total acabados: %d\n", ended, acabados);
    }
}

int main(int argc, char const *argv[])
{
    // Comprobar sintaxis
    if(argc==2){
        printf("**Simulacion de 100 procesos realizando cada uno 50 escrituras**\n");
        acabados=0;
        // Montar disco.
        bmount(argv[1]); // Proceso padre
        time(&tiempo);
        tm = localtime(&tiempo);
        char tmp [100];
        // Crear el directorio de simulación en la raíz: /simul_aaaammddhhmmss/
        sprintf(tmp, "%d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(dir,"/simul_");
        strcat(dir,tmp);
        strcat(dir, "/");
        if(mi_creat(dir,6)<0){
            bumount(); // (en caso de error desmontar el dispositivo).
            perror("Error al montar el disco padre");
            return -1;
        }
        printf("Directorio simulación: %s\n", dir);
        // Asignamos la función enterrador  a la señal de finalización de un hijo, SIGCHLD, la cual eliminará los hijos zombie
        signal(SIGCHLD, reaper);
        // Para proceso:=1 hasta proceso <=100 hacer
        for(int pro = 1; pro<=NUMPROCESOS; pro++){
            // Crear un proceso con fork().
            if(fork() == 0){ // se trata del hijo
                // Para pasar el PID a un string creamos una variable spid a la que le copiaremos el PID
                char spid[10];
                int pid = getpid();
                sprintf(spid,"%d",pid);
                // Montar disco. //proceso hijo
                bmount(argv[1]);
                // Crear el directorio del proceso, añadiendo su pid al nombre, dentro del directorio de simulación
                strcpy(dir1,dir);
                strcat(dir1,"proceso_");
                strcat(dir1,spid);
                strcat(dir1, "/");
                if(mi_creat(dir1,6)<0){
                    bumount(); // (en caso de error desmontar el dispositivo).
                    perror("Error al montar el disco hijo");
                    return -1;
                }
                
                // Crear el fichero prueba.dat dentro del directorio del proceso
                strcat(dir1,"prueba.dat");
                if(mi_creat(dir1,6)<0){
                    bumount(); // (en caso de error desmontar el dispositivo)
                    return -1;
                }
                // Inicializar la semilla para los números aleatorios
                srand(time(NULL)+getpid());

                // Para cada una de las 50 operaciones de escritura i (i inicialmente=0) hacer
                for(int i =0; i<NUMESCRITURAS;i++){
                    // Inicializar el registro
                    struct REGISTRO registro;
                    // registro.fecha=time(NULL)
                    registro.fecha=time(NULL); 
                    // registro.pid = getpid()
                    registro.pid = getpid(); 
                    // registro.nEscritura = i+1
                    registro.nEscritura = i+1; 
                    // registro.nRegistro =  rand() % REGMAX
                    registro.nRegistro =  rand() % REGMAX;  
                    // Escribir el registro con mi_write() en registro.nRegistro * sizeof(struct registro)
                    if(mi_write(dir1,&registro,registro.nRegistro * sizeof(struct REGISTRO),sizeof(struct REGISTRO))<0){
                        perror("Error al hacer mi_write en simulacion");
                        return -1;
                    }
                    //fprintf(stderr,"[simulación.c -> Escritura %d en %s\n", i, dir1);
                    usleep(50000); // Esperar 0'5 seg para hacer la siguiente escritura.
                }
                //printf("Proceso %d: Completadas 50 escrituras en %s\n",pro,dir1);
                // Desmontar disco. //proceso hijo
                bumount();
                // Necesario para que se emita la señal SIGCHLD
                exit(0);
            }
            usleep(20000); // Esperar 0'2 seg para lanzar otro proceso.
        }
    }else{
        printf("Sintaxis : ./simulacion <disco>\n");
        return -1;
    }

    // Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS){
        pause();
    }
    //fprintf(stderr,"Total de procesos terminados: %d\n",acabados);
    bumount(); // Proceso padre
    return 0; // o return 0
}
      

  

