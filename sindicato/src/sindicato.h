/*
 * sindicato.h
 *
 *  Created on: 30 ago. 2020
 *      Author: utnso
 */

#ifndef SRC_SINDICATO_H_
#define SRC_SINDICATO_H_

// Para que no me salte la warning de asprintf()
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <sys/stat.h> // Para tener mkdir
#include <semaphore.h> // para los semaforos
#include <dirent.h> // Para poder escanear carpetas
#include <errno.h> // Para checkear el error al escanear una carpeta
#include <pthread.h> // Para los hilos
#include "shared/estructuras.h"
#include "shared/cargador.h"
#include "shared/utilidades.h"
#include "commons/string.h"
#include "commons/bitarray.h"
#include "commons/collections/list.h"

// Header files mios
#include "consola.h"
#include "iniciarFS.h"
#include "bitarray.h"

typedef struct{
	int cantCocineros;
	char* posicion;
	char* afinidad;
	char* platos;
	char* preciosPlatos;
	int cantHornos;
}datosRestaurant;

typedef struct{
	char* pasos;
	char* tiempoPasos;
}datosReceta;

char* PUERTO_ESCUCHA;

char* pathMetadata;
char* pathRestaurantes;
char* pathRecetas;
char* pathBloques;
char* pathFiles;

int BLOCKS;
int BLOCK_SIZE;
char* MAGIC_NUMBER;

t_log* logger;

// Arrays
int cantidadDeElementosEnArray(char**);
void freeDeArray(char**);

// Restaurantes
void crearRestaurant(char*, datosRestaurant);
char* crearCarpetaRestaurant(char*);
char* generarSringInfoRestaurant(datosRestaurant hola);
int existeRestaurant(char*);

char* generarPathInfoRestaurant(char*);
char* leerBloquesRestaurant(int, int);

// Recetas
void crearReceta(char*, datosReceta unaReceta);
char* generarStringInfoReceta(datosReceta unaReceta);
int existeReceta(char*);

// Bloques/Archivos
void escribirDatoEnBloque(char*, int, int);
void escribirLineasEnBloques(t_list*, t_list*);
char* crearArchivoVacioEn(char*, char*);
void llenarBloquesConString(char*, char*, char*);
t_list* separarStringEnBloques(char*, int);
void fijarValorArchivoA(char*, int, char*);
void loguearAsignacionBloques(char*, t_list*);
char* generarPathABloque(int numeroBloque);


#endif /* SRC_SINDICATO_H_ */
