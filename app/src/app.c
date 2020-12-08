#include "app.h"

int main(){
	inicializar_listas();
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
	posX_restoDefault = config_get_int_value(config,"POSICION_REST_DEFAULT_X");
	posY_restoDefault = config_get_int_value(config,"POSICION_REST_DEFAULT_Y");

	int32_t nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
	  sem_wait(semLog);
	  log_error(logger, "Comanda esta muerta, me muero yo tambien");
	  sem_post(semLog);
	  exit(-2);
	}
	close(nuevoSocketComanda);
	sem_wait(semLog);
	log_trace(logger, "[APP] Comanda esta viva, procedo a completar las demas configuraciones.");
	sem_post(semLog);

	// NOTA: Alterar el orden de estos llamados va a romper (dependen uno del anterior)
	// Inicializo semaforos necesarios para planif
	iniciarSemaforosPlanificacion();
	// Leo los datos que necesito para planificacion
	leerPlanificacionConfig(config);
	// Inicializo los semaforos para ciclos de CPU
	iniciarSemaforosCiclos();

	iniciarPlanificacion();

	//inicio el server
	iniciar_server(mi_puerto);

	return EXIT_SUCCESS;
}

void inicializar_listas(){
	listaRestos = list_create(); //inicializo la lista de restaurantes
	listaPedidos = list_create(); //inicializo la lista de pedidos
	listaAsociaciones = list_create();
}

void inicializar_semaforos(){
	semId = malloc(sizeof(sem_t));
	semLog = malloc(sizeof(sem_t));
	mutexListaRestos = malloc(sizeof(sem_t));
	mutexListaAsociaciones = malloc(sizeof(sem_t));
	mutexListaPedidos = malloc(sizeof(sem_t));
	sem_init(semId, 0, 1);
	sem_init(semLog, 0, 1);
	sem_init(mutexListaRestos, 0, 1);
	sem_init(mutexListaAsociaciones, 0, 1);
	sem_init(mutexListaPedidos, 0, 1);
}

// *********************************FIN SETUP******************************************************

//-------------------------------PRUEBAS DE MENSAJES-----------------------------------------------

/*
 *
 * CONSULTAR RESTAURANTES - Tested
 * SELECCIONAR RESTAURANTES - Tested
 * CONSULTAR PLATOS -
 * CREAR PEDIDO -
 * ANIADIR PLATO -
 * PLATO LISTO -
 * CONFIRMAR PEDIDO -
 * CONSULTAR PEDIDO -
 *
 */


