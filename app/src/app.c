#include "app.h"

int main(){

	inicializar_colas();
	inicializar_semaforos();
	id_global = 0;
	id_restoDefault = 0;

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
	logger = cargarUnLogApp(LOG_PATH, "App");
	if(logger != NULL)
	{
		puts("Archivo de LOGS cargado correctamente.");
	}
	else
	{
		puts("Error al cargar archivo de LOGS, abortando...");
		abort();
	}
	//cargo algoritmo de planificacion a utilizar
	//algoritmo = valor_para_switch_case_planificacion(config_get_string_value(config, "ALGORITMO_DE_PLANIFICACION"));


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

/*
	//conexion a commanda, pero hariamos on demand, chequear despues
	socket_commanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(socket_commanda<0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerto, procedo a fallecer");
		sem_post(semLog);
		exit(-2);
	}
	close(socket_commanda);
*/
	pthread_create(&planificacion, NULL,(void*)iniciarPlanificacion, NULL);
	pthread_detach(planificacion);

	sem_wait(planificacionInicializada);
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
    planificacionInicializada = malloc(sizeof(sem_t));
	sem_init(semId, 0, 1);
	sem_init(semLog, 0, 1);
	sem_init(planificacionInicializada, 0, 1);
}

// *********************************FIN SETUP******************************************************

//-------------------------------PRUEBAS DE MENSAJES-----------------------------------------------

/*
 *
 * CONSULTAR RESTAURANTES - Tested
 * SELECCIONAR RESTAURANTES -
 * CONSULTAR PLATOS -
 * CREAR PEDIDO -
 * ANIADIR PLATO -
 * PLATO LISTO -
 * CONFIRMAR PEDIDO -
 * CONSULTAR PEDIDO -
 *
 */


//manda un array con los nombres de todos los restaurantes conectados o restoDefault si no hay ninguno
void consultar_restaurantes(int32_t socket_cliente){
	respuesta_consultar_restaurantes* mensajeRespuestaConsultarRestaurantes;
	char* stringCompleto;
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

	}
	else{
		stringCompleto = string_new();
		string_append(&stringCompleto, stringInicial);

		for(int i = 0; i < listaRestos->elements_count; i++){
			if(i > 1){
				string_append(&stringCompleto, stringSeparador);// agregas una , si ya hay elementos agregados
			}
			resto = list_get(listaRestos,i);// coseguis el restaurante de la posicion i
			stringNuevoParaAgregar = resto->nombre_resto;
			string_append(&stringCompleto, stringNuevoParaAgregar);
		}
	}
	string_append(&stringCompleto, stringFinal);// agregas un ] al final
	mensajeRespuestaConsultarRestaurantes = malloc(sizeof(respuesta_consultar_restaurantes));

	if(listaRestos->elements_count == 0){
		mensajeRespuestaConsultarRestaurantes->cantRestaurantes = 1;

	}else{
		mensajeRespuestaConsultarRestaurantes->cantRestaurantes = listaRestos->elements_count;
	}

	mensajeRespuestaConsultarRestaurantes->listaRestaurantes = malloc(strlen(stringCompleto) + 1);
	strcpy(mensajeRespuestaConsultarRestaurantes->listaRestaurantes,stringCompleto);
	mensajeRespuestaConsultarRestaurantes->longitudListaRestaurantes = strlen(stringCompleto);

	mandar_mensaje(mensajeRespuestaConsultarRestaurantes, RESPUESTA_CONSULTAR_R ,socket_cliente);
}

/* si el restaurante seleccionado existe en la lista de restaurantes manda un 1 y selecciona ese
 * restaurante para el cliente, en caso de no encontrar el nombre manda un 0 en señal de error
*/
void seleccionarRestaurante(char* idCliente, char* nombreResto, int32_t socket_cliente){
	int restoBuscado,numero_cliente;
	respuesta_ok_error* respuesta;
	perfil_cliente* perfil;

	restoBuscado = buscar_resto(nombreResto);
	respuesta = malloc(sizeof(respuesta_ok_error));

	if(restoBuscado != -2 || strcmp(nombreResto,"RestoDefault") == 0){
		numero_cliente = buscar_cliente(idCliente);
		perfil = list_get(listaPedidos,numero_cliente);
		perfil->nombre_resto = malloc(strlen(nombreResto)+1);
		strcpy(perfil->nombre_resto,nombreResto);
		perfil->id_global = crear_id_pedidos(id_global);

		respuesta->respuesta = 1;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);

	}else{
		respuesta->respuesta = 0;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);
	}
	free(respuesta);
}

