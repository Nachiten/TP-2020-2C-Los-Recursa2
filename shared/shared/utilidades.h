#ifndef SHARED_UTILIDADES_H_
#define SHARED_UTILIDADES_H_

#include<stdio.h>
#include<stdlib.h>
#include<readline/readline.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<commons/collections/list.h>
#include"estructuras.h"
#include"socket.h"

//las proximas 2 funciones son para transformar de STRING a INT----------------

//eleva el 1er numero que recibe al 2do que recibe
uint32_t elevar_a(uint32_t a, uint32_t b);
//recibe un string y lo transforma en int (mandar solo numeros, no sean forros)
uint32_t cambia_a_int(char* enString);

//-----------------------------------------------------------------------------

//muestra por pantalla los bytes recibidos de RECV
void bytesRecibidos (int32_t cantidad);
//muestra por pantalla los bytes enviados de SEND
void bytesEnviados (int32_t cantidad);
// Hace free de cada uno de los elementos y de la lista misma
void destruirListaYElementos(t_list* unaLista);

//esta porqueria "traduce" el numero de OK/FAIL para ahorrarnos los if si queremos mostrar por pantalla
char* resultadoDeRespuesta(uint32_t resultado);

// Cuenta la cantidad de elementos en un array
int cantidadDeElementosEnArray(char** array);

//para evitar la repeticion constante de los recv del codigo de operacion y del tamaño a alocar, Exito = 0 o 1 (utilizar para validaciones)
//USAR *SOLO* si ya estan dentro del case y esperan la respuesta de un mensaje enviado
void los_recv_repetitivos(int32_t socket_conexion_establecida, uint32_t *exito, int32_t *sizeAAllocar);

#endif /* SHARED_UTILIDADES_H_ */