//manda un array con los nombres de todos los restaurantes conectados o restoDefault si no hay ninguno
void consultarRestaurantes(int32_t socket_cliente){
	respuesta_consultar_restaurantes* mensajeRespuestaConsultarRestaurantes;
	char* stringCompleto;
	char* stringNuevoParaAgregar;
	char* stringInicial = "[";
	char* stringFinal = "]";
	char* stringSeparador = ",";
	info_resto* resto;

	sem_wait(mutexListaRestos);
	if(listaRestos->elements_count == 0){
		stringCompleto  = string_new();
		stringNuevoParaAgregar = "RestoDefault";
		string_append(&stringCompleto, stringInicial);// agregas un [ al principio
		string_append(&stringCompleto, stringNuevoParaAgregar);

	}else{
		stringCompleto = string_new();
		string_append(&stringCompleto, stringInicial);

		for(int i = 0; i < listaRestos->elements_count; i++){
			if(i >= 1){
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
	sem_post(mutexListaRestos);

	mensajeRespuestaConsultarRestaurantes->listaRestaurantes = malloc(strlen(stringCompleto) + 1);
	strcpy(mensajeRespuestaConsultarRestaurantes->listaRestaurantes,stringCompleto);
	mensajeRespuestaConsultarRestaurantes->longitudListaRestaurantes = strlen(stringCompleto);

	mandar_mensaje(mensajeRespuestaConsultarRestaurantes, RESPUESTA_CONSULTAR_R ,socket_cliente);
}

/*
 * si el restaurante seleccionado existe en la lista de restaurantes manda un 1 y selecciona ese
 * restaurante para el cliente, en caso de no encontrar el nombre manda un 0 en señal de error
*/
void seleccionarRestaurante(seleccionar_restaurante* seleccRestoRecibido, int32_t socket_cliente){
	int indiceRestoBuscado, indiceAsociacionBuscada;
	respuesta_ok_error* respuesta;
	asociacion_cliente* asociacionBuscada;

	indiceRestoBuscado = buscarRestaurante(seleccRestoRecibido->nombreRestaurante);

	respuesta = malloc(sizeof(respuesta_ok_error));

	sem_wait(mutexListaAsociaciones);
	if(indiceRestoBuscado != -2 || strcmp(seleccRestoRecibido->nombreRestaurante,"RestoDefault") == 0){
        //el restaurante existe, me fijo si el tipo no esta asociado ya con algun restaurante de antes
		indiceAsociacionBuscada = buscarAsociacion(seleccRestoRecibido->idCliente);
		if(indiceAsociacionBuscada == -2){
			//el cliente no hizo el handshake, no deberia pasar nunca
			//no puedo hacer nadaxd
			sem_wait(semLog);
			log_error(logger, "[APP] De Alguna manera, hay un cliente que no hizo el handshake y me mando seleccionar.");
			sem_post(semLog);
		} else {
			//le seteo el restaurante que quiere en esta ocasion
			asociacionBuscada = list_get(listaAsociaciones, indiceAsociacionBuscada);
			free(asociacionBuscada->nombreRestaurante);
			asociacionBuscada->nombreRestaurante = malloc(seleccRestoRecibido->largoNombreRestaurante+1);
			strcpy(asociacionBuscada->nombreRestaurante, seleccRestoRecibido->nombreRestaurante);
			sem_wait(semLog);
			log_trace(logger, "[APP] El <%s> ha sido correctamente asociado al restaurante <%s>.",
					seleccRestoRecibido->idCliente, seleccRestoRecibido->nombreRestaurante);
			sem_post(semLog);
		}
		//se concluyo bien la operacion, se agrego un restaurante a la asociacion del cliente o se piso uno previo
		respuesta->respuesta = 1;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);
	}else{
		//el restaurante deseado no existe, tengo que denegar la operacion
		sem_wait(semLog);
		log_error(logger, "[APP] El <%s> ha intentado seleccionar un restaurante que no existe en los registros.",
				seleccRestoRecibido->idCliente);
		sem_post(semLog);
		respuesta->respuesta = 0;
		mandar_mensaje(respuesta,RESPUESTA_SELECCIONAR_R,socket_cliente);

	}
	sem_post(mutexListaAsociaciones);
	free(respuesta);
}

void consultarPlatos(consultar_platos* consultarPlatosRecibido, int32_t socket_cliente){
	int indiceAsociacionBuscada, indiceRestoBuscado;
	asociacion_cliente* asociacionBuscada;
	info_resto* restoAsociado;
	respuesta_consultar_platos* platosRespondidos;
	int32_t nuevoSocketRestaurante;
	uint32_t sizePayload;
	codigo_operacion codigoRecibido;

	sem_wait(mutexListaAsociaciones);
	indiceAsociacionBuscada = buscarAsociacion(consultarPlatosRecibido->id);

	if(indiceAsociacionBuscada == -2){
		sem_wait(semLog);
	    log_error(logger, "[APP] Somehow al conectarse un cliente no registro una asociacion. Exploto en consultarPlatos.");
	    sem_post(semLog);
	    exit(-2);
	}

	asociacionBuscada = list_get(listaAsociaciones, indiceAsociacionBuscada);// busca el perfil del cliente en la lista de pedidos

	if(strcmp(asociacionBuscada->nombreRestaurante, "N/A") != 0){

	  //buscamos al resto (comprobamos que no sea el RestoDefault basicamente)
		indiceRestoBuscado = buscarRestaurante(asociacionBuscada->nombreRestaurante);

			if(indiceRestoBuscado != -2){
				sem_wait(mutexListaRestos);
				restoAsociado = list_get(listaRestos, indiceRestoBuscado);

				nuevoSocketRestaurante = establecer_conexion(restoAsociado->ip,restoAsociado->puerto);
				if(nuevoSocketRestaurante < 1){
				  sem_wait(semLog);
				  log_error(logger, "Fracase en consultarle los platos al restaurante < %s > porque no se encuentra levantado."
						 , restoAsociado->nombre_resto);
				  sem_post(semLog);
				  //exit(-2);
				}

                //reutilizo la estructura que me mandaron, tan forro no voy a ser
				mandar_mensaje(consultarPlatosRecibido, CONSULTAR_PLATOS, nuevoSocketRestaurante);

				bytesRecibidos(recv(nuevoSocketRestaurante, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
				bytesRecibidos(recv(nuevoSocketRestaurante, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

				if(codigoRecibido != RESPUESTA_CONSULTAR_PLATOS){
					sem_wait(semLog);
					log_error(logger, "El restaurante me contesto cualquier verdura en lugar de sus platos.");
					sem_post(semLog);
                    //exit(-2);
				}

				platosRespondidos = malloc(sizeof(respuesta_consultar_platos));
                recibir_mensaje(platosRespondidos, RESPUESTA_CONSULTAR_PLATOS, nuevoSocketRestaurante);
                close(nuevoSocketRestaurante);
                sem_post(mutexListaRestos);

                mandar_mensaje(platosRespondidos, RESPUESTA_CONSULTAR_PLATOS, socket_cliente);
                free(platosRespondidos->nombresPlatos);
                free(platosRespondidos);

				//recibir_respuesta(CONSULTAR_PLATOS,resto,cliente,socket_cliente);
				return;

			}else if(strcmp(asociacionBuscada->nombreRestaurante,"RestoDefault") == 0){
				platosRespondidos = malloc(sizeof(respuesta_consultar_platos));
				platosRespondidos->longitudNombresPlatos = strlen(platos_default);
				platosRespondidos->nombresPlatos = malloc(strlen(platos_default)+1);
				strcpy(platosRespondidos->nombresPlatos, platos_default);

				mandar_mensaje(platosRespondidos,RESPUESTA_CONSULTAR_PLATOS,socket_cliente);
				free(platosRespondidos->nombresPlatos);
				free(platosRespondidos);
			}
			sem_wait(semLog);
			log_error(logger,"[APP] El restaurante magicamente dejo de existir en nuestros registros");
			sem_post(semLog);
		}else{
			sem_wait(semLog);
			log_error(logger,"[APP] Un cliente ha intentado consultar platos sin seleccionar un restaurante.");
			sem_post(semLog);
		}
	sem_post(mutexListaAsociaciones);
}


void crearPedido(crear_pedido* crearPedidoRecibido, int32_t socket_cliente){
	int indiceAsociacionBuscada, indiceRestoAsociado;
	respuesta_crear_pedido* respuestaCreacion;
	respuesta_ok_error* respuestaComanda;
	perfil_pedido* elPedidoACrear;
	info_resto* restoAsociado;
	asociacion_cliente* asociacionBuscada;
	guardar_pedido* guardarPedidoRequerido;
	int32_t sizePayload = 0, nuevoSocketComanda, nuevoSocketRestaurante;
	codigo_operacion codigoRecibido;

	sem_wait(mutexListaAsociaciones);
	indiceAsociacionBuscada = buscarAsociacion(crearPedidoRecibido->id);

	asociacionBuscada = list_get(listaAsociaciones, indiceAsociacionBuscada);
	if(strcmp(asociacionBuscada->nombreRestaurante, "N/A") != 0){

		if(listaRestos->elements_count != 0 && strcmp(asociacionBuscada->nombreRestaurante,"RestoDefault") != 0){
			indiceRestoAsociado = buscarRestaurante(asociacionBuscada->nombreRestaurante);

			if(indiceRestoAsociado != -2){
				sem_wait(mutexListaRestos);
				restoAsociado = list_get(listaRestos, indiceRestoAsociado);

				nuevoSocketRestaurante = establecer_conexion(restoAsociado->ip,restoAsociado->puerto);
				if(nuevoSocketRestaurante < 1){
				  sem_wait(semLog);
				  log_error(logger, "Fracase en crear pedido al restaurante < %s > porque no se encuentra levantado."
						 , restoAsociado->nombre_resto);
				  sem_post(semLog);
				  //exit(-2);
				}
                mandar_mensaje(crearPedidoRecibido, CREAR_PEDIDO, nuevoSocketRestaurante);

                bytesRecibidos(recv(nuevoSocketRestaurante, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
				bytesRecibidos(recv(nuevoSocketRestaurante, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
				//if recibidos es cero o sale mal aca validaria

				respuestaCreacion = malloc(sizeof(respuesta_crear_pedido));
				recibir_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, nuevoSocketRestaurante);
				close(nuevoSocketRestaurante);

				if(respuestaCreacion->idPedido == 0){
					//fracaso en crearse el pedido, algo debe haber pasado con sindicato, tengo que denegar la operacion
					sem_wait(semLog);
				    log_error(logger, "Fracase en crear pedido al restaurante < %s > porque hubo un error en restaurante/sindic."
						 , restoAsociado->nombre_resto);
				    sem_post(semLog);
                    mandar_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, socket_cliente);
                    sem_post(mutexListaRestos);
                    sem_post(mutexListaAsociaciones);
                    free(respuestaCreacion);
                    return;
				}

				nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
				if(nuevoSocketComanda < 0){
					sem_wait(semLog);
					log_info(logger, "Comanda esta muerta, me muero yo tambien");
					sem_post(semLog);
					exit(-2);
				}

				guardarPedidoRequerido = malloc(sizeof(guardar_pedido));
				guardarPedidoRequerido->largoNombreRestaurante = strlen(asociacionBuscada->nombreRestaurante);
				guardarPedidoRequerido->nombreRestaurante = malloc(guardarPedidoRequerido->largoNombreRestaurante+1);
				strcpy(guardarPedidoRequerido->nombreRestaurante, asociacionBuscada->nombreRestaurante);
				guardarPedidoRequerido->idPedido = respuestaCreacion->idPedido;

				mandar_mensaje(guardarPedidoRequerido, GUARDAR_PEDIDO, nuevoSocketComanda);

				bytesRecibidos(recv(nuevoSocketComanda, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
				bytesRecibidos(recv(nuevoSocketComanda, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
				//if recibidos es cero o sale mal aca validaria

				respuestaComanda = malloc(sizeof(respuesta_ok_error));
				recibir_mensaje(respuestaComanda, RESPUESTA_GUARDAR_PEDIDO, nuevoSocketComanda);

				if(respuestaComanda->respuesta == 0){
					//comanda fallo en guardar el pedido en sus estructuras, tengo que abortar la operacion
					sem_wait(semLog);
					log_error(logger, "[APP] Fracase en crear pedido al restaurante < %s > porque hubo un error en comanda."
						 , restoAsociado->nombre_resto);
					sem_post(semLog);
					respuestaCreacion->idPedido = 0;
					mandar_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, socket_cliente);
					sem_post(mutexListaRestos);
					sem_post(mutexListaAsociaciones);
					free(respuestaCreacion);
					free(respuestaComanda);
					free(guardarPedidoRequerido->nombreRestaurante);
					free(guardarPedidoRequerido);
					return;
				}
				//si llegue hasta aca es porque salio bien la creacion tanto en resto/sindic como en comanda

				//RECIEN aca lo tendria que crear al pedido en sus estructuras internas la app
                elPedidoACrear = malloc(sizeof(perfil_pedido));
                elPedidoACrear->id_pedido_resto = respuestaCreacion->idPedido;
                elPedidoACrear->id_pedido_global = crearIDGlobalPedido();
                elPedidoACrear->socket_cliente = socket_cliente;
                elPedidoACrear->posClienteX = asociacionBuscada->posClienteX;
                elPedidoACrear->posClienteY = asociacionBuscada->posClienteY;
                elPedidoACrear->idCliente = malloc(strlen(asociacionBuscada->idCliente)+1);
                elPedidoACrear->nombreRestaurante = malloc(strlen(asociacionBuscada->nombreRestaurante)+1);
                strcpy(elPedidoACrear->idCliente, asociacionBuscada->idCliente);
                strcpy(elPedidoACrear->nombreRestaurante, asociacionBuscada->nombreRestaurante);

                sem_wait(semLog);
				log_trace(logger, "[APP] Exito al crear un pedido para el cliente < %s >. Sus credenciales corresponden a:"
						"ID Global <%d> / ID Restaurante <%d>."
					 , elPedidoACrear->idCliente, elPedidoACrear->id_pedido_global, elPedidoACrear->id_pedido_resto);
				sem_post(semLog);

        //esto lo hago para que el cliente reciba y se maneje con el id global, el que app reconoce univocamente
                respuestaCreacion->idPedido = elPedidoACrear->id_pedido_global;
                mandar_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, socket_cliente);

                sem_wait(mutexListaPedidos);
                list_add(listaPedidos, elPedidoACrear);
                sem_post(mutexListaPedidos);

				free(respuestaCreacion);
				free(respuestaComanda);
				free(guardarPedidoRequerido->nombreRestaurante);
				free(guardarPedidoRequerido);
				sem_post(mutexListaRestos);
			}

		}else if(strcmp(asociacionBuscada->nombreRestaurante,"RestoDefault") == 0){
			//no esta conectado ningun restaurante, se prueba app sola

			guardarPedidoRequerido = malloc(sizeof(guardar_pedido));
			guardarPedidoRequerido->idPedido = crearIDGlobalPedido();
			guardarPedidoRequerido->nombreRestaurante = malloc(strlen(asociacionBuscada->nombreRestaurante)+1);
			guardarPedidoRequerido->largoNombreRestaurante = strlen(asociacionBuscada->nombreRestaurante);
			strcpy(guardarPedidoRequerido->nombreRestaurante, asociacionBuscada->nombreRestaurante);

			nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
			if(nuevoSocketComanda < 0){
				sem_wait(semLog);
				log_info(logger, "Comanda esta muerta, me muero yo tambien");
				sem_post(semLog);
				exit(-2);
			}

			mandar_mensaje(guardarPedidoRequerido, GUARDAR_PEDIDO, nuevoSocketComanda);

			bytesRecibidos(recv(nuevoSocketComanda, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
			bytesRecibidos(recv(nuevoSocketComanda, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
			//if recibidos es cero o sale mal aca validaria

			respuestaComanda = malloc(sizeof(respuesta_ok_error));

			recibir_mensaje(respuestaComanda, RESPUESTA_GUARDAR_PEDIDO, nuevoSocketComanda);

			close(nuevoSocketComanda);

			respuestaCreacion = malloc(sizeof(respuesta_crear_pedido));

			if(respuestaComanda->respuesta == 0){
			//comanda fallo en guardar el pedido en sus estructuras, tengo que abortar la operacion
				sem_wait(semLog);
				log_error(logger, "Fracase en crear pedido para el cliente < %s > en el restaurante < %s > "
						"porque hubo un error en comanda."
						, asociacionBuscada->idCliente, "RestoDefault");
				sem_post(semLog);
				respuestaCreacion->idPedido = 0;
				mandar_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, socket_cliente);
				sem_post(mutexListaAsociaciones);
				free(respuestaComanda);
				free(respuestaCreacion);
				free(guardarPedidoRequerido->nombreRestaurante);
				free(guardarPedidoRequerido);
				return;
		    }
        //salio bien en comanda, tonces puedo devolver el id generado al cliente y dsp crearmelo yo
			respuestaCreacion->idPedido = guardarPedidoRequerido->idPedido;

			mandar_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, socket_cliente);

			elPedidoACrear = malloc(sizeof(perfil_pedido));
			elPedidoACrear->id_pedido_resto = respuestaCreacion->idPedido;
			elPedidoACrear->id_pedido_global = respuestaCreacion->idPedido;
			elPedidoACrear->socket_cliente = socket_cliente;
			elPedidoACrear->posClienteX = asociacionBuscada->posClienteX;
			elPedidoACrear->posClienteY = asociacionBuscada->posClienteY;
			elPedidoACrear->idCliente = malloc(strlen(asociacionBuscada->idCliente)+1);
			elPedidoACrear->nombreRestaurante = malloc(strlen(asociacionBuscada->nombreRestaurante)+1);
			strcpy(elPedidoACrear->idCliente, asociacionBuscada->idCliente);
			strcpy(elPedidoACrear->nombreRestaurante, asociacionBuscada->nombreRestaurante);

			sem_wait(mutexListaPedidos);
			list_add(listaPedidos, elPedidoACrear);
			sem_post(mutexListaPedidos);

            free(respuestaComanda);
			free(guardarPedidoRequerido->nombreRestaurante);
			free(guardarPedidoRequerido);
			free(respuestaCreacion);
		} else {
		respuestaCreacion = malloc(sizeof(respuesta_crear_pedido));
		respuestaCreacion->idPedido = -1;
		mandar_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, socket_cliente);
		free(respuestaCreacion);
		sem_wait(semLog);
		log_error(logger, "[APP] Problemas internos creando el pedido.");
		sem_post(semLog);
		}
	}else{
		respuestaCreacion = malloc(sizeof(respuesta_crear_pedido));
		respuestaCreacion->idPedido = -1;
		mandar_mensaje(respuestaCreacion, RESPUESTA_CREAR_PEDIDO, socket_cliente);
		free(respuestaCreacion);
		sem_wait(semLog);
		log_error(logger, "[APP] Un cliente ha intentado crear un pedido sin haber seleccionado un restaurante.");
		sem_post(semLog);
	}
	sem_post(mutexListaAsociaciones);
}

void aniadirPlato(a_plato* recibidoAPlato, int32_t socket_cliente){
	int indiceAsociacionBuscada, indicePedidoBuscado, indiceRestoAsociado;
	asociacion_cliente* asociacionBuscada;
	perfil_pedido* elPedidoAModificar;
	info_resto* restoAsociado;
	respuesta_ok_error* respuestaAniadir;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar, nuevoSocketComanda, nuevoSocketRestaurante;
	codigo_operacion cod_op;
	guardar_plato* pasamanosGuardarPlato;

	sem_wait(mutexListaAsociaciones);
	indiceAsociacionBuscada = buscarAsociacion(recibidoAPlato->id);
	asociacionBuscada = list_get(listaAsociaciones, indiceAsociacionBuscada);

	if(strcmp(asociacionBuscada->nombreRestaurante, "N/A") == 0){
		sem_wait(semLog);
		log_error(logger, "[APP] Dale pibe, enserio, dejate de joder y seleccioná un restaurante por favor.");
		sem_post(semLog);
		sem_post(mutexListaAsociaciones);
		return;
	}
	sem_post(mutexListaAsociaciones);

	//IMPORTANTEEEUEUEU!!!! Tendriamos que asegurar que el cliente una vez q creo un pedido/s,
	//no va a cambiar de seleccion de restaurante
	//hasta que dicho pedido/s finalicen porque sino la asociacion logica deja de tener sentido y es clave.
	//(ME MATA PERDER EL NOMBRE DEL RESTAURANTE PORQUE LO UNICO IRREPETIBLE PARA APP ES LA DUPLA IDPEDIDO/RESTAURANTE).
	//ESTO SERIA CONSIDERANDO QUE EL CLIENTE ME MANDE IDPEDIDO (EL QUE CONOCE EL RESTAURANTE), EN LUGAR DEL IDGLOBAL.

	//SI EL CLIENTE ME MANDA EL IDGLOBAL, IGNORAR TODA LA BOSTA QUE ESCRIBI ARRIBA.
	//VOY A ENCAMINARLO POR IDGLOBAL.

	sem_wait(mutexListaPedidos);
	indicePedidoBuscado = buscarPedidoPorIDGlobal(recibidoAPlato->idPedido);

	if(indicePedidoBuscado != -2){

		elPedidoAModificar = list_get(listaPedidos, indicePedidoBuscado);

		if(strcmp(elPedidoAModificar->nombreRestaurante,"RestoDefault") == 0){
			respuestaAniadir = malloc(sizeof(respuesta_ok_error));
			respuestaAniadir->respuesta = 0;

			nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
			if(nuevoSocketComanda < 0){
				sem_wait(semLog);
				log_info(logger, "Comanda esta muerta, me muero yo tambien");
				sem_post(semLog);
				exit(-2);
			}

			//antes de hacer nada me fijo si me mandaron un plato que el restoDefault no puede cocinar lmao
			int platoDisponible = 0;
			int i = 0;
			char** platosDelRestoDefault = string_get_string_as_array(platos_default);
			while(platosDelRestoDefault[i] != NULL){
                 if(strcmp(platosDelRestoDefault[i], recibidoAPlato->nombrePlato) == 0){
                	 platoDisponible = 1;
                	 break;
                 }
                 i++;
			}
			freeDeArray(platosDelRestoDefault);

            if(platoDisponible == 1){

			pasamanosGuardarPlato = malloc(sizeof(guardar_plato));
			pasamanosGuardarPlato->nombrePlato = malloc(strlen(recibidoAPlato->nombrePlato)+1);
			pasamanosGuardarPlato->largoNombrePlato = strlen(recibidoAPlato->nombrePlato);
			pasamanosGuardarPlato->nombreRestaurante = malloc(strlen(elPedidoAModificar->nombreRestaurante)+1);
			pasamanosGuardarPlato->largoNombreRestaurante = strlen(elPedidoAModificar->nombreRestaurante);
			strcpy(pasamanosGuardarPlato->nombrePlato, recibidoAPlato->nombrePlato);
			strcpy(pasamanosGuardarPlato->nombreRestaurante, elPedidoAModificar->nombreRestaurante);
			pasamanosGuardarPlato->idPedido = elPedidoAModificar->id_pedido_global;
			pasamanosGuardarPlato->cantidadPlatos = 1;

			mandar_mensaje(pasamanosGuardarPlato, GUARDAR_PLATO ,nuevoSocketComanda);

			recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
			if(recibidos >= 1){
				recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);
				recibir_mensaje(respuestaAniadir, RESPUESTA_GUARDAR_PLATO, nuevoSocketComanda);
				if(respuestaAniadir->respuesta == 0){
					sem_wait(semLog);
					log_error(logger, "[APP] Comanda fracaso al guardar un/a <%s> al pedido <%d>."
							, pasamanosGuardarPlato->nombrePlato, pasamanosGuardarPlato->idPedido);
					sem_post(semLog);
				} else {
					sem_wait(semLog);
					log_trace(logger, "[APP] Se aniadio un/a <%s> al pedido <%d> correctamente."
							, pasamanosGuardarPlato->nombrePlato, pasamanosGuardarPlato->idPedido);
					sem_post(semLog);
				}
				mandar_mensaje(respuestaAniadir, RESPUESTA_A_PLATO, socket_cliente);
			} else {
				sem_wait(semLog);
				log_error(logger, "[APP] Comanda se murio en el proceso de responder a un guardar_plato.");
				sem_post(semLog);
				respuestaAniadir->respuesta = 0;
				mandar_mensaje(respuestaAniadir, RESPUESTA_A_PLATO, socket_cliente);
			}

			close(nuevoSocketComanda);
			free(pasamanosGuardarPlato->nombrePlato);
			free(pasamanosGuardarPlato->nombreRestaurante);
			free(pasamanosGuardarPlato);
			free(respuestaAniadir);
		} else {
			sem_wait(semLog);
			log_error(logger, "[APP] Se ha intentado aniadir un plato que el RestoDefault no conoce.");
			sem_post(semLog);
			respuestaAniadir->respuesta = 0;
			mandar_mensaje(respuestaAniadir, RESPUESTA_A_PLATO, socket_cliente);
		}

		}else{
			indiceRestoAsociado = buscarRestaurante(elPedidoAModificar->nombreRestaurante);

			if(indiceRestoAsociado != -2){
				sem_wait(mutexListaRestos);
				restoAsociado = list_get(listaRestos,indiceRestoAsociado);
				nuevoSocketRestaurante = establecer_conexion(restoAsociado->ip,restoAsociado->puerto);
				sem_post(mutexListaRestos);
				if(nuevoSocketRestaurante < 0){
					sem_wait(semLog);
					log_error(logger, "Un restaurante con un pedido activo no esta levantado, me muero yo tambien");
					sem_post(semLog);
					exit(-2);
				}

				respuestaAniadir = malloc(sizeof(respuesta_ok_error));
				respuestaAniadir->respuesta = 0;

				recibidoAPlato->idPedido = elPedidoAModificar->id_pedido_resto;

				mandar_mensaje(recibidoAPlato, A_PLATO, nuevoSocketRestaurante);

				recibidos = recv(nuevoSocketRestaurante, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

				if(recibidos >= 1){
					recibidosSize = recv(nuevoSocketRestaurante, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
					bytesRecibidos(recibidosSize);
					recibir_mensaje(respuestaAniadir, RESPUESTA_A_PLATO, nuevoSocketRestaurante);
				} else{
					sem_wait(semLog);
					log_error(logger, "[APP] Un restaurante fallo en responderme sobre un aniadir plato, algo feo acontecio");
					sem_post(semLog);
					exit(-2);
				}
				close(nuevoSocketRestaurante);

				if(respuestaAniadir->respuesta == 1){
                //en restaurante/sindicato hubo exito, por lo que toca pedirle a comanda ahora
					nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
					if(nuevoSocketComanda < 0){
						sem_wait(semLog);
						log_info(logger, "Comanda esta muerta, me muero yo tambien");
						sem_post(semLog);
						exit(-2);
					}

					pasamanosGuardarPlato = malloc(sizeof(guardar_plato));
					pasamanosGuardarPlato->nombrePlato = malloc(strlen(recibidoAPlato->nombrePlato)+1);
					pasamanosGuardarPlato->largoNombrePlato = strlen(recibidoAPlato->nombrePlato);
					pasamanosGuardarPlato->nombreRestaurante = malloc(strlen(elPedidoAModificar->nombreRestaurante)+1);
					pasamanosGuardarPlato->largoNombreRestaurante = strlen(elPedidoAModificar->nombreRestaurante);
					strcpy(pasamanosGuardarPlato->nombrePlato, recibidoAPlato->nombrePlato);
					strcpy(pasamanosGuardarPlato->nombreRestaurante, elPedidoAModificar->nombreRestaurante);
					pasamanosGuardarPlato->idPedido = elPedidoAModificar->id_pedido_resto;
					pasamanosGuardarPlato->cantidadPlatos = 1;

					mandar_mensaje(pasamanosGuardarPlato,GUARDAR_PLATO,nuevoSocketComanda);

					recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
					if(recibidos >= 1){
						recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
						bytesRecibidos(recibidosSize);
						recibir_mensaje(respuestaAniadir, RESPUESTA_GUARDAR_PLATO, nuevoSocketComanda);
					}
					close(nuevoSocketComanda);
				     //Haya salido OK o FAIL en comanda, va a tener la ultima palabra, asi que lo mando asi como me vino
					 //y el cliente sabra de todas maneras el resultado definitivo
					mandar_mensaje(respuestaAniadir,RESPUESTA_A_PLATO, socket_cliente);
					free(pasamanosGuardarPlato->nombrePlato);
					free(pasamanosGuardarPlato->nombreRestaurante);
					free(pasamanosGuardarPlato);
				}else{
					//fallo en restaurante/sindicato, se manda FAIL
					mandar_mensaje(respuestaAniadir, RESPUESTA_A_PLATO, socket_cliente);
				}
				free(respuestaAniadir);
				close(nuevoSocketRestaurante);
			}

		}

	}else{
		sem_wait(semLog);
		log_error(logger, "[APP] Se ingreso un ID de pedido global equivocado al querer aniadir un plato.");
		sem_post(semLog);
		respuestaAniadir = malloc(sizeof(respuesta_ok_error));
		respuestaAniadir->respuesta = 0;
		mandar_mensaje(respuestaAniadir, RESPUESTA_A_PLATO, socket_cliente);
		free(respuestaAniadir);
	}
	sem_post(mutexListaPedidos);
}


void consultarPedido(consultar_pedido* elPedidoBuscado, int32_t socket_cliente){
	perfil_pedido* elPedidoQueQueremosConsultar;
	obtener_pedido* datosPedido;
	respuesta_obtener_pedido* elPedidoObtenido;
	respuesta_consultar_pedido* elPedidoYaConsultado;
	int32_t indiceDelPedidoAConsultar, nuevoSocketComanda;
	uint32_t sizePayload;
	codigo_operacion codigoRecibido;

	sem_wait(mutexListaPedidos);
	indiceDelPedidoAConsultar = buscarPedidoPorIDGlobal(elPedidoBuscado->idPedido);
    sem_post(mutexListaPedidos);

	if(indiceDelPedidoAConsultar == -2){
		sem_wait(semLog);
		log_error(logger, "[APP] Arribo una solicitud de consulta de un pedido que no existe en los registros...");
		sem_post(semLog);
	}

	sem_wait(mutexListaPedidos);
	elPedidoQueQueremosConsultar = list_get(listaPedidos, indiceDelPedidoAConsultar);

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	datosPedido = malloc(sizeof(obtener_pedido));

	datosPedido->largoNombreRestaurante = strlen(elPedidoQueQueremosConsultar->nombreRestaurante);
	datosPedido->nombreRestaurante = malloc(strlen(elPedidoQueQueremosConsultar->nombreRestaurante) +1);
	strcpy(datosPedido->nombreRestaurante,elPedidoQueQueremosConsultar->nombreRestaurante);

	if(strcmp(datosPedido->nombreRestaurante, "RestoDefault") == 0){
	    datosPedido->idPedido = elPedidoQueQueremosConsultar->id_pedido_global;
    }else{
    	datosPedido->idPedido = elPedidoQueQueremosConsultar->id_pedido_resto;
    }
	sem_post(mutexListaPedidos);

	mandar_mensaje(datosPedido, OBTENER_PEDIDO, nuevoSocketComanda);

    bytesRecibidos(recv(nuevoSocketComanda, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
    bytesRecibidos(recv(nuevoSocketComanda, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
    //aca validaria con if los recibidos y eso, dsp lo miro

    elPedidoObtenido = malloc(sizeof(respuesta_obtener_pedido));

    recibir_mensaje(elPedidoObtenido, RESPUESTA_OBTENER_PEDIDO, nuevoSocketComanda);
    close(nuevoSocketComanda);

    elPedidoYaConsultado = malloc(sizeof(respuesta_consultar_pedido));

    elPedidoYaConsultado->estado = elPedidoObtenido->estado;
    elPedidoYaConsultado->largoNombreRestaurante = strlen(elPedidoQueQueremosConsultar->nombreRestaurante);
    elPedidoYaConsultado->nombreRestaurante = malloc(strlen(elPedidoQueQueremosConsultar->nombreRestaurante)+1);
	strcpy(elPedidoYaConsultado->nombreRestaurante, elPedidoQueQueremosConsultar->nombreRestaurante);
	elPedidoYaConsultado->sizeComidas = elPedidoObtenido->sizeComidas;
	elPedidoYaConsultado->sizeCantListas = elPedidoObtenido->sizeCantListas;
	elPedidoYaConsultado->sizeCantTotales = elPedidoObtenido->sizeCantTotales;
	elPedidoYaConsultado->comidas = malloc(elPedidoYaConsultado->sizeComidas+1);
	elPedidoYaConsultado->cantListas = malloc(elPedidoYaConsultado->sizeCantListas+1);
	elPedidoYaConsultado->cantTotales = malloc(elPedidoYaConsultado->sizeCantTotales+1);
	strcpy(elPedidoYaConsultado->comidas, elPedidoObtenido->comidas);
	strcpy(elPedidoYaConsultado->cantListas, elPedidoObtenido->cantListas);
	strcpy(elPedidoYaConsultado->cantTotales, elPedidoObtenido->cantTotales);

    mandar_mensaje(elPedidoYaConsultado, RESPUESTA_CONSULTAR_PEDIDO, socket_cliente);

    free(datosPedido->nombreRestaurante);
    free(datosPedido);
    free(elPedidoObtenido->comidas);
    free(elPedidoObtenido->cantTotales);
    free(elPedidoObtenido->cantListas);
    free(elPedidoObtenido);
    free(elPedidoYaConsultado->nombreRestaurante);
    free(elPedidoYaConsultado->comidas);
    free(elPedidoYaConsultado->cantListas);
    free(elPedidoYaConsultado->cantTotales);
    free(elPedidoYaConsultado);
}


void confirmarPedido(confirmar_pedido* datosPedidoAConfirmar, int32_t socket_cliente){

	int indicePedidoAsociado, indiceRestoAsociado;
	perfil_pedido* elPedidoAConfirmar;
	obtener_pedido* elPedidoAObtener;
	respuesta_obtener_pedido* elPedidoObtenido;
	info_resto* restoAsociado;
	respuesta_ok_error* respuestaConfirmacion;
	pcb_pedido* nuevoPcb;
	int32_t recibidos, recibidosSize = 0, sizePayload, sizeAAllocar, nuevoSocketComanda, nuevoSocketRestaurante;
	codigo_operacion codigoRecibido, cod_op;
	confirmar_pedido* solicitudConfirmacion;

    sem_wait(mutexListaPedidos);
    indicePedidoAsociado = buscarPedidoPorIDGlobal(datosPedidoAConfirmar->idPedido);
    sem_post(mutexListaPedidos);

    respuestaConfirmacion = malloc(sizeof(respuesta_ok_error));

    sem_wait(mutexListaPedidos);
	if(indicePedidoAsociado != -2){
		elPedidoAConfirmar = list_get(listaPedidos, indicePedidoAsociado);
		elPedidoAConfirmar->socket_cliente = socket_cliente; //lo piso porque este socket es el que se va a quedar abierto
		//nada mas que para recibir las eventuales notificaciones que emite Restaurante, APP siendo el pasamanos.

		if(strcmp(elPedidoAConfirmar->nombreRestaurante,"RestoDefault") == 0){
        //EN ESTE CASO NO tengo que mandarle el CONFIRMAR_PEDIDO al restaurante/sindicato, el default es instantaneo
        //DIRECTAMENTE CHEQUEO CON COMANDA EL ESTADO
			nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
			if(nuevoSocketComanda < 0){
				sem_wait(semLog);
				log_info(logger, "Comanda esta muerta, me muero yo tambien");
				sem_post(semLog);
				exit(-2);
			}
			elPedidoAObtener = malloc(sizeof(obtener_pedido));
			elPedidoAObtener->idPedido = elPedidoAConfirmar->id_pedido_global;
			elPedidoAObtener->largoNombreRestaurante = strlen(elPedidoAConfirmar->nombreRestaurante);
			elPedidoAObtener->nombreRestaurante = malloc(strlen(elPedidoAConfirmar->nombreRestaurante)+1);
			strcpy(elPedidoAObtener->nombreRestaurante, elPedidoAConfirmar->nombreRestaurante);

			mandar_mensaje(elPedidoAObtener, OBTENER_PEDIDO, nuevoSocketComanda);

			recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

			if(recibidos >= 1){
				recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
				bytesRecibidos(recibidosSize);

				elPedidoObtenido = malloc(sizeof(respuesta_obtener_pedido));

				recibir_mensaje(elPedidoObtenido, RESPUESTA_OBTENER_PEDIDO,nuevoSocketComanda);

				close(nuevoSocketComanda);

				if(elPedidoObtenido->estado == PENDIENTE){
					//los datos en comanda estan correctos, podemos pedirle que confirme
					nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
					if(nuevoSocketComanda < 0){
						sem_wait(semLog);
						log_error(logger, "Comanda esta muerta, me muero yo tambien");
						sem_post(semLog);
						exit(-2);
					}

                    solicitudConfirmacion = malloc(sizeof(confirmar_pedido));
                    solicitudConfirmacion->idPedido = elPedidoAConfirmar->id_pedido_global;
                    solicitudConfirmacion->largoNombreRestaurante = strlen(elPedidoAConfirmar->nombreRestaurante);
                    solicitudConfirmacion->nombreRestaurante = malloc(strlen(elPedidoAConfirmar->nombreRestaurante)+1);
                    strcpy(solicitudConfirmacion->nombreRestaurante, elPedidoAConfirmar->nombreRestaurante);

                    mandar_mensaje(solicitudConfirmacion, CONFIRMAR_PEDIDO, nuevoSocketComanda);

                    bytesRecibidos(recv(nuevoSocketComanda, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
                    bytesRecibidos(recv(nuevoSocketComanda, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
                    //aca validaria con if los recibidos, ya se me esta haciendo eterno...

                    respuestaConfirmacion = malloc(sizeof(respuesta_ok_error));

                    recibir_mensaje(respuestaConfirmacion, RESPUESTA_CONFIRMAR_PEDIDO, nuevoSocketComanda);

                    close(nuevoSocketComanda);

                    if(respuestaConfirmacion->respuesta == 1){
                    	//deberia crear el pcb pedido y agregarlo a NEW, caso contrario no lo hago

						nuevoPcb = malloc(sizeof(pcb_pedido));

						nuevoPcb->pedidoID = elPedidoAConfirmar->id_pedido_resto;
						nuevoPcb->pedidoIDGlobal = elPedidoAConfirmar->id_pedido_global;
						nuevoPcb->posRestauranteX = posX_restoDefault;
						nuevoPcb->posRestauranteY = posY_restoDefault;
						nuevoPcb->posClienteX = elPedidoAConfirmar->posClienteX;
						nuevoPcb->posClienteY = elPedidoAConfirmar->posClienteY;
						//este es el socket de parte del cliente que se va a quedar esperando
						//los PLATO_LISTO que sean necesarios y un eventual FINALIZAR_PEDIDO
						//que APP tendra que validar, lo fijo para no perderlo
						nuevoPcb->socketCliente = socket_cliente;
						elPedidoAConfirmar->socket_cliente = socket_cliente;

						sem_wait(semLog);
						log_trace(logger, "[APP] Se confirmo el pedido de id global: <%d>, id restaur: <%d>"
								", del restaurante: <%s>."
								,elPedidoAConfirmar->id_pedido_global
								,elPedidoAConfirmar->id_pedido_resto
								,elPedidoAConfirmar->nombreRestaurante);
						sem_post(semLog);

						guardarPedidoListo(elPedidoAConfirmar);

						agregarANew(nuevoPcb);

						sem_post(mutexListaPedidos);
						sem_post(mutexListaRestos);
						free(elPedidoAObtener->nombreRestaurante);
						free(elPedidoAObtener);
						free(elPedidoObtenido->comidas);
						free(elPedidoObtenido->cantListas);
						free(elPedidoObtenido->cantTotales);
						free(elPedidoObtenido);
						free(solicitudConfirmacion->nombreRestaurante);
						free(solicitudConfirmacion);
						free(respuestaConfirmacion);
						return;
                    }

                    sem_wait(semLog);
					log_error(logger, "[APP] La confirmacion definitiva fracaso en comanda.");
					sem_post(semLog);

                    free(elPedidoAObtener->nombreRestaurante);
					free(elPedidoAObtener);
					free(elPedidoObtenido->comidas);
					free(elPedidoObtenido->cantListas);
					free(elPedidoObtenido->cantTotales);
					free(elPedidoObtenido);
					free(solicitudConfirmacion->nombreRestaurante);
					free(solicitudConfirmacion);
					free(respuestaConfirmacion);
					respuestaConfirmacion->respuesta = 0;
				}
				sem_wait(semLog);
				log_error(logger, "[APP] Se ha intentando confirmar un pedido que no se encuentra pendiente.");
				sem_post(semLog);

				free(elPedidoObtenido->comidas);
				free(elPedidoObtenido->cantListas);
				free(elPedidoObtenido->cantTotales);
				free(elPedidoObtenido);
				respuestaConfirmacion->respuesta = 0;

			}else{
				sem_wait(semLog);
				log_error(logger, "[APP] Ocurrio un error en comanda al obtener los datos del pedido para confirmar.");
				sem_post(semLog);
				close(nuevoSocketComanda);
                free(elPedidoAObtener->nombreRestaurante);
                free(elPedidoAObtener);
				free(respuestaConfirmacion);
				respuestaConfirmacion->respuesta = 0;
			}

		}else{
			//EN ESTE OTRO CASO, el restaurante asociado NO es el default, por lo que tengo que adicionalmente
			//solicitar confirmacion del pedido en el mismo (y por transitividad, en sindicato)
			sem_wait(mutexListaRestos);
			indiceRestoAsociado = buscarRestaurante(elPedidoAConfirmar->nombreRestaurante);

			if(indiceRestoAsociado != -2){
				restoAsociado = list_get(listaRestos, indiceRestoAsociado);

				solicitudConfirmacion = malloc(sizeof(confirmar_pedido));
				solicitudConfirmacion->idPedido = elPedidoAConfirmar->id_pedido_resto;
				solicitudConfirmacion->largoNombreRestaurante = strlen(restoAsociado->nombre_resto);
				solicitudConfirmacion->nombreRestaurante = malloc(strlen(restoAsociado->nombre_resto)+1);
				strcpy(solicitudConfirmacion->nombreRestaurante, restoAsociado->nombre_resto);

				nuevoSocketRestaurante = establecer_conexion(restoAsociado->ip, restoAsociado->puerto);
				if(nuevoSocketRestaurante < 0){
					sem_wait(semLog);
					log_info(logger, "Un restaurante al que le quiero confirmar pedido esta muerto, me muero yo tambien");
					sem_post(semLog);
					exit(-2);
				}
                sem_post(mutexListaRestos);

				mandar_mensaje(solicitudConfirmacion, CONFIRMAR_PEDIDO, nuevoSocketRestaurante);

				recibidos = recv(nuevoSocketRestaurante, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

				if(recibidos >= 1){
					recibidosSize = recv(nuevoSocketRestaurante, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
					bytesRecibidos(recibidosSize);
					recibir_mensaje(respuestaConfirmacion,RESPUESTA_CONFIRMAR_PEDIDO, nuevoSocketRestaurante);

				}else{
					sem_wait(semLog);
					log_error(logger,"[APP] El restaurante/sindicato murio durante el proceso de contestarme confirmacion.");
					sem_post(semLog);
				}

				if(respuestaConfirmacion->respuesta == 1){
				//SI SALIO BIEN EN EL RESTAURANTE/SINDICATO, ENTONCES PROSIGO A CHEQUEAR EL ESTADO CON COMANDA.
					nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
					if(nuevoSocketComanda < 0){
						sem_wait(semLog);
						log_info(logger, "Comanda esta muerta, me muero yo tambien");
						sem_post(semLog);
						exit(-2);
					}
					elPedidoAObtener = malloc(sizeof(obtener_pedido));
					elPedidoAObtener->idPedido = elPedidoAConfirmar->id_pedido_resto;
					elPedidoAObtener->largoNombreRestaurante = strlen(elPedidoAConfirmar->nombreRestaurante);
					elPedidoAObtener->nombreRestaurante = malloc(strlen(elPedidoAConfirmar->nombreRestaurante)+1);
					strcpy(elPedidoAObtener->nombreRestaurante, elPedidoAConfirmar->nombreRestaurante);

					mandar_mensaje(elPedidoAObtener, OBTENER_PEDIDO, nuevoSocketComanda);

					recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
					if(recibidos >= 1){
						recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
						bytesRecibidos(recibidosSize);

						elPedidoObtenido = malloc(sizeof(respuesta_obtener_pedido));

						recibir_mensaje(elPedidoObtenido,RESPUESTA_OBTENER_PEDIDO,nuevoSocketComanda);

						close(nuevoSocketComanda);

						if(elPedidoObtenido->estado == PENDIENTE){
							//los datos en comanda estan correctos, podemos pedirle que confirme
							nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
							if(nuevoSocketComanda < 0){
								sem_wait(semLog);
								log_info(logger, "Comanda esta muerta, me muero yo tambien");
								sem_post(semLog);
								exit(-2);
							}

							solicitudConfirmacion = malloc(sizeof(confirmar_pedido));
							solicitudConfirmacion->idPedido = elPedidoAConfirmar->id_pedido_resto;
							solicitudConfirmacion->largoNombreRestaurante = strlen(elPedidoAConfirmar->nombreRestaurante);
							solicitudConfirmacion->nombreRestaurante = malloc(strlen(elPedidoAConfirmar->nombreRestaurante)+1);
							strcpy(solicitudConfirmacion->nombreRestaurante, elPedidoAConfirmar->nombreRestaurante);

							mandar_mensaje(solicitudConfirmacion, CONFIRMAR_PEDIDO, nuevoSocketComanda);

							bytesRecibidos(recv(nuevoSocketComanda, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
							bytesRecibidos(recv(nuevoSocketComanda, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
							//aca validaria con if los recibidos, ya se me esta haciendo eterno...

							respuestaConfirmacion = malloc(sizeof(respuesta_ok_error));

							recibir_mensaje(respuestaConfirmacion, RESPUESTA_CONFIRMAR_PEDIDO, nuevoSocketComanda);

							close(nuevoSocketComanda);

							if(respuestaConfirmacion->respuesta == 1){
								//deberia crear el pcb pedido y agregarlo a NEW, caso contrario no lo hago

								nuevoPcb = malloc(sizeof(pcb_pedido));

								nuevoPcb->pedidoID = elPedidoAConfirmar->id_pedido_resto;
								nuevoPcb->pedidoIDGlobal = elPedidoAConfirmar->id_pedido_global;
								nuevoPcb->posRestauranteX = posX_restoDefault;
								nuevoPcb->posRestauranteY = posY_restoDefault;
								nuevoPcb->posClienteX = elPedidoAConfirmar->posClienteX;
								nuevoPcb->posClienteY = elPedidoAConfirmar->posClienteY;
								nuevoPcb->socketCliente = elPedidoAConfirmar->socket_cliente;

								agregarANew(nuevoPcb);

								sem_wait(semLog);
								log_trace(logger, "[APP] Se confirmo el pedido de id global: <%d>, id restaur: <%d>"
										", del restaurante: <%s>."
										,elPedidoAConfirmar->id_pedido_global
										,elPedidoAConfirmar->id_pedido_resto
										,elPedidoAConfirmar->nombreRestaurante);
								sem_post(semLog);

								mandar_mensaje(respuestaConfirmacion, RESPUESTA_CONFIRMAR_PEDIDO, socket_cliente);

								sem_post(mutexListaPedidos);
								sem_post(mutexListaRestos);
								free(elPedidoAObtener->nombreRestaurante);
								free(elPedidoAObtener);
								free(elPedidoObtenido->comidas);
								free(elPedidoObtenido->cantListas);
								free(elPedidoObtenido->cantTotales);
								free(elPedidoObtenido);
								free(solicitudConfirmacion->nombreRestaurante);
								free(solicitudConfirmacion);
								free(respuestaConfirmacion);
								return;
							}
							sem_wait(semLog);
							log_error(logger, "[APP] La confirmacion definitiva fracaso en comanda.");
							sem_post(semLog);

							free(elPedidoAObtener->nombreRestaurante);
							free(elPedidoAObtener);
							free(elPedidoObtenido->comidas);
							free(elPedidoObtenido->cantListas);
							free(elPedidoObtenido->cantTotales);
							free(elPedidoObtenido);
							free(solicitudConfirmacion->nombreRestaurante);
							free(solicitudConfirmacion);
							respuestaConfirmacion->respuesta = 0;
						}
						sem_wait(semLog);
						log_error(logger, "[APP] Se intento confirmar un pedido que no esta pendiente en comanda.");
						sem_post(semLog);
						free(solicitudConfirmacion->nombreRestaurante);
						free(solicitudConfirmacion);
						free(elPedidoAObtener->nombreRestaurante);
						free(elPedidoAObtener);
						free(elPedidoObtenido->comidas);
						free(elPedidoObtenido->cantListas);
						free(elPedidoObtenido->cantTotales);
						free(elPedidoObtenido);
						respuestaConfirmacion->respuesta = 0;
					}
					sem_wait(semLog);
					log_error(logger, "[APP] Hubo un problema recibiendo la respuesta de obtener el pedido en comanda...");
					sem_post(semLog);
					free(solicitudConfirmacion->nombreRestaurante);
					free(solicitudConfirmacion);
					free(elPedidoAObtener->nombreRestaurante);
					free(elPedidoAObtener);
                    respuestaConfirmacion->respuesta = 0;

				}else{
					sem_wait(semLog);
					log_error(logger, "[APP] La confirmacion fracaso en alguna parte de restaur/sindic.");
					sem_post(semLog);
					free(solicitudConfirmacion->nombreRestaurante);
					free(solicitudConfirmacion);
					respuestaConfirmacion->respuesta = 0;
				}

			}else{
				sem_wait(semLog);
				log_error(logger, "[APP] No se pudo encontrar el restaurante de un pedido ya creado...");
				sem_post(semLog);
				free(respuestaConfirmacion);
				respuestaConfirmacion->respuesta = 0;
			}
			sem_post(mutexListaRestos);
		}

	}else{
		sem_wait(semLog);
		log_error(logger, "[APP] No se pudo encontrar el pedido que se pretende confirmar.");
		sem_post(semLog);
		respuestaConfirmacion->respuesta = 0;
	}
	sem_post(mutexListaPedidos);
	mandar_mensaje(respuestaConfirmacion, RESPUESTA_CONFIRMAR_PEDIDO, socket_cliente);
	free(respuestaConfirmacion);
}


//generalmente me los va a mandar el restaurante, pero podria ser el cliente tambien
//el parametro "socket_cliente" en esta funcion, va a representar cual de ellos es
void platoListo(plato_listo* notificacionPlatoListo, int32_t socket_cliente){

	respuesta_ok_error* respuestaNotificacion;
	obtener_pedido* obtenerPedidoRequerido;
	respuesta_obtener_pedido* pedidoObtenido;
	int32_t recibidos, recibidosSize = 0, sizeAAllocar, sizePayload, nuevoSocketComanda;
	codigo_operacion cod_op, codigoRecibido;
	perfil_pedido* elPedidoBuscado;
	int indicePedidoBuscado = 0;

	respuestaNotificacion = malloc(sizeof(respuesta_ok_error));
	respuestaNotificacion->respuesta = 0;

	sem_wait(mutexListaPedidos);
	//como el ID que me ingresa puede venir desde cliente (global) o desde restaurante (id_resto), tengo que buscarlo
	//de una manera un poco diferente, ver si es estrictamente necesario
	indicePedidoBuscado = buscarPedidoPorIDRestoYNombreResto(notificacionPlatoListo->idPedido, notificacionPlatoListo->nombreRestaurante);
    sem_post(mutexListaPedidos);

	if(indicePedidoBuscado == -2){
		sem_wait(semLog);
		log_error(logger, "[APP] Arribo una notificacion de un plato listo de un pedido que no existe en los registros...");
		sem_post(semLog);
		return;
	}

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	mandar_mensaje(notificacionPlatoListo, PLATO_LISTO, nuevoSocketComanda);

	recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
	if(recibidos >= 1){
		recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
		bytesRecibidos(recibidosSize);
		recibir_mensaje(respuestaNotificacion, RESPUESTA_PLATO_LISTO,nuevoSocketComanda);

		close(nuevoSocketComanda);

		if(respuestaNotificacion->respuesta == 1){
			//comanda pudo actualizar las cantListas correctamente, entonces ahora chequeo a ver que onda si estan
			//todas las comidas cocinadas

			nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
			if(nuevoSocketComanda < 0){
				sem_wait(semLog);
				log_error(logger, "Comanda esta muerta, me muero yo tambien");
				sem_post(semLog);
				exit(-2);
			}

			obtenerPedidoRequerido = malloc(sizeof(obtener_pedido));
			obtenerPedidoRequerido->largoNombreRestaurante = notificacionPlatoListo->largoNombreRestaurante;
			obtenerPedidoRequerido->nombreRestaurante = malloc(strlen(notificacionPlatoListo->nombreRestaurante)+1);
			strcpy(obtenerPedidoRequerido->nombreRestaurante,notificacionPlatoListo->nombreRestaurante);
			obtenerPedidoRequerido->idPedido = notificacionPlatoListo->idPedido;

			mandar_mensaje(obtenerPedidoRequerido, OBTENER_PEDIDO, nuevoSocketComanda);

			pedidoObtenido = malloc(sizeof(respuesta_obtener_pedido));

			bytesRecibidos(recv(nuevoSocketComanda, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
			bytesRecibidos(recv(nuevoSocketComanda, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
			//aca validaria con if los recibidos, ya se me esta haciendo eterno...

			recibir_mensaje(pedidoObtenido, RESPUESTA_OBTENER_PEDIDO, nuevoSocketComanda);
			close(nuevoSocketComanda);

			//chequeo si cantTotales equivale a cantListas, caso afirmativo el pedido esta listo para ser procesado
			//por un repartidor
			char** listaComidas = string_get_string_as_array(pedidoObtenido->comidas);
			char** listaComidasTotales = string_get_string_as_array(pedidoObtenido->cantTotales);
			char** listaComidasListas = string_get_string_as_array(pedidoObtenido->cantListas);
			int i = 0;
			int contadorTotales = 0;
			int contadorListas = 0;

			//puedo usar cualquiera de las dos tranquilamente, son 2 arrays de char de exactamente la misma longitud
			while(listaComidasTotales[i] != NULL){
				contadorTotales += atoi(listaComidasTotales[i]);
				contadorListas += atoi(listaComidasListas[i]);
			    i++;
			}
			//el pedido esta terminado
			if(contadorTotales == contadorListas){

				sem_wait(mutexListaPedidos);
				elPedidoBuscado = list_get(listaPedidos, indicePedidoBuscado);
				sem_wait(mutexPedidosListos);
				guardarPedidoListo(elPedidoBuscado);
				sem_post(mutexPedidosListos);
				sem_post(mutexListaPedidos);

				sem_wait(semLog);
				log_info(logger, "[APP] El pedido <%d> del restaurante <%s> ha sido cocinado en su totalidad."
						, notificacionPlatoListo->idPedido, notificacionPlatoListo->nombreRestaurante);
				sem_post(semLog);
			} else {
			//el pedido no esta terminado, no hago nada especial
			}

			respuestaNotificacion->respuesta = 1;

			freeDeArray(listaComidas);
			freeDeArray(listaComidasTotales);
			freeDeArray(listaComidasListas);
			free(obtenerPedidoRequerido->nombreRestaurante);
			free(obtenerPedidoRequerido);
			free(pedidoObtenido->comidas);
			free(pedidoObtenido->cantListas);
			free(pedidoObtenido->cantTotales);
			free(pedidoObtenido);
		} else {
			sem_wait(semLog);
			log_error(logger, "[APP] Comanda tuvo un problema actualizando las comidas listas.");
			sem_post(semLog);
		}
	} else {
		sem_wait(semLog);
		log_error(logger, "[APP] Comanda murio antes de intentar mandarme la respuesta a la notificacion plato listo.");
		sem_post(semLog);
		close(nuevoSocketComanda);
	}
	mandar_mensaje(respuestaNotificacion, RESPUESTA_PLATO_LISTO, socket_cliente);
	free(respuestaNotificacion);
}



// TODO | Revisar despues si esto sirve para el cliente
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


// agrega un restaurante que se conecto a app a la lista de restaurantes
void registrarRestaurante(agregar_restaurante* recibidoAgregarRestaurante, int32_t socket_cliente){
	int idResto;
	info_resto* nuevoRestaurante;
	respuesta_ok_error* respuestaRegistro;
	int correspondeAgregar = 0;

	respuestaRegistro = malloc(sizeof(respuesta_ok_error));

	if(listaRestos->elements_count != 0){
		idResto = buscarRestaurante(recibidoAgregarRestaurante->nombreRestaurante);

			if(idResto != -2){ // se encontro el restaurante en la lista de antes
				sem_wait(semLog);
				log_error(logger, "[APP] Se intento agregar al restaurante < %s >, pero ya existe en los registros."
						, recibidoAgregarRestaurante->nombreRestaurante);
				sem_post(semLog);
				respuestaRegistro->respuesta=0;
				mandar_mensaje(respuestaRegistro, RESPUESTA_AGREGAR_RESTAURANTE, socket_cliente);
				return;
			}else{
				correspondeAgregar = 1;
			}

		}
	    correspondeAgregar = 1;


	    if(correspondeAgregar == 1){

			nuevoRestaurante = malloc(sizeof(info_resto));
			nuevoRestaurante->nombre_resto = malloc(recibidoAgregarRestaurante->largoNombreRestaurante+1);
			nuevoRestaurante->ip = malloc(recibidoAgregarRestaurante->largoIp+1);
			nuevoRestaurante->puerto = malloc(recibidoAgregarRestaurante->largoPuerto+1);
			strcpy(nuevoRestaurante->nombre_resto, recibidoAgregarRestaurante->nombreRestaurante);
			strcpy(nuevoRestaurante->ip, recibidoAgregarRestaurante->ip);
			strcpy(nuevoRestaurante->puerto, recibidoAgregarRestaurante->puerto);
			nuevoRestaurante->posX = recibidoAgregarRestaurante->posX;
			nuevoRestaurante->posY = recibidoAgregarRestaurante->posY;
			sem_wait(mutexListaRestos);
			list_add(listaRestos,nuevoRestaurante);
			sem_post(mutexListaRestos);
			sem_wait(semLog);
			log_trace(logger, "[APP] Se agrego al restaurante < %s > en los registros satisfactoriamente."
					, recibidoAgregarRestaurante->nombreRestaurante);
			sem_post(semLog);
			respuestaRegistro->respuesta = 1;
			mandar_mensaje(respuestaRegistro, RESPUESTA_AGREGAR_RESTAURANTE, socket_cliente);
	    }
     free(respuestaRegistro);
}

void registrarHandshake(handshake* recibidoHandshake, int32_t socket_cliente){
	asociacion_cliente* nuevaAsociacion;
	int indiceAsociacionPrevia;

	sem_wait(mutexListaAsociaciones);
	indiceAsociacionPrevia = buscarAsociacion(recibidoHandshake->id);
	sem_post(mutexListaAsociaciones);

	if(indiceAsociacionPrevia != -2){
		sem_wait(semLog);
		log_error(logger, "[APP] Se intento crear una asociacion para el: < %s >, pero ya existe una de antes.", recibidoHandshake->id);
		sem_post(semLog);
		return;
	}

	nuevaAsociacion = malloc(sizeof(asociacion_cliente));
	nuevaAsociacion->posClienteX = recibidoHandshake->posX;
	nuevaAsociacion->posClienteY = recibidoHandshake->posY;
	nuevaAsociacion->idCliente = malloc(strlen(recibidoHandshake->id)+1);
	nuevaAsociacion->nombreRestaurante = malloc(strlen("N/A")+1);
	strcpy(nuevaAsociacion->idCliente, recibidoHandshake->id);
	strcpy(nuevaAsociacion->nombreRestaurante, "N/A");

	sem_wait(mutexListaAsociaciones);
	list_add(listaAsociaciones, nuevaAsociacion);
	sem_post(mutexListaAsociaciones);

	sem_wait(semLog);
	log_trace(logger, "[APP] Se creo exitosamente una asociacion para el: < %s >.", recibidoHandshake->id);
	sem_post(semLog);
}

// **************************************FIN MENSAJES**************************************

int32_t crearIDGlobalPedido(){
	sem_wait(semId);
	id_global += 1;
	sem_post(semId);
	return id_global;
}

int buscarPedidoPorIDGlobal(uint32_t idPedidoSolicitado){
	perfil_pedido* pedidoBuscado;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		pedidoBuscado = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if(pedidoBuscado->id_pedido_global == idPedidoSolicitado){
			return i;
		}
	}
	return -2;
}

int buscarPedidoPorIDRestoYNombreResto(uint32_t idPedidoSolicitado, char* restauranteAsociado){
	perfil_pedido* pedidoBuscado;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		pedidoBuscado = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if(pedidoBuscado->id_pedido_resto == idPedidoSolicitado && strcmp(pedidoBuscado->nombreRestaurante, restauranteAsociado) == 0){
			return i;
		}
	}
	return -2;
}

int buscarAsociacion(char* idClienteBuscado){
	asociacion_cliente* unaAsociacion;
	for(int i = 0; i < listaAsociaciones->elements_count; i++){
		unaAsociacion = list_get(listaAsociaciones,i);

		if(strcmp(unaAsociacion->idCliente, idClienteBuscado) == 0){
			return i;
		}
	}
	return -2;
}

int buscarRestaurante(char* nombreResto){
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


/*
//maneja todos los flujos de mensajes que requieran respuestas
void recibir_respuesta(codigo_operacion cod_op, info_resto* resto, perfil_cliente* cliente, int32_t socketCliente){
	int32_t socketRespuestas;
	respuesta_ok_error* respuesta;
	guardar_pedido* estructura_guardar_pedido;
	//----esto es porque la serializacion de consultar_platos reutiliza la de obtener_restaurante, son lo mismo----
	obtener_restaurante* mensajeConsultarPlatos;
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
				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socketCliente);
				cliente->id_pedido_resto = estructura_idPedido->idPedido;

			}else{
				respuesta->respuesta = 0;
				mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socketCliente);
			}
			free(estructura_guardar_pedido->nombreRestaurante);
			free(estructura_guardar_pedido);
			free(respuesta);

		}else{
			respuesta = malloc(sizeof(respuesta_ok_error));
			respuesta->respuesta = 0;
			mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socketCliente);
			free(respuesta);
		}
		close(socketRespuestas);
		close(nuevoSocketComanda);
		break;

	case CONSULTAR_PLATOS:

		socketRespuestas = establecer_conexion(resto->ip,resto->puerto);

		mensajeConsultarPlatos = malloc(sizeof(obtener_restaurante));
		mensajeConsultarPlatos->largoNombreRestaurante = strlen(resto->nombre_resto);
		mensajeConsultarPlatos->nombreRestaurante = malloc(strlen(resto->nombre_resto)+1);
		strcpy(mensajeConsultarPlatos->nombreRestaurante, resto->nombre_resto);

		mandar_mensaje(mensajeConsultarPlatos,CONSULTAR_PLATOS, socketRespuestas);

		recibidos = recv(socketRespuestas, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);

		if(recibidos >= 1){
			recibidosSize = recv(socketRespuestas, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
			bytesRecibidos(recibidosSize);

			estructura_consultar_platos = malloc(sizeAAllocar);

			recibir_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS,socketRespuestas);

			mandar_mensaje(estructura_consultar_platos,RESPUESTA_CONSULTAR_PLATOS, socketCliente);

			free(estructura_consultar_platos->nombresPlatos);
			free(estructura_consultar_platos);
		} else{
			printf("Hubo un problema al recibir consultar platos del restaurante");
		}

		free(mensajeConsultarPlatos->nombreRestaurante);
		free(mensajeConsultarPlatos);
		close(socketRespuestas);
		break;

	default:
		break;
	}
}
*/


//************* FUNCIONES DE SERVER *************

void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar)  {
	agregar_restaurante* recibidoAgregarRestaurante;
	seleccionar_restaurante* recibidoSeleccionarRestaurante;
	a_plato* recibidoAPlato;
	plato_listo* recibidoPlatoListo ;
	confirmar_pedido* recibidoConfirmarPedido;
	consultar_pedido* recibidoConsultarPedido;
	handshake* recibidoHandshake;
	consultar_platos* recibidoConsultarPlatos;
	crear_pedido* recibidoCrearPedido;

	/*
	 *   STATUS Manejo Mensajes APP 2.0
	 *
	 * CONSULTAR_RESTAURANTES -> Done + Tested
	 * SELECCIONAR_RESTAURANTES -> Done + Tested
	 * CONSULTAR_PLATOS -> Done + Tested
	 * CREAR_PEDIDO -> Done + Tested
	 * ANIADIR_PLATO -> Done + Tested
	 * CONSULTAR_PEDIDO -> Done +
	 * CONFIRMAR_PEDIDO -> Done +
	 * PLATO_LISTO -> Done +
	 * REGISTRO HANDSHAKE/ASOCIACION -> Done + Tested
	 * AGREGAR_RESTAURANTE -> Done + Tested
	 *
	 */


	switch (cod_op) {
		case CONSULTAR_RESTAURANTES:
			consultarRestaurantes(socket_cliente);
			break;

		case SELECCIONAR_RESTAURANTE:
			recibidoSeleccionarRestaurante = malloc(sizeof(seleccionar_restaurante));
			recibir_mensaje(recibidoSeleccionarRestaurante,SELECCIONAR_RESTAURANTE,socket_cliente);
			seleccionarRestaurante(recibidoSeleccionarRestaurante,socket_cliente);
			free(recibidoSeleccionarRestaurante->idCliente);
			free(recibidoSeleccionarRestaurante->nombreRestaurante);
			free(recibidoSeleccionarRestaurante);
			break;

		case CONSULTAR_PLATOS:
			recibidoConsultarPlatos = malloc(sizeof(consultar_platos));
			recibir_mensaje(recibidoConsultarPlatos,CONSULTAR_PLATOS,socket_cliente);
			consultarPlatos(recibidoConsultarPlatos,socket_cliente);
			free(recibidoConsultarPlatos->id);
			free(recibidoConsultarPlatos->nombreResto);
			free(recibidoConsultarPlatos);
			break;

		case CREAR_PEDIDO:
			recibidoCrearPedido = malloc(sizeof(crear_pedido));
			recibir_mensaje(recibidoCrearPedido,CREAR_PEDIDO,socket_cliente);
			crearPedido(recibidoCrearPedido,socket_cliente);
			free(recibidoCrearPedido->id);
			free(recibidoCrearPedido);
			break;

		case A_PLATO:
			recibidoAPlato = malloc(sizeof(a_plato));
			recibir_mensaje(recibidoAPlato,A_PLATO,socket_cliente);
			aniadirPlato(recibidoAPlato, socket_cliente);
			free(recibidoAPlato->nombrePlato);
			free(recibidoAPlato->id);
			free(recibidoAPlato);
			break;

		case PLATO_LISTO:
			recibidoPlatoListo = malloc(sizeof(plato_listo));
			recibir_mensaje(recibidoPlatoListo,PLATO_LISTO,socket_cliente);
			platoListo(recibidoPlatoListo,socket_cliente);
			free(recibidoPlatoListo); //todo ver si esto no rompe nada
			break;

		case CONFIRMAR_PEDIDO:
			recibidoConfirmarPedido = malloc(sizeof(guardar_pedido));
			recibir_mensaje(recibidoConfirmarPedido,CONFIRMAR_PEDIDO,socket_cliente);
			confirmarPedido(recibidoConfirmarPedido, socket_cliente);
			free(recibidoConfirmarPedido);
			break;

		case CONSULTAR_PEDIDO:
            recibidoConsultarPedido = malloc(sizeof(consultar_pedido));
            recibir_mensaje(recibidoConsultarPedido,CONSULTAR_PEDIDO, socket_cliente);
            consultarPedido(recibidoConsultarPedido, socket_cliente);
            free(recibidoConsultarPedido);
			break;

		case AGREGAR_RESTAURANTE:
			recibidoAgregarRestaurante = malloc(sizeof(agregar_restaurante));
			recibir_mensaje(recibidoAgregarRestaurante,AGREGAR_RESTAURANTE,socket_cliente);
			registrarRestaurante(recibidoAgregarRestaurante, socket_cliente);
			free(recibidoAgregarRestaurante->nombreRestaurante);
			free(recibidoAgregarRestaurante->ip);
			free(recibidoAgregarRestaurante->puerto);
			free(recibidoAgregarRestaurante);
			break;

		case HANDSHAKE:
			recibidoHandshake = malloc(sizeof(handshake));
			recibir_mensaje(recibidoHandshake,HANDSHAKE,socket_cliente);
			registrarHandshake(recibidoHandshake, socket_cliente);
			free(recibidoHandshake->id);
			free(recibidoHandshake);
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
}

void esperar_cliente(int32_t socket_servidor)
{
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	pthread_t thread;
	int32_t* socket_cliente = malloc(sizeof(int32_t));
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

