#include "terminarPrograma.h"

void matarPrograma(t_log* logger, t_config* config, int32_t conexion)
{
	config_destroy(config);
	puts("Puntero a archivo .config destruido.");
	log_destroy(logger);
	puts("Puntero al logger destruido.");
	cerrar_conexion(conexion);
	puts("Conexion terminada.");
}
