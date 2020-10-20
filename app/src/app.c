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

	// NOTA: Alterar el orden de estos llamados va a romper (dependen uno del anterior)
	// Inicializo semaforos necesarios para planif
	iniciarSemaforosPlanificacion();
	// Leo los datos que necesito para planificacion
	leerPlanificacionConfig(config);
	// Inicializo los semaforos para ciclos de CPU
	iniciarSemaforosCiclos();


	//coneccion a commanda
	socket_commanda = establecer_conexion(ip_commanda,puerto_commanda);

	//inicio el server | TODO Se quita esto para testear planificacion
	//iniciar_server(mi_puerto);

	iniciarPlanificacion();

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

	restoBuscado = buscar_resto(nombreResto);
	respuesta = malloc(sizeof(respuesta_ok_error));

	if(restoBuscado != -2){
		perfil = malloc(sizeof(perfil_cliente));
		perfil->socket_cliente = socket_cliente;
		perfil->nombre_resto = malloc(strlen(nombreResto)+1);
		strcpy(perfil->nombre_resto,nombreResto);
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

void consultar_platos(int32_t socket_cliente){
	int numero_cliente, numero_resto;
	perfil_cliente* cliente;
	info_resto* resto;
	respuesta_consultar_platos* platosDefault;

	numero_cliente = buscar_cliente(socket_cliente);

	if(numero_cliente != -2){
		cliente = list_get(listaPedidos,numero_cliente);// busca el perfil del cliente en la lista de pedidos

		if(cliente->perfilActivo == 1){
			numero_resto = buscar_resto(cliente->nombre_resto);

			if(numero_resto != -2){
				resto = list_get(listaRestos,numero_resto);
				recibir_respuesta(CONSULTAR_PLATOS,resto,cliente);
				return;

			}else if(strcmp(cliente->nombre_resto,"RestoDefault") == 0){
				platosDefault = malloc(sizeof(respuesta_consultar_platos));
				platosDefault->longitudNombresPlatos = strlen(platos_default);
				platosDefault->nombresPlatos = malloc(strlen(platos_default)+1);
				strcpy(platosDefault->nombresPlatos, platos_default);

				mandar_mensaje(platosDefault,RESPUESTA_CONSULTAR_PLATOS,socket_cliente);
				free(platosDefault);
			}
			sem_wait(semLog);
			log_info(logger, "no se encontro el restaurante");
			sem_post(semLog);

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
	int32_t recibidos, recibidosSize = 0, sizeAAllocar;
	codigo_operacion cod_op;

	numero_cliente = buscar_cliente(socket_cliente);

	if(numero_cliente != -2){
		cliente = list_get(listaPedidos,numero_cliente);// busca el perfil del cliente en la lista de pedidos

		if(listaRestos->elements_count != 0 && strcmp(cliente->nombre_resto,"RestoDefault") != 0){
			numero_resto = buscar_resto(cliente->nombre_resto);

			if(numero_resto != -2){
				resto = list_get(listaRestos,numero_resto);
				recibir_respuesta(CREAR_PEDIDO,resto,cliente);
			}

		}else if(strcmp(cliente->nombre_resto,"RestoDefault") == 0){
			int idPedido = crear_id_pedidos();

			estructura_guardar_pedido = malloc(sizeof(guardar_pedido));

			estructura_guardar_pedido->idPedido = idPedido;
			estructura_guardar_pedido->nombreRestaurante = malloc(strlen("RestoDefault")+1);
			strcpy(estructura_guardar_pedido->nombreRestaurante,"RestoDefault");
			estructura_guardar_pedido->largoNombreRestaurante = strlen("RestoDefault");

			respuesta = malloc(sizeof(respuesta_ok_error));

			mandar_mensaje(estructura_guardar_pedido,GUARDAR_PEDIDO,socket_commanda);

			recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);

				recibir_mensaje(respuesta,GUARDAR_PEDIDO,socket_commanda);
			}

			mandar_mensaje(respuesta,CREAR_PEDIDO,socket_cliente);
			free(estructura_guardar_pedido);
			free(respuesta);
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

void aniadir_plato(a_plato* recibidoAPlato){
	int numero_cliente, numero_resto;
	perfil_cliente* cliente;
	info_resto* resto;
	int32_t socketRespuestas;
	respuesta_ok_error* respuesta;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar;
	codigo_operacion cod_op;

	numero_cliente = buscar_cliente_id(recibidoAPlato->idPedido);

	if(numero_cliente != -2){
		cliente = list_get(listaPedidos,numero_cliente);
		if(strcmp(cliente->nombre_resto,"RestoDefault") == 0){
			respuesta = malloc(sizeof(respuesta_ok_error));
			mandar_mensaje(recibidoAPlato,A_PLATO,socket_commanda);

			recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);
				recibir_mensaje(respuesta,RESPUESTA_A_PLATO,socket_commanda);
			}

			mandar_mensaje(respuesta,RESPUESTA_A_PLATO,cliente->socket_cliente);
			free(respuesta);

		}else{
			numero_resto = buscar_resto(cliente->nombre_resto);

			if(numero_resto != -2){
				resto = list_get(listaRestos,numero_resto);
				socketRespuestas = establecer_conexion(resto->ip,resto->puerto);

				respuesta = malloc(sizeof(respuesta_ok_error));
				mandar_mensaje(recibidoAPlato,A_PLATO,socketRespuestas);

				recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

				if(recibidos >= 1){
					recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
					bytesRecibidos(recibidosSize);
					recibir_mensaje(respuesta,RESPUESTA_A_PLATO,socketRespuestas);
				}

				if(respuesta->respuesta == 1){
					mandar_mensaje(recibidoAPlato,A_PLATO,socket_commanda);

					recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

					if(recibidos >= 1){
						recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
						bytesRecibidos(recibidosSize);
						recibir_mensaje(respuesta,RESPUESTA_A_PLATO,socket_commanda);
					}

					mandar_mensaje(respuesta,RESPUESTA_A_PLATO,cliente->socket_cliente);

				}else{
					mandar_mensaje(respuesta,RESPUESTA_A_PLATO,cliente->socket_cliente);
				}
				free(respuesta);
				close(socketRespuestas);
			}
		}

	}else{
		sem_wait(semLog);
		log_info(logger, "id de pedido erroneo");
		sem_post(semLog);
	}
}

void plato_Listo(plato_listo* platoListo){
	respuesta_ok_error* respuesta = malloc(sizeof(respuesta_ok_error));
	obtener_pedido* obtener_Pedido;
	respuesta_obtener_pedido* respuesta_consulta;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar;
	codigo_operacion cod_op;

	obtener_Pedido = malloc(sizeof(obtener_pedido));
	obtener_Pedido->largoNombreRestaurante = platoListo->largoNombreRestaurante;
	obtener_Pedido->nombreRestaurante = malloc(strlen(platoListo->nombreRestaurante)+1);
	strcpy(obtener_Pedido->nombreRestaurante,platoListo->nombreRestaurante);
	obtener_Pedido->idPedido = platoListo->idPedido;

	mandar_mensaje(platoListo,PLATO_LISTO,socket_commanda);

	recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

	if(recibidos >= 1){
		recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
		bytesRecibidos(recibidosSize);
		recibir_mensaje(respuesta,RESPUESTA_PLATO_LISTO,socket_commanda);
	}

	mandar_mensaje(obtener_Pedido,OBTENER_PEDIDO,socket_commanda);

	recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

	if(recibidos >= 1){
		recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
		bytesRecibidos(recibidosSize);

		respuesta_consulta = malloc(sizeAAllocar);

		recibir_mensaje(respuesta_consulta,RESPUESTA_OBTENER_PEDIDO,socket_commanda);
	}
	//todo como manejo la lista de patos?

	//todo avisar al repartidor

	free(respuesta);
}

void confirmar_Pedido(confirmar_pedido* pedido){
	//todo ver que devuelve obtenerPedido cuando esta vacio o no esta creado
	int numero_cliente, numero_resto;
	perfil_cliente* cliente;
	info_resto* resto;
	respuesta_ok_error* respuesta = malloc(sizeof(respuesta_ok_error));
	pcb_pedido* nuevoPcb;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar;
	codigo_operacion cod_op;

	numero_cliente = buscar_cliente_id(pedido->idPedido);

	if(numero_cliente != -2){
		cliente = list_get(listaPedidos,numero_cliente);

		if(strcmp(cliente->nombre_resto,"RestoDefault") == 0){
			nuevoPcb = malloc(sizeof(pcb_pedido));
			int id = crear_id_pedidos();

			nuevoPcb->pedidoID = id;
			nuevoPcb->posObjetivoX = posX_resto;
			nuevoPcb->posObjetivoY = posY_resto;
			nuevoPcb->nombre_resto = malloc(strlen("RestoDefault")+1);

			agregarANew(nuevoPcb);

			mandar_mensaje(pedido,CONFIRMAR_PEDIDO,socket_commanda);

			recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);
				recibir_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,socket_commanda);
			}

			mandar_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,cliente->socket_cliente);

		}else{
			numero_resto = buscar_resto(cliente->nombre_resto);

			if(numero_resto != -2){
				resto = list_get(listaRestos,numero_resto);

				mandar_mensaje(pedido,CONFIRMAR_PEDIDO,resto->socket);

				recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

				if(recibidos >= 1){
					recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
					bytesRecibidos(recibidosSize);
					recibir_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,resto->socket);
				}

				if(respuesta->respuesta == 1){
					nuevoPcb = malloc(sizeof(pcb_pedido));

					nuevoPcb->pedidoID = pedido->idPedido;
					nuevoPcb->posObjetivoX = resto->posX;
					nuevoPcb->posObjetivoY = resto->posY;
					nuevoPcb->nombre_resto = malloc(strlen(resto->nombre_resto)+1);

					agregarANew(nuevoPcb);

					mandar_mensaje(pedido,CONFIRMAR_PEDIDO,socket_commanda);

					recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

					if(recibidos >= 1){
						recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
						bytesRecibidos(recibidosSize);
						recibir_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,socket_commanda);
					}

					mandar_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,cliente->socket_cliente);

				}else{
					mandar_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,cliente->socket_cliente);
				}

			}else{
				sem_wait(semLog);
				log_info(logger, "no se pudo encontrar el restaurante");
				sem_post(semLog);
			}
		}

	}else{
		sem_wait(semLog);
		log_info(logger, "no se pudo encontrar el cliente");
		sem_post(semLog);
	}

	free(respuesta);
}

