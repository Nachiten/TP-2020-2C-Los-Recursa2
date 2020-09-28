#include "app.h"

int main(){

	inicializar_colas();
	inicializar_semaforos();
	id_inicial_pedidos = 0;

	//Cargo las configuraciones del .config
	config = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/app.config");
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
	LOG_PATH = config_get_string_value(config,"ARCHIVO_LOG");
	//Dejo cargado un logger para loguear los eventos.
	logger = cargarUnLog(LOG_PATH, "App");
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
	mi_puerto = config_get_string_value(config,"PUERTO_ESCUCHA");
	puerto_commanda = config_get_string_value(config,"PUERTO_COMANDA");
	ip_commanda = config_get_string_value(config,"IP_COMANDA");

	// cargo datos de restoDefault
	platos_default = config_get_string_value(config,"PLATOS_DEFAULT");
	posX_resto = config_get_int_value(config,"POSICION_REST_DEFAULT_X");
	posY_resto = config_get_int_value(config,"POSICION_REST_DEFAULT_Y");

	//coneccion a commanda
	socket_commanda = establecer_conexion(ip_commanda,puerto_commanda);

	//inicio el server
	iniciar_server(mi_puerto);

	return EXIT_SUCCESS;
}

void inicializar_colas(){
	listaRestos = list_create(); //inicializo la lista de restaurantes
	listaPedidos = list_create(); //inicializo la lista de pedidos
}

void inicializar_semaforos(){
	semId = malloc(sizeof(sem_t));
	semLog = malloc(sizeof(sem_t));

	sem_init(semId, 0, 1);
	sem_init(semLog, 0, 1);
}

// *********************************FIN SETUP*********************************************

//manda un array con los nombres de todos los restaurantes conectados o restoDefault si no hay ninguno
void consultar_restaurantes(int32_t socket_cliente){

	char* stringCompleto = string_new();
	char* stringNuevoParaAgregar;
	char* stringInicial = "[";
	char* stringFinal = "]";
	char* stringSeparador = ",";
	info_resto* resto;

	if(listaRestos->elements_count == 0){
		stringCompleto  = string_new();
		stringNuevoParaAgregar = "RestoDefault";
		string_append(&stringCompleto, stringInicial);// agregas un [ al principio
		string_append(&stringCompleto, stringNuevoParaAgregar);
	}else{
		stringCompleto  = string_new();
		string_append(&stringCompleto, stringInicial);

		for(int i = 0; i < listaRestos->elements_count; i++){
			if(i > 1){
				string_append(&stringCompleto, stringSeparador);// agregas una , si ya hay elementos agregados
			}
			resto = list_get(listaRestos,i);// coseguis el restaurante de la posicion i
			stringNuevoParaAgregar = resto->nombre_resto;
			string_append(&stringCompleto, stringNuevoParaAgregar);
		}

		string_append(&stringCompleto, stringFinal);// agregas un ] al final
		mandar_mensaje(stringCompleto,RESPUESTA_CONSULTAR_R,socket_cliente);
	}
}

/* si el restaurante seleccionado existe en la lista de restaurantes manda un 1 y selecciona ese
 * restaurante para el cliente, en caso de no encontrar el nombre manda un 0 en señal de error
*/
void seleccionarRestaurante(char* nombreResto, int32_t socket_cliente){
	int restoBuscado;
	respuesta_ok_error* respuesta;
	perfil_cliente* perfil;
	int size;

	restoBuscado = buscar_resto(nombreResto);
	respuesta = malloc(sizeof(respuesta_ok_error));

	if(restoBuscado == 1){
		size = sizeof(int32_t) + sizeof(strlen(nombreResto)) + sizeof(uint32_t) + sizeof(int);
		// todo ver si esto esta bien
		perfil = malloc(size);
		perfil->socket_cliente = socket_cliente;
		perfil->nombre_resto = nombreResto;
		perfil->id_pedido = 0;
		perfil->perfilActivo = 1;
		list_add(listaPedidos,perfil);

		respuesta->respuesta = 1;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);
	}else{
		respuesta->respuesta = 0;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);
	}
	free(respuesta);
}

// todo terminar
void consultar_platos(int32_t socket_cliente){
	int numero_cliente, numero_resto;
	perfil_cliente* cliente;
	info_resto* resto;

	numero_cliente = buscar_cliente(socket_cliente);
	if(numero_cliente != -2){
		cliente = list_get(listaPedidos,numero_cliente);// busca el perfil del cliente en la lista de pedidos
		if(cliente->perfilActivo == 1){
			numero_resto = buscar_resto(cliente->nombre_resto);
			if(numero_resto != 0){
				resto = list_get(listaRestos,numero_resto);
				recibir_respuesta(CONSULTAR_PLATOS,resto,cliente);
				return;
			}else{
				//todo terminar la parte de resto default
			}
		}else{
			sem_wait(semLog);
			log_info(logger, "hace falta seleccionar restaurante");
			sem_post(semLog);
		}
	}
}

