#ifndef SHARED_UTILIDADES_H_
#define SHARED_UTILIDADES_H_

#include<stdio.h>
#include<stdlib.h>
#include<readline/readline.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include"estructuras.h"

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

#endif /* SHARED_UTILIDADES_H_ */