void consultar_Pedido(obtener_pedido* pedido){
	respuesta_obtener_pedido* datosObtenerPedido;
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

int32_t crear_id_pedidos(){
	sem_wait(semId);
	id_inicial_pedidos += 1;
	sem_post(semId);
	return id_inicial_pedidos;
}

int buscar_pedido(uint32_t id_pedido){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente
		if(cliente->id_pedido == id_pedido){
			return i;
		}
	}
	return -2;
}

int buscar_cliente(int32_t socket_cliente_buscado){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente
		if(cliente->socket_cliente == socket_cliente_buscado && cliente->id_pedido != 0){
			return i;
		}
	}
	return -2;
}

int buscar_cliente_id(uint32_t id_pedido_buscado){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente
		if(cliente->id_pedido == id_pedido_buscado){
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
	return -2;
}

//maneja todos los flujos de mensajes que requieran respuestas
void recibir_respuesta(codigo_operacion cod_op, info_resto* resto, perfil_cliente* cliente){
	int32_t socketRespuestas;
	respuesta_ok_error* respuesta;
	guardar_pedido* estructura_guardar_pedido;
	confirmar_pedido* estructura_idPedido;
	respuesta_consultar_platos* estructura_consultar_platos;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar;

	switch(cod_op){
	case CREAR_PEDIDO:
		estructura_idPedido = malloc(sizeof(confirmar_pedido));
		socketRespuestas = establecer_conexion(resto->ip,resto->puerto);
		mandar_mensaje(estructura_idPedido,CREAR_PEDIDO,socketRespuestas);//todo cambiar si rompe

		recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

		if(recibidos >= 1){
			recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);

			recibir_mensaje(estructura_idPedido,RESPUESTA_CREAR_PEDIDO,socketRespuestas);
		}

		if(estructura_idPedido->idPedido != 0){
			estructura_guardar_pedido = malloc(sizeof(guardar_pedido));

			estructura_guardar_pedido->idPedido = estructura_idPedido->idPedido;
			estructura_guardar_pedido->nombreRestaurante = malloc(strlen(cliente->nombre_resto)+1);
			strcpy(estructura_guardar_pedido->nombreRestaurante, cliente->nombre_resto);
			estructura_guardar_pedido->largoNombreRestaurante = strlen(cliente->nombre_resto);

			respuesta = malloc(sizeof(respuesta_ok_error));

			mandar_mensaje(estructura_guardar_pedido,GUARDAR_PEDIDO,socket_commanda);

			recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);

				recibir_mensaje(respuesta,RESPUESTA_GUARDAR_PEDIDO,socket_commanda);
			}

			if(respuesta->respuesta == 1){
				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,cliente->socket_cliente);
				cliente->id_pedido = estructura_idPedido->idPedido;

			}else{
				respuesta->respuesta = 0;
				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,cliente->socket_cliente);
			}
			free(estructura_guardar_pedido);
			free(respuesta);

		}else{
			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta->respuesta = 0;
			mandar_mensaje(respuesta,CREAR_PEDIDO,cliente->socket_cliente);
			free(respuesta);
		}
		close(socketRespuestas);
		break;

	case CONSULTAR_PLATOS:
		// todo ver que hacer con este malloc

		socketRespuestas = establecer_conexion(resto->ip,resto->puerto);

		mandar_mensaje(estructura_consultar_platos,CONSULTAR_PLATOS,socketRespuestas);//todo cambiar si rompe

		recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

		if(recibidos >= 1){
			recibidosSize = recv(*socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);

			estructura_consultar_platos = malloc(sizeAAllocar);

			recibir_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS,socketRespuestas);
			mandar_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS,cliente->socket_cliente);

			free(estructura_consultar_platos);
		}

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
	a_plato* recibidoAPlato;
	plato_listo* recibidoPlatoListo ;
	confirmar_pedido* recibidoConfirmarPedido;

	switch (cod_op) {
		case CONSULTAR_RESTAURANTES:
			consultar_restaurantes(socket_cliente);
			break;

		case SELECCIONAR_RESTAURANTE:
			recibidoSeleccionarRestaurante = malloc(sizeAAllocar);
			recibir_mensaje(recibidoSeleccionarRestaurante,SELECCIONAR_RESTAURANTE,socket_cliente);
			seleccionarRestaurante(recibidoSeleccionarRestaurante->nombreRestaurante,socket_cliente);
			free(recibidoSeleccionarRestaurante);
			break;

		case CONSULTAR_PLATOS:
			consultar_platos(socket_cliente);
			break;

		case CREAR_PEDIDO:
			crear_pedido(socket_cliente);
			break;

		case A_PLATO:
			recibidoAPlato = malloc(sizeAAllocar);
			recibir_mensaje(recibidoAPlato,A_PLATO,socket_cliente);
			aniadir_plato(recibidoAPlato);
			free(recibidoAPlato);
			break;

		case PLATO_LISTO:
			recibidoPlatoListo = malloc(sizeAAllocar);
			recibir_mensaje(recibidoPlatoListo,PLATO_LISTO,socket_cliente);
			plato_Listo(recibidoPlatoListo);
			free(recibidoPlatoListo); //todo ver si esto no rompe nada
			break;

		case AGREGAR_RESTAURANTE:
			recibidoAgregarRestaurante = malloc(sizeAAllocar);
			recibir_mensaje(recibidoAgregarRestaurante,AGREGAR_RESTAURANTE,socket_cliente);
			agregar_restaurante(recibidoAgregarRestaurante);
			break;

		case CONFIRMAR_PEDIDO:
			recibidoConfirmarPedido = malloc(sizeAAllocar);
			recibir_mensaje(recibidoConfirmarPedido,CONFIRMAR_PEDIDO,socket_cliente);
			confirmar_Pedido(recibidoConfirmarPedido);
			free(recibidoConfirmarPedido);
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