void consultarPlatos(int32_t socket_cliente){
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
				free(platosDefault->nombresPlatos);
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
	int32_t recibidos, recibidosSize = 0, sizeAAllocar, nuevoSocketComanda;
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
			estructura_guardar_pedido = malloc(sizeof(guardar_pedido));

			estructura_guardar_pedido->idPedido = cliente->id_global;
			estructura_guardar_pedido->nombreRestaurante = malloc(strlen("RestoDefault")+1);
			strcpy(estructura_guardar_pedido->nombreRestaurante,"RestoDefault");
			estructura_guardar_pedido->largoNombreRestaurante = strlen("RestoDefault");

			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta = 0;

			nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
			if(nuevoSocketComanda < 0){
				sem_wait(semLog);
				log_info(logger, "Comanda sigue morido, me muero yo tambien");
				sem_post(semLog);
				exit(-2);
			}

			mandar_mensaje(estructura_guardar_pedido,GUARDAR_PEDIDO,nuevoSocketComanda);

			recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);

				recibir_mensaje(respuesta,RESPUESTA_GUARDAR_PEDIDO,nuevoSocketComanda);

				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socket_cliente);

			} else {
				sem_wait(semLog);
				log_info(logger, "Problemas recibiendo la respuesta de crear pedido de comanda");
				sem_post(semLog);
			}

            close(nuevoSocketComanda);
			free(estructura_guardar_pedido);
			free(respuesta);
		}
		sem_wait(semLog);
		log_info(logger, "Problemas creando el pedido");
		sem_post(semLog);

	}else{
		sem_wait(semLog);
		log_info(logger, "Se requiere seleccionar restaurante antes de poder crear un pedido");
		sem_post(semLog);
	}
}

void aniadir_plato(a_plato* recibidoAPlato, int32_t socket_cliente){
	int numero_cliente, numero_resto;
	perfil_cliente* cliente;
	info_resto* resto;
	int32_t socketRespuestas;
	respuesta_ok_error* respuesta;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar, nuevoSocketComanda;
	codigo_operacion cod_op;

	numero_cliente = buscar_pedido_por_id(recibidoAPlato->idPedido);

	if(numero_cliente != -2){
		cliente = list_get(listaPedidos,numero_cliente);

		if(strcmp(cliente->nombre_resto,"RestoDefault") == 0){
			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta->respuesta = 0;

			nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
			if(nuevoSocketComanda < 0){
				sem_wait(semLog);
				log_info(logger, "Comanda esta muerta, me muero yo tambien");
				sem_post(semLog);
				exit(-2);
			}

			mandar_mensaje(recibidoAPlato,A_PLATO,nuevoSocketComanda);

			recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);
				recibir_mensaje(respuesta,RESPUESTA_A_PLATO,nuevoSocketComanda);
			}

			mandar_mensaje(respuesta,RESPUESTA_A_PLATO, socket_cliente);
			close(nuevoSocketComanda);
			free(respuesta);

		}else{
			numero_resto = buscar_resto(cliente->nombre_resto);

			if(numero_resto != -2){
				resto = list_get(listaRestos,numero_resto);
				socketRespuestas = establecer_conexion(resto->ip,resto->puerto);

				respuesta = malloc(sizeof(respuesta_ok_error));
				respuesta->respuesta = 0;

				mandar_mensaje(recibidoAPlato,A_PLATO,socketRespuestas);

				recibidos = recv(socketRespuestas, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

				if(recibidos >= 1){
					recibidosSize = recv(socketRespuestas, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
					bytesRecibidos(recibidosSize);
					recibir_mensaje(respuesta,RESPUESTA_A_PLATO,socketRespuestas);
				}

				if(respuesta->respuesta == 1){

					nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
					if(nuevoSocketComanda < 0){
						sem_wait(semLog);
						log_info(logger, "Comanda esta muerta, me muero yo tambien");
						sem_post(semLog);
						exit(-2);
					}

					mandar_mensaje(recibidoAPlato,A_PLATO,nuevoSocketComanda);

					recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

					if(recibidos >= 1){
						recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
						bytesRecibidos(recibidosSize);
						recibir_mensaje(respuesta,RESPUESTA_A_PLATO,nuevoSocketComanda);
					}

					mandar_mensaje(respuesta,RESPUESTA_A_PLATO, socket_cliente);

				}else{
					mandar_mensaje(respuesta,RESPUESTA_A_PLATO, socket_cliente);
				}
				free(respuesta);
				close(nuevoSocketComanda);
				close(socketRespuestas);
			}
		}

	}else{
		sem_wait(semLog);
		log_info(logger, "id de pedido erroneo");
		sem_post(semLog);
	}
}

