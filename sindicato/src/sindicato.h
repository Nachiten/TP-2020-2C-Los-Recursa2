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
#include "shared/estructuras.h"
#include "shared/cargador.h"
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
	char* nombre;
	char* pasos;
	char* tiempoPasos;
}datosPasos;

char* PUERTO_ESCUCHA;

int cantidadDeElementosEnArray(char**);
void freeDeArray(char**);

// Creacion restaurantes
void crearRestaurant(char*, datosRestaurant);
char* crearCarpetaRestaurant(char*);
char* crearArchivoVacioEn(char*, char*);
void llenarBloquesRestaurant(char*, datosRestaurant hola);
char* generarSringInfoRestaurant(datosRestaurant hola);

t_list* separarStringEnBloques(char*, int);

void escribirDatoEnBloque(char*, int, int);
void escribirLineasEnBloques(t_list*, t_list*);

void fijarValorArchivoA(char*, int, char*);

t_list* obtenerPrimerosLibresDeBitmap(int cantidad);

char* pathMetadata;
char* pathRestaurantes;
char* pathRecetas;
char* pathBloques;
char* pathFiles;

int BLOCKS;
int BLOCK_SIZE;
char* MAGIC_NUMBER;

#endif /* SRC_SINDICATO_H_ */
