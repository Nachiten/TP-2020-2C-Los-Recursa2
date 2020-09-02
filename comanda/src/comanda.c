#include "comanda.h"

int main(){
	PIDCoMAnda = getpid();
	socketEscucha = 0;
	printf("CoMAnda PID: %u.\n", PIDCoMAnda);
	MEMORIA_PRINCIPAL = NULL;
	AREA_DE_SWAP = NULL;

	//Cargo las configuraciones del .config
	config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/comanda.config");
	if(config != NULL)
	{
		puts("Archivo de configuración cargado correctamente.");
	}
	else
	{
		puts("Error al cargar archivo de configuración, abortando...\n");
		abort();
	}

	//cargo el path del archivo log
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH");
	//Dejo cargado un logger para loguear los eventos.
	logger = cargarUnLog(LOG_PATH, "Cliente");
	if(logger != NULL)
	{
		puts("Archivo de LOGS cargado correctamente.");
	}
	else
	{
		puts("Error al cargar archivo de LOGS, abortando...");
		abort();
	}
	//cargo IPs y Puertos...
	cliente_IP = config_get_string_value(config,"IP_CLIENTE");
	app_IP = config_get_string_value(config,"IP_APP");
	mi_puerto = config_get_string_value(config,"MI_PUERTO");
	app_puerto = config_get_string_value(config,"PUERTO_APP");
	cliente_puerto = config_get_string_value(config,"PUERTO_CLIENTE");

	puts("Cargando configuraciones de memoria...");
	puts("****************************************");
	TAMANIO_MEMORIA_PRINCIPAL = config_get_int_value(config,"TAMANIO_MEMORIA");
	printf("Tamaño de memoria principal: %u bytes.\n", TAMANIO_MEMORIA_PRINCIPAL);
	TAMANIO_AREA_DE_SWAP = config_get_int_value(config,"TAMANIO_SWAP");
	printf("Tamaño del área de swapping: %u bytes.\n", TAMANIO_AREA_DE_SWAP);
	FRECUENCIA_COMPACTACION = config_get_int_value(config,"FRECUENCIA_COMPACTACION");
	printf("Frecuencia de compactación: %u.\n", FRECUENCIA_COMPACTACION);
	ALGOR_REEMPLAZO = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	printf("Algoritmo de reemplazo: %s.\n", ALGOR_REEMPLAZO);
	puts("****************************************");

	MEMORIA_PRINCIPAL = malloc(TAMANIO_MEMORIA_PRINCIPAL);
	if(MEMORIA_PRINCIPAL != NULL)
	{
		puts("Memoria Principal inicializada.");
	}
	else
	{
		log_error(logger, "Error al inicializar Memoria Principal.");
		puts("Abortando...");
		abort();
	}
	AREA_DE_SWAP = malloc(TAMANIO_AREA_DE_SWAP);
	if(MEMORIA_PRINCIPAL != NULL)
	{
		puts("Área de SWAP inicializada.");
	}
	else
	{
		log_error(logger, "Error al inicializar el área de SWAP.");
		puts("Abortando...");
		abort();
	}




	//int32_t socketApp;
	//int32_t socketCliente;








	return EXIT_SUCCESS;
}