void plato_Listo(plato_listo* platoListo, int32_t socket_cliente){
	respuesta_ok_error* respuesta = malloc(sizeof(respuesta_ok_error));
	obtener_pedido* obtener_Pedido;
	respuesta_obtener_pedido* respuesta_consulta;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar, nuevoSocketComanda;
	codigo_operacion cod_op;
	info_resto* resto;
	perfil_cliente* cliente;

	int numResto = buscar_resto_por_socket(socket_cliente), numCliente;

	if(numResto != -2){
		resto = list_get(listaRestos,numResto);
		numCliente = buscar_pedido_por_id_y_resto(platoListo->idPedido,resto);
		cliente = list_get(listaPedidos,numCliente);
	}

	respuesta->respuesta = 0;

	obtener_Pedido = malloc(sizeof(obtener_pedido));
	obtener_Pedido->largoNombreRestaurante = platoListo->largoNombreRestaurante;
	obtener_Pedido->nombreRestaurante = malloc(strlen(platoListo->nombreRestaurante)+1);
	strcpy(obtener_Pedido->nombreRestaurante,platoListo->nombreRestaurante);
	obtener_Pedido->idPedido = platoListo->idPedido;

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	mandar_mensaje(platoListo,PLATO_LISTO,nuevoSocketComanda);

	recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

	if(recibidos >= 1){
		recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
		bytesRecibidos(recibidosSize);
		recibir_mensaje(respuesta,RESPUESTA_PLATO_LISTO,nuevoSocketComanda);
	}

	if(respuesta->respuesta == 1){
		mandar_mensaje(obtener_Pedido,OBTENER_PEDIDO,nuevoSocketComanda);

		recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

		if(recibidos >= 1){
			recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);

			respuesta_consulta = malloc(sizeAAllocar);

			recibir_mensaje(respuesta_consulta,RESPUESTA_OBTENER_PEDIDO,nuevoSocketComanda);

			char** listaComidas = string_get_string_as_array(respuesta_consulta->comidas);
			char** listaComidasTotales = string_get_string_as_array(respuesta_consulta->cantTotales);
			char** listaComidasListas = string_get_string_as_array(respuesta_consulta->cantListas);
			int i = 0, j = 0;

			while(listaComidas != NULL && strcmp(platoListo->nombrePlato,listaComidas[i]) == 0){
				i++;
			}

			if(strcmp(platoListo->nombrePlato,listaComidas[i]) == 0){

				if(strcmp(listaComidasTotales[i],listaComidasListas[i]) == 0){

					i = 0;
					while(listaComidas != NULL){

						if(strcmp(listaComidasTotales[i],listaComidasListas[i]) == 0){
							j++;
						}
						i++;
					}
					if(i == j){
						sem_wait(semLog);
						log_info(logger, "pedido completo, avisando al repartidor");
						sem_post(semLog);

						// Se avisa a planificacion que el pedido esta listo
						guardarPedidoListo(cliente->id_global);


					}else{
						sem_wait(semLog);
						log_info(logger, "pedido incompleto, faltan comidas por preparar");
						sem_post(semLog);
					}

				}else{
					sem_wait(semLog);
					log_info(logger, "pedido incompleto, actualizando commanda");
					sem_post(semLog);

					mandar_mensaje(platoListo,PLATO_LISTO,nuevoSocketComanda);
				}

			}else{
				sem_wait(semLog);
				log_info(logger, "plato no perteneciente al pedido");
				sem_post(semLog);
			}
		}

	}else{
		sem_wait(semLog);
		log_info(logger, "ocurrio un error");
		sem_post(semLog);
	}
	close(nuevoSocketComanda);
	free(respuesta);
}

