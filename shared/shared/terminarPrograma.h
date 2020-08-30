#ifndef SHARED_TERMINARPROGRAMA_H_
#define SHARED_TERMINARPROGRAMA_H_

#include"commons/log.h"
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include"socket.h"

//Usar para liberar recursos.
void matarPrograma(t_log* logger, t_config* config, int32_t conexion);


#endif /* SHARED_TERMINARPROGRAMA_H_ */
