#include "cliente.h"

int main(){

	PIDCliente = getpid();
	socketEscucha = 0;

	//Cargo las configuraciones del .config
	config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/cliente.config");
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH"); //cargo el path del archivo log
	//Dejo cargado un logger para loguear los eventos.
	logger = cargarUnLog(LOG_PATH, "Cliente");


	//levantar hilos para conexiones a Comanda y Sindicato

	//un socket para recibir mensajes (hilo)
	//levantar sockets cada vez que quiere mandar un mensaje

printf("ejecute\n");

	return EXIT_SUCCESS;
}