void confirmar_Pedido(confirmar_pedido* pedido, int32_t socket_cliente){
	int numero_cliente, numero_resto;
	perfil_cliente* cliente;
	info_resto* resto;
	respuesta_ok_error* respuesta = malloc(sizeof(respuesta_ok_error));
	pcb_pedido* nuevoPcb;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar, nuevoSocketComanda;
	codigo_operacion cod_op;
	confirmar_pedido* aux;
	respuesta->respuesta = 0;

	numero_cliente = buscar_pedido_por_id(pedido->idPedido);

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	if(numero_cliente != -2){
		cliente = list_get(listaPedidos,numero_cliente);

		if(strcmp(cliente->nombre_resto,"RestoDefault") == 0){
			nuevoPcb = malloc(sizeof(pcb_pedido));

			nuevoPcb->pedidoID = pedido->idPedido;
			nuevoPcb->posRestauranteX = posX_resto;
			nuevoPcb->posRestauranteY = posY_resto;
			nuevoPcb->socketCliente = cliente->socket_cliente;

			agregarANew(nuevoPcb);

			mandar_mensaje(pedido,CONFIRMAR_PEDIDO,nuevoSocketComanda);

			recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);
				recibir_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,nuevoSocketComanda);

			}else{
				sem_wait(semLog);
				log_info(logger, "ocurrio un error");
				sem_post(semLog);
			}

			mandar_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO, socket_cliente);

		}else{
			numero_resto = buscar_resto(cliente->nombre_resto);

			if(numero_resto != -2){
				resto = list_get(listaRestos,numero_resto);

				aux = malloc(sizeof(confirmar_pedido));
				aux->idPedido = cliente->id_pedido_resto;

				mandar_mensaje(aux,CONFIRMAR_PEDIDO,resto->socket);

				recibidos = recv(resto->socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

				if(recibidos >= 1){
					recibidosSize = recv(resto->socket, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
					bytesRecibidos(recibidosSize);
					recibir_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,resto->socket);

				}else{
					sem_wait(semLog);
					log_info(logger, "ocurrio un error");
					sem_post(semLog);
				}

				if(respuesta->respuesta == 1){
					nuevoPcb = malloc(sizeof(pcb_pedido));

					nuevoPcb->pedidoID = pedido->idPedido;
					nuevoPcb->posRestauranteX = posX_resto;
					nuevoPcb->posRestauranteY = posY_resto;

					agregarANew(nuevoPcb);

					mandar_mensaje(pedido,CONFIRMAR_PEDIDO,nuevoSocketComanda);

					recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

					if(recibidos >= 1){
						recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
						bytesRecibidos(recibidosSize);
						recibir_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,nuevoSocketComanda);
					}

					mandar_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,socket_cliente);

				}else{
					mandar_mensaje(respuesta,RESPUESTA_CONFIRMAR_PEDIDO,socket_cliente);
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
    close(nuevoSocketComanda);
	free(respuesta);
}

// TODO | Ver si esto es cliente
/*
void consultar_Pedido(consultar_pedido* pedido){
	perfil_cliente* cliente;
	respuesta_consultar_pedido* datosObtenerPedido;
	obtener_pedido* obtener_Pedido;
	respuesta_obtener_pedido* respuesta_consulta;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar, numero_cliente, nuevoSocketComanda;
	codigo_operacion cod_op;

	numero_cliente = buscar_pedido_por_id(pedido->idPedido);
	cliente = list_get(listaPedidos,numero_cliente);

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	obtener_Pedido = malloc(sizeof(obtener_pedido));
	obtener_Pedido->largoNombreRestaurante = strlen(cliente->nombre_resto);
	obtener_Pedido->nombreRestaurante = malloc(strlen(cliente->nombre_resto)+1);
	strcpy(obtener_Pedido->nombreRestaurante,cliente->nombre_resto);
	obtener_Pedido->idPedido = cliente->id_global;



	mandar_mensaje(obtener_Pedido,OBTENER_PEDIDO,nuevoSocketComanda);

	recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

	if(recibidos >= 1){
		recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
		bytesRecibidos(recibidosSize);

		respuesta_consulta = malloc(sizeAAllocar);

		recibir_mensaje(respuesta_consulta,RESPUESTA_OBTENER_PEDIDO,nuevoSocketComanda);

		char** listaComidas = string_get_string_as_array(respuesta_consulta->comidas);
		char** listaComidasTotales = string_get_string_as_array(respuesta_consulta->cantTotales);
		char** listaComidasListas = string_get_string_as_array(respuesta_consulta->cantListas);
		int i = 0, j = 0;

		char* stringCompleto = string_new();
		char* stringInicial = "[";
		char* stringFinal = "]";
		char* stringSeparador = ",";
		char* stringCorcheteInicial = "{";
		char* stringCorcheteFinal = "}";

		string_append(&stringCompleto, stringInicial);

		while(listaComidas[i] != NULL){
			string_append(&stringCompleto, stringCorcheteInicial);
			string_append(&stringCompleto, listaComidas[i]);
			string_append(&stringCompleto, stringSeparador);
			string_append(&stringCompleto, listaComidasTotales[i]);
			string_append(&stringCompleto, stringSeparador);
			string_append(&stringCompleto, listaComidasListas[i]);
			string_append(&stringCompleto, stringCorcheteFinal);

			if(listaComidas[i+1] != NULL){
					string_append(&stringCompleto, stringSeparador);// agregas una , si ya hay elementos agregados
				}
			i++;
			j++;
		}
		string_append(&stringCompleto, stringFinal);

		datosObtenerPedido = malloc(sizeof(respuesta_consultar_pedido));
		datosObtenerPedido->largoNombreRestaurante = strlen(cliente->nombre_resto);
		datosObtenerPedido->nombreRestaurante = malloc(strlen(cliente->nombre_resto) + 1);
		strcpy(datosObtenerPedido->nombreRestaurante,cliente->nombre_resto);

		// TODO | Revisar
		datosObtenerPedido->comidas = malloc(strlen(stringCompleto) + 1);
		strcpy(datosObtenerPedido->comidas,stringCompleto);
		datosObtenerPedido->sizeComidas = strlen(stringCompleto);

		mandar_mensaje(datosObtenerPedido,RESPUESTA_CONSULTAR_PEDIDO,cliente->socket_cliente);
	}
}*/


void consultar_Pedido(consultar_pedido* elPedido, int32_t socket_cliente){
	perfil_cliente* cliente;
	respuesta_obtener_pedido* respuesta_obtener;
	int32_t numero_cliente, nuevoSocketComanda;

	numero_cliente = buscar_pedido_por_id(elPedido->idPedido);
	cliente = list_get(listaPedidos,numero_cliente);

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	obtener_pedido* datosPedido = malloc(sizeof(obtener_pedido));
	datosPedido->idPedido = elPedido->idPedido;
	datosPedido->largoNombreRestaurante = strlen(cliente->nombre_resto);
	datosPedido->nombreRestaurante = malloc(strlen(cliente->nombre_resto) +1);
	strcpy(datosPedido->nombreRestaurante,cliente->nombre_resto);

	respuesta_obtener = malloc(sizeof(respuesta_obtener_pedido));

	mandar_mensaje(datosPedido,OBTENER_PEDIDO,nuevoSocketComanda);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(nuevoSocketComanda, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(nuevoSocketComanda, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    printf("El size del buffer/payload es: %u", sizePayload);

    recibir_mensaje(respuesta_obtener, RESPUESTA_OBTENER_PEDIDO, nuevoSocketComanda);

    mandar_mensaje(respuesta_obtener, RESPUESTA_CONSULTAR_PEDIDO, socket_cliente);
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

void registrarHandshake(handshake* recibidoHandshake, int32_t socket_cliente){
	perfil_cliente* perfil;

	perfil = malloc(sizeof(perfil_cliente));
	perfil->socket_cliente = socket_cliente;
	perfil->perfilActivo = 1;
	perfil->posX = recibidoHandshake->posX;
	perfil->posY = recibidoHandshake->posY;
	perfil->idCliente = malloc(strlen(recibidoHandshake->id)+1);
	strcpy(perfil->idCliente, recibidoHandshake->id);
	list_add(listaPedidos,perfil);

	free(recibidoHandshake->id);
	free(recibidoHandshake);

	sem_wait(semLog);
	log_info(logger, "perfil creado");
	sem_post(semLog);
}

// **************************************FIN MENSAJES**************************************

int32_t crear_id_pedidos(){
	sem_wait(semId);
	id_global += 1;
	sem_post(semId);
	return id_global;
}



int buscar_pedido_por_id_y_resto(uint32_t id_pedido, info_resto* resto){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if(cliente->id_global == id_global && strcmp(resto->nombre_resto,cliente->nombre_resto) == 0){
			return i;
		}
	}
	return -2;
}

int buscar_cliente(char* idClienteBuscado){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if((strcmp(cliente->idCliente, idClienteBuscado) == 0) && cliente->id_pedido_resto == 0){
			return i;
		}
	}
	return -2;
}

int buscar_resto(char* nombreResto){
	info_resto* resto;
	if(listaRestos->elements_count != 0){

		for(int i = 0; i < listaRestos->elements_count; i++){
			resto = list_get(listaRestos,i);// coseguis el restaurante de la posicion i

			if(strcmp(resto->nombre_resto,nombreResto) == 0){
				return i;
			}
		}
	}

	return -2;
}

int buscar_resto_por_socket(int32_t socket_resto){
	info_resto* resto;
	if(listaRestos->elements_count != 0){

		for(int i = 0; i < listaRestos->elements_count; i++){
			resto = list_get(listaRestos,i);// coseguis el restaurante de la posicion i

			if(resto->socket == socket_resto){
				return i;
			}
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
	int32_t recibidos, recibidosSize = 0, sizeAAllocar = 0, nuevoSocketComanda;

	switch(cod_op){
	case CREAR_PEDIDO:
		estructura_idPedido = malloc(sizeof(confirmar_pedido));
		estructura_idPedido->idPedido = 0;

		socketRespuestas = establecer_conexion(resto->ip,resto->puerto);
		mandar_mensaje(estructura_idPedido,CREAR_PEDIDO,socketRespuestas);//todo cambiar si rompe

		recibidos = recv(socketRespuestas, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

		if(recibidos >= 1){
			recibidosSize = recv(socketRespuestas, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);

			recibir_mensaje(estructura_idPedido,RESPUESTA_CREAR_PEDIDO,socketRespuestas);
		}

		if(estructura_idPedido->idPedido != 0){
			nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
			if(nuevoSocketComanda < 0){
				sem_wait(semLog);
				log_info(logger, "Comanda esta muerta, me muero yo tambien");
				sem_post(semLog);
				exit(-2);
			}
			estructura_guardar_pedido = malloc(sizeof(guardar_pedido));

			estructura_guardar_pedido->idPedido = cliente->id_global;
			estructura_guardar_pedido->nombreRestaurante = malloc(strlen(cliente->nombre_resto)+1);
			strcpy(estructura_guardar_pedido->nombreRestaurante, cliente->nombre_resto);
			estructura_guardar_pedido->largoNombreRestaurante = strlen(cliente->nombre_resto);

			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta->respuesta = 0;


			mandar_mensaje(estructura_guardar_pedido,GUARDAR_PEDIDO,nuevoSocketComanda);

			recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);

				recibir_mensaje(respuesta,RESPUESTA_GUARDAR_PEDIDO,nuevoSocketComanda);
			}

			if(respuesta->respuesta == 1){
				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,cliente->socket_cliente);
				cliente->id_pedido_resto = estructura_idPedido->idPedido;

			}else{
				respuesta->respuesta = 0;
				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,cliente->socket_cliente);
			}
			free(estructura_guardar_pedido->nombreRestaurante);
			free(estructura_guardar_pedido);
			free(respuesta);

		}else{
			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta->respuesta = 0;
			mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,cliente->socket_cliente);
			free(respuesta);
		}
		close(socketRespuestas);
		close(nuevoSocketComanda);
		break;

	case CONSULTAR_PLATOS:
		// todo ver que hacer con este malloc

		socketRespuestas = establecer_conexion(resto->ip,resto->puerto);

		respuesta = malloc(sizeof(respuesta_ok_error));
		respuesta->respuesta = 0;

		mandar_mensaje(respuesta,CONSULTAR_PLATOS,socketRespuestas);//todo cambiar si rompe

		recibidos = recv(socketRespuestas, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

		if(recibidos >= 1){
			recibidosSize = recv(socketRespuestas, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);

			estructura_consultar_platos = malloc(sizeAAllocar);

			recibir_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS,socketRespuestas);
			mandar_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS,cliente->socket_cliente);

			free(estructura_consultar_platos->nombresPlatos);
			free(estructura_consultar_platos);
		}

		free(respuesta);
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
	consultar_pedido* recibidoConsultarPedido;
	handshake* recibidoHandshake;

	switch (cod_op) {
		case CONSULTAR_RESTAURANTES:
			consultar_restaurantes(socket_cliente);
			break;

		case SELECCIONAR_RESTAURANTE:
			recibidoSeleccionarRestaurante = malloc(sizeAAllocar);
			recibir_mensaje(recibidoSeleccionarRestaurante,SELECCIONAR_RESTAURANTE,socket_cliente);
			seleccionarRestaurante(recibidoSeleccionarRestaurante->idCliente,recibidoSeleccionarRestaurante->nombreRestaurante,socket_cliente);
			free(recibidoSeleccionarRestaurante->idCliente);
			free(recibidoSeleccionarRestaurante->nombreRestaurante);
			free(recibidoSeleccionarRestaurante);
			break;

		case CONSULTAR_PLATOS:
			consultarPlatos(socket_cliente);
			break;

		case CREAR_PEDIDO:
			crear_pedido(socket_cliente);
			break;

		case A_PLATO:
			recibidoAPlato = malloc(sizeAAllocar);
			recibir_mensaje(recibidoAPlato,A_PLATO,socket_cliente);
			aniadir_plato(recibidoAPlato, socket_cliente);
			free(recibidoAPlato);
			break;

		case PLATO_LISTO:
			recibidoPlatoListo = malloc(sizeAAllocar);
			recibir_mensaje(recibidoPlatoListo,PLATO_LISTO,socket_cliente);
			plato_Listo(recibidoPlatoListo,socket_cliente);
			free(recibidoPlatoListo); //todo ver si esto no rompe nada
			break;

		case CONFIRMAR_PEDIDO:
			recibidoConfirmarPedido = malloc(sizeAAllocar);
			recibir_mensaje(recibidoConfirmarPedido,CONFIRMAR_PEDIDO,socket_cliente);
			confirmar_Pedido(recibidoConfirmarPedido, socket_cliente);
			free(recibidoConfirmarPedido);
			break;

		case CONSULTAR_PEDIDO:
            recibidoConsultarPedido = malloc(sizeAAllocar);
            recibir_mensaje(recibidoConsultarPedido,CONSULTAR_PEDIDO, socket_cliente);
            consultar_Pedido(recibidoConsultarPedido, socket_cliente);
            free(recibidoConsultarPedido);
			break;

		case AGREGAR_RESTAURANTE:
			recibidoAgregarRestaurante = malloc(sizeAAllocar);
			recibir_mensaje(recibidoAgregarRestaurante,AGREGAR_RESTAURANTE,socket_cliente);
			agregar_restaurante(recibidoAgregarRestaurante);
			break;

		case HANDSHAKE:
			recibidoHandshake = malloc(sizeAAllocar);
			recibir_mensaje(recibidoHandshake,HANDSHAKE,socket_cliente);
			registrarHandshake(recibidoHandshake, socket_cliente);
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
	//while(1){
		int32_t sizeAAllocar = 0;
		codigo_operacion cod_op = 0;
		int32_t recibidosSize = 0;

		int32_t recibidos = recv(*socket, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
		bytesRecibidos(recibidos);

		if(cod_op == 1)
		{
			process_request(cod_op, *socket, sizeAAllocar);
		}else{

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
		}

		recibidosSize = 0;
		recibidos = 0;
	//}
	//close(socket);
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