void crear_pedido(int32_t socket_cliente){
	int numero_resto, numero_cliente;
	perfil_cliente* cliente;
	info_resto* resto;
	guardar_pedido* estructura_guardar_pedido;
	respuesta_ok_error* respuesta;

	numero_cliente = buscar_cliente(socket_cliente);
	if(numero_cliente != -2){
		if(listaRestos->elements_count != 0){
			numero_cliente = buscar_cliente(socket_cliente);
			cliente = list_get(listaPedidos,numero_cliente);// busca el perfil del cliente en la lista de pedidos
			if(cliente->perfilActivo == 1){
				numero_resto = buscar_resto(cliente->nombre_resto);
				if(numero_resto != 0){
					resto = list_get(listaRestos,numero_resto);
					recibir_respuesta(CREAR_PEDIDO,resto,cliente);
					return;
				}
			}
		}else {
			int idPedido = crear_id_pedidos();
			estructura_guardar_pedido = malloc(sizeof(guardar_pedido));

			estructura_guardar_pedido->idPedido = idPedido;
			estructura_guardar_pedido->nombreRestaurante = "RestoDefault";
			estructura_guardar_pedido->largoNombreRestaurante = strlen("RestoDefault");

			respuesta = malloc(sizeof(respuesta_ok_error));

			mandar_mensaje(estructura_guardar_pedido,GUARDAR_PEDIDO,socket_commanda);
			recibir_mensaje(respuesta,GUARDAR_PEDIDO,socket_commanda);
			mandar_mensaje(respuesta,CREAR_PEDIDO,socket_cliente);
			return;
		}
		sem_wait(semLog);
		log_info(logger, "no se logro crear pedido");
		sem_post(semLog);

	}else{
		sem_wait(semLog);
		log_info(logger, "hace falta seleccionar restaurante");
		sem_post(semLog);
	}
}

void aniadir_plato(){
	/*
	char* prueba = "[pedido1,pedido2,pedido3]";
	char** a = string_get_string_as_array(prueba);
	int i = cantidadDeElementosEnArray(a);
	printf("%i", i);
	*/
}

// agrega un restaurante que se conecto a app a la lista de restaurantes
void agregar_restaurante(info_resto* recibidoAgregarRestaurante){
	int numero_resto;
	info_resto* resto;

	if(listaRestos->elements_count != 0){
		numero_resto = buscar_resto(recibidoAgregarRestaurante->nombre_resto);
		if(numero_resto != 0){ // si encontro el restaurante en la lista actualiza el socket, sino lo agrega
			resto = list_get(listaRestos,numero_resto);
			resto->socket = recibidoAgregarRestaurante->socket;
		}else{
			list_add(listaRestos,recibidoAgregarRestaurante);
		}
	}else{
		list_add(listaRestos,recibidoAgregarRestaurante);
	}
}

// **************************************FIN MENSAJES**************************************

/* esto esta en utils.c
// Cuenta la cantidad de elementos en un array
int cantidadDeElementosEnArray(char** array){
	int i = 0;
	while(array[i] != NULL && strcmp(array[i], "\n") != 0){
		i++;
	}
	return i; // 0 es vacio
}
*/

int32_t crear_id_pedidos(){
	sem_wait(semId);
	id_inicial_pedidos += 1;
	sem_post(semId);
	return id_inicial_pedidos;
}

int buscar_cliente(int32_t socket){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente
		if(cliente->socket_cliente == socket && cliente->id_pedido != 0){
			return i;
		}
	}
	return -2;
}

int buscar_resto(char* nombreResto){
	info_resto* resto;
	for(int i = 0; i < listaRestos->elements_count; i++){
		resto = list_get(listaRestos,i);// coseguis el restaurante de la posicion i
		if(strcmp(resto->nombre_resto,nombreResto) == 0){
			return i;
		}
	}
	return 0;
}

