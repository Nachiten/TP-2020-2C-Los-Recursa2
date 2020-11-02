#ifndef SHARED_CARGADOR_H_
#define SHARED_CARGADOR_H_

#include <inttypes.h>
#include"commons/log.h"
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

//Usar para dejar cargadas las cosas que necesitamos.

t_log* cargarUnLog(char*, char*); //tirarle la ruta del log y el nombre del log
t_config* leerConfiguracion(char*); //tirarle la ruta del config
t_log* cargarUnLogApp(char*, char*); // Cargar un log para app

#endif /* SHARED_CARGADOR_H_ */
