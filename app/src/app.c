#include "app.h"

int main(){
	listaRestos = list_create(); //inicializo la lista de restaurantes
	listaPedidos = list_create(); //inicializo la lista de pedidos
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
	LOG_PATH = config_get_string_value(config,"LOG_FILE_PATH");
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
	mi_puerto = config_get_string_value(config,"MI_PUERTO");
	puerto_commanda = config_get_string_value(config,"PUERTO_COMANDA");
	ip_commanda = config_get_string_value(config,"IP_COMANDA");

	//coneccion a commanda
	socket_commanda = establecer_conexion(ip_commanda,puerto_commanda);

	//inicio el server
	iniciar_server(mi_puerto);
	return EXIT_SUCCESS;
}

int32_t crear_id_pedidos(){
	sem_wait(semId);
	id_inicial_pedidos += 1;
	sem_post(semId);
	return id_inicial_pedidos;
}

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

	restoBuscado = buscar_restaurante(nombreResto);
	respuesta = malloc(sizeof(respuesta_ok_error));

	if(restoBuscado == 1){
		perfil = malloc(sizeof(perfil_cliente));
		perfil->socket_cliente = socket_cliente;
		perfil->nombre_resto = nombreResto;
		perfil->id_pedido = 0;
		list_add(listaPedidos,perfil);

		respuesta->respuesta = 1;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);
	}else{
		respuesta->respuesta = 0;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);
	}
}


int buscar_restaurante(char* nombreResto){
	info_resto* resto;
	for(int i = 0; i < listaRestos->elements_count; i++){
		resto = list_get(listaRestos,i);// coseguis el restaurante de la posicion i
		if(resto->nombre_resto == nombreResto){
			return 1;
		}
	}
	return 0;
}

int32_t crear_pedido(int32_t socket_cliente){
	int numero_socket_resto, numero_cliente;
	perfil_cliente* cliente;
	guardar_pedido* estructura_guardar_pedido;
	respuesta_ok_error* respuesta;

	if(listaRestos->elements_count != 0){
		numero_cliente = buscar_cliente(socket_cliente);
		if(numero_cliente != -2){
			cliente = list_get(listaPedidos,numero_cliente);// busca el perfil del cliente en la lista de pedidos
			numero_socket_resto = buscar_socket_resto(cliente->nombre_resto);
			if(numero_socket_resto != 0){
				recibir_respuesta(CREAR_PEDIDO,numero_socket_resto,socket_cliente,cliente->nombre_resto);
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
	}

	return 0;
}

int buscar_cliente(int32_t socket){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente
		if(cliente->socket_cliente == socket){
			return i;
		}
	}
	return -2;
}

int32_t buscar_socket_resto(char* nombreResto){
	info_resto* resto;
	for(int i = 0; i < listaRestos->elements_count; i++){
		resto = list_get(listaRestos,i);// coseguis el restaurante de la posicion i
		if(resto->nombre_resto == nombreResto){
			return resto->socket;
		}
	}
	return 0;
}

// agrega un restaurante que se conecto a app a la lista de restaurantes
void agregar_restaurante(info_resto* recibidoAgregarRestaurante){
	list_add(listaRestos,recibidoAgregarRestaurante);
}

//maneja todos los flujos de mensajes que requieran respuestas
void recibir_respuesta(codigo_operacion cod_op, int32_t socket_resto, int32_t  socket_cliente, char* nombreResto){
	int32_t socketRespuestas;
	respuesta_ok_error* respuesta;
	guardar_pedido* estructura_guardar_pedido;
	confirmar_pedido* estructura_idPedido;

	switch(cod_op){
	case CREAR_PEDIDO:
		estructura_idPedido = malloc(sizeof(confirmar_pedido));
		//todo poner ip y puerto
		//socketRespuestas = establecer_conexion();//ese es el restaurante
		//mandar_mensaje(estructura_idPedido,cod_op,socketRespuestas); todo como mandar esto
		recibir_mensaje(estructura_idPedido,cod_op,socketRespuestas);
		if(estructura_idPedido->idPedido != 0){
			estructura_guardar_pedido = malloc(sizeof(guardar_pedido));

			estructura_guardar_pedido->idPedido = estructura_idPedido->idPedido;
			estructura_guardar_pedido->nombreRestaurante = nombreResto;
			estructura_guardar_pedido->largoNombreRestaurante = strlen(nombreResto);

			respuesta = malloc(sizeof(respuesta_ok_error));

			mandar_mensaje(estructura_guardar_pedido,GUARDAR_PEDIDO,socket_commanda);
			recibir_mensaje(respuesta,GUARDAR_PEDIDO,socket_commanda);
			mandar_mensaje(respuesta,CREAR_PEDIDO,socket_cliente);

		}else{
			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta->respuesta = 0;
			mandar_mensaje(respuesta,CREAR_PEDIDO,socket_cliente);
		}
		break;
	default:
		break;
	}
}

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
		case CREAR_PEDIDO:
			break;
		case AGREGAR_RESTAURANTE:
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
			//todo preguntar si los semaforos servian para arlo y si esta estructura esta bien o genera problemas
			//sem_wait(semRecibirMensajes);

			recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);
			printf("Tamaño de lo que sigue en el buffer: %u.\n", sizeAAllocar);

			process_request(cod_op, *socket, sizeAAllocar);

			//sem_post(semRecibirMensajes);
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
	int32_t socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
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

