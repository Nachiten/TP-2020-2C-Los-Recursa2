#ifndef SRC_FUNCIONESCLIENTE_H_
#define SRC_FUNCIONESCLIENTE_H_

#include "shared/estructuras.h"
#include "shared/cargador.h"
#include "shared/utilidades.h"
#include "shared/socket.h"

char* ipConexion;
char* puerto_local;
char* puerto_destino;


typedef struct{
	char* ip_destino;
	char* puerto_destino;
	t_log* mi_logger;
	//ignorar por ahora
	//char* mi_puerto; //para poder levantarme el socket de escucha
}t_conexion;


//firmas de funciones
uint32_t valor_para_switch_case(char* input);
int cantidadDeElementosEnArray(char** array);
void freeDeArray(char** array);




#endif /* SRC_FUNCIONESCLIENTE_H_ */
