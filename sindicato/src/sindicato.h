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
#include "mensajes.h"
#include "semaforos.h"
#include "conexiones.h"
#include "estructuras.h"

char* PUERTO_ESCUCHA;
char* IP_SINDICATO;

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
int encontrarElementoEnArray(char*, char*);
char* generarStringArray(char** elementosArray);

// Restaurantes
char* crearCarpetaRestaurant(char*);
char* generarSringInfoRestaurant(datosRestaurant restaurant);
int existeRestaurant(char*);
char* generarPathInfoRestaurant(char*);
char* generarPathCarpetaRestaurant(char*);
char* leerDatosRestaurant(char*);
int obtenerPrecioPlatoRestaurant(char* nombrestaurant, char*);

// Recetas
char* generarStringInfoReceta(datosReceta unaReceta);
int existeReceta(char*);

// Bloques/Archivos
void escribirDatoEnBloque(char*, int, int);
void escribirLineasEnBloques(t_list*, t_list*);
char* crearArchivoVacioEn(char*, char*);
void llenarBloquesConString(char*, char*, char*);
t_list* separarStringEnBloques(char*, int);
void fijarValorArchivoA(char*, int, char*);
int leerValorArchivo(char*, char*);
void loguearAsignacionBloques(char*, t_list*);
char* generarPathABloque(int);
char* leerDatosBloques(int, int);
int cantidadDeBloquesQueOcupa(int);

// Pedidos
int existePedido(char*, int);
int pedidoEstaEnEstado(char*, char*);
char* generarPathAPedido(char*, int);
char* generarStringPedidoDefault();
t_list* obtenerListaBloquesPedido(char*, int);
char* cambiarEstadoPedidoA(char*, char*);
char* leerDatosPedido(char*, int);

// Platos
char* sumarCantidadAStringPlatos(char* stringPlatos, int cantidad, int indexArray);

// Printear
void printearRespuestaConsultarPlatos(respuesta_consultar_platos*);
void printearRespuestaObtenerPedido(respuesta_obtener_pedido*);

// Otros
char* sumarPrecioTotal(char* precioInicialString, int precioPlato, int cantidadPlato);
char* agregarElementoEnStringArray(char* stringArray, char* nombreElemento);

#endif /* SRC_SINDICATO_H_ */
