#include "cliente.h"

int main(){

	PIDCliente = getpid();
	socketEscucha = 0;

	//Cargo las configuraciones del .config
	config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/cliente.config");
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH"); //cargo el path del archivo log
	ip_destino = config_get_string_value(config,"IP_DESTINO");
	puerto_destino = config_get_string_value(config,"PUERTO_DESTINO");
	puerto_local = config_get_string_value(config,"PUERTO_LOCAL");
	miPosicionX = config_get_int_value(config, "POSICION_X");
	miPosicionY = config_get_int_value(config, "POSICION_Y");

	//Dejo cargado un logger para loguear los eventos.
	logger = cargarUnLog(LOG_PATH, "Cliente");

	//ToDo esto esta para que corten/peguen donde sea necesario, asi no pierden tiempo viendo como funcionan estas cosas
	//FUNCIONES DE ESTABLECER CONEXIONES, USAR COMO Y CUANDO HAGAN FALTA********************************************************************
//	socketComanda = establecer_conexion(ip_destino, puerto_destino);
//	resultado_de_conexion(socketComanda, logger, "CoMAnda");

//	socketApp = establecer_conexion(app_IP, app_puerto);
//	resultado_de_conexion(socketApp, logger, "App");
//
//	socketSindicato = establecer_conexion(sindicato_IP, sindicato_puerto);
//	resultado_de_conexion(socketSindicato, logger, "Sindicato");
//
//	socketRestaurante = establecer_conexion(restaurante_IP, restaurante_puerto);
//	resultado_de_conexion(socketRestaurante, logger, "Restaurante");

//	close(socketComanda);
//	close(socketApp);
//	close(socketSindicato);
//	close(socketRestaurante);
	//*************************************************************************************************************************************

	//ToDo levantar hilos para conexiones a Comanda y Sindicato???

	//ToDo levantamos socket para recibir mensajes (hilo)

	//ToDo hay que levantarlo como hilo, y agregar toda la parte del accept, recibir mensaje y el manejo del mensaje recibido


	//Preparar consola que se mantendra activa hasta la terminacion del proceso (hilo)  WIP

	t_conexion tuplaConexion;
    tuplaConexion.ip_destino = ip_destino;
    tuplaConexion.puerto_destino = puerto_destino;


	while(1){

		pthread_create(&hiloConsola, NULL,(void*)obtenerInputConsolaCliente, &tuplaConexion);
		pthread_join(hiloConsola, NULL);

	}










	//levantar sockets cada vez que se quiere mandar un mensaje




	//para cerrar el programa
	matarPrograma(logger, config, socketEscucha);

	return EXIT_SUCCESS;
}