//maneja todos los flujos de mensajes que requieran respuestas
void recibir_respuesta(codigo_operacion cod_op, info_resto* resto, perfil_cliente* cliente){
	int32_t socketRespuestas;
	respuesta_ok_error* respuesta;
	guardar_pedido* estructura_guardar_pedido;
	confirmar_pedido* estructura_idPedido;
	respuesta_consultar_platos* estructura_consultar_platos;
	int size;

	switch(cod_op){
	case CREAR_PEDIDO:
		estructura_idPedido = malloc(sizeof(confirmar_pedido));
		socketRespuestas = establecer_conexion(resto->ip,resto->puerto);
		// todo ver con lucas que estructura mandar aca
		//mandar_mensaje(estructura_idPedido,CREAR_PEDIDO,socketRespuestas);
		recibir_mensaje(estructura_idPedido,RESPUESTA_CREAR_PEDIDO,socketRespuestas);

		if(estructura_idPedido->idPedido != 0){
			// todo ver si esto esta bien
			size = sizeof(uint32_t) + sizeof(strlen(cliente->nombre_resto)) + sizeof(uint32_t);
			estructura_guardar_pedido = malloc(size);

			estructura_guardar_pedido->idPedido = estructura_idPedido->idPedido;
			estructura_guardar_pedido->nombreRestaurante = cliente->nombre_resto;
			estructura_guardar_pedido->largoNombreRestaurante = strlen(cliente->nombre_resto);

			respuesta = malloc(sizeof(respuesta_ok_error));

			mandar_mensaje(estructura_guardar_pedido,GUARDAR_PEDIDO,socket_commanda);
			recibir_mensaje(respuesta,RESPUESTA_GUARDAR_PEDIDO,socket_commanda);

			if(respuesta->respuesta == 1){
				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,cliente->socket_cliente);
				cliente->id_pedido = estructura_idPedido->idPedido;
			}else{
				respuesta = malloc(sizeof(respuesta_ok_error));
				respuesta->respuesta = 0;
				mandar_mensaje(respuesta,CREAR_PEDIDO,cliente->socket_cliente);
			}

			// todo verificar si estan bien los cod_op

		}else{
			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta->respuesta = 0;
			mandar_mensaje(respuesta,CREAR_PEDIDO,cliente->socket_cliente);
		}
		close(socketRespuestas);
		break;

	case CONSULTAR_PLATOS:
		// todo ver que hacer con este malloc
		estructura_consultar_platos = malloc(sizeof(respuesta_consultar_platos));
		socketRespuestas = establecer_conexion(resto->ip,resto->puerto);
		// todo ver con lucas que estructura mandar aca
		//mandar_mensaje(estructura_consultar_platos,CONSULTAR_PLATOS,socketRespuestas);
		recibir_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS,socketRespuestas);
		mandar_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS,cliente->socket_cliente);
		// todo verificar si estan bien los cod_op
		close(socketRespuestas);
		break;

	default:
		break;
	}
}
//************* FUNCIONES DE SERVER *************

void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar)  {
	info_resto* recibidoAgregarRestaurante;
	seleccionar_restaurante* recibidoSeleccionarRestaurante;

	switch (cod_op) {
		case CONSULTAR_RESTAURANTES:
			consultar_restaurantes(socket_cliente);
			break;

		case SELECCIONAR_RESTAURANTE:
			recibidoSeleccionarRestaurante = malloc(sizeof(seleccionar_restaurante));
			recibir_mensaje(recibidoSeleccionarRestaurante,SELECCIONAR_RESTAURANTE,socket_cliente);
			seleccionarRestaurante(recibidoSeleccionarRestaurante->nombreRestaurante,socket_cliente);
			break;

		case CONSULTAR_PLATOS:
			consultar_platos(socket_cliente);
			break;

		case CREAR_PEDIDO:
			crear_pedido(socket_cliente);
			break;

		case A_PLATO:
			break;

		case AGREGAR_RESTAURANTE:
			//todo como hago este malloc si no tengo el tamaño de los char*
			recibidoAgregarRestaurante = malloc(sizeof(info_resto));
			recibir_mensaje(recibidoAgregarRestaurante,AGREGAR_RESTAURANTE,socket_cliente);
			agregar_restaurante(recibidoAgregarRestaurante);
			break;

		case DESCONEXION:
			pthread_exit(NULL);

		case ERROR:
			pthread_exit(NULL);

		default:
			break;
		}
}

void serve_client(int32_t* socket)
{
	while(1){
		int32_t sizeAAllocar;
		codigo_operacion cod_op;
		int32_t recibidosSize = 0;

		int32_t recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
		bytesRecibidos(recibidos);

		if(recibidos >= 1)
		{
			recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);
			printf("Tamaño de lo que sigue en el buffer: %u.\n", sizeAAllocar);

			process_request(cod_op, *socket, sizeAAllocar);
		}


		else
		{
			pthread_exit(NULL);
		}

		recibidosSize = 0;
		recibidos = 0;
	}
}

void esperar_cliente(int32_t socket_servidor)
{
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	pthread_t thread;
	int32_t* socket_cliente = malloc(sizeof(int32_t));// todo verificar que no da problemas
	*socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	//pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_create(&thread,NULL,(void*)serve_client,socket_cliente);
	pthread_detach(thread);
}

void iniciar_server(char* puerto)
{
	int32_t socket_servidor;
	socket_servidor = reservarSocket(puerto);

    while(1)
    {
    	esperar_cliente(socket_servidor);
    }

}

