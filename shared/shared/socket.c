#include "socket.h"

int32_t reservarSocket(char* miPuerto)
{
	uint32_t miPuertoInt = cambia_a_int(miPuerto); //pasa a int la key del config

	//preparo una direccion con estas configuraciones
	struct sockaddr_in direccionMiSocket;
	direccionMiSocket.sin_family = PF_INET;
	direccionMiSocket.sin_addr.s_addr = INADDR_ANY;
	direccionMiSocket.sin_port = htons(miPuertoInt); //puerto en el que queremos que escuche

	//me armo el socket en el que quiero escuchar
	int32_t miSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); //con estos parametros le decimos que use el protocolo TCP/IP

	//este cacho de codigo es para que si se cierra mal el programa, se pueda reutilizar el socket sin problema
	uint32_t verdadero = 1;
	setsockopt(miSocket, SOL_SOCKET, SO_REUSEADDR, &verdadero, sizeof(verdadero));

	//asocio el socket que creamos con la direccion(el puerto) que arme antes
	if(bind(miSocket, (void*) &direccionMiSocket, sizeof(direccionMiSocket)) != 0)
	{
		puts("El socket no se pudo asociar correctamente (Es posible que esté en uso).");
		return 1;
	}

	puts("Socket reservado correctamente.");
	listen(miSocket, SOMAXCONN); //SOMAXCONN podria ser un numero (Nº max de conexiones en la cola), pero el parametro significa que tiene el tamaño maximo de cola

	return miSocket;
}

int32_t crearSocketServidor(char* ip, char* puerto){

	int socket_servidor;
	int activo = 1;

	    struct addrinfo hints, *servinfo, *p;

	    memset(&hints, 0, sizeof(hints));
	    hints.ai_family = PF_INET;
	    hints.ai_socktype = SOCK_STREAM;
	    hints.ai_flags = IPPROTO_TCP;

	    getaddrinfo(ip, puerto, &hints, &servinfo);

	    for (p=servinfo; p != NULL; p = p->ai_next)
	    {
	        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
	            continue;

	        //para que pueda reusar el socket si se cae ToDo esto serviria ahora?
	        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activo,sizeof(activo));

	        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
	            close(socket_servidor);
	            continue;
	        }
	        break;
	    }

		listen(socket_servidor, SOMAXCONN);
	    freeaddrinfo(servinfo);

	    return socket_servidor;
}

int32_t establecer_conexion(char* ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info = malloc(sizeof(struct addrinfo)); //malloc forro!

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = IPPROTO_TCP;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int32_t socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		printf("Error al intentar conectarse.\n");
		socket_cliente = -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void resultado_de_conexion(int32_t socket, t_log* logger, char* modulo)
{
	if(socket == -1)
	{
		log_warning(logger, "Conexión fallida con socket de %s.", modulo);
	}
	else
	{
		log_info(logger, "Conexión establecida con socket de %s.", modulo);
	}
}

void cerrar_conexion(int32_t socket)
{
	close(socket);
}

void mandar_mensaje(void* mensaje, codigo_operacion tipoMensaje, int32_t socket)
{
	t_paquete* paquete_por_armar = malloc(sizeof(t_paquete));
	paquete_por_armar->buffer = malloc(sizeof(t_buffer));
	uint32_t size_serializado = 0;

	//printf("Voy a mandar un mensaje del tipo %i.\n", tipoMensaje);

	//preparo el paquete para mandar
	void* paquete_serializado = serializar_paquete(paquete_por_armar, mensaje, tipoMensaje, &size_serializado);

	//mando el mensaje
	bytesEnviados(send(socket, paquete_serializado, size_serializado, 0));

	//libero los malloc utilizados
	eliminar_paquete(paquete_por_armar);
	free(paquete_serializado);
}

void* serializar_paquete(t_paquete* paquete, void* mensaje, codigo_operacion tipoMensaje, uint32_t* size_serializado)
{
	uint32_t size_ya_armado = 0;
	paquete->codigo_op = tipoMensaje;

	void* buffer_serializar;//aca se va a guardar el choclo ya armado

	switch(tipoMensaje){
		case CONSULTAR_RESTAURANTES: //este se pasa el mensaje por el culo, solo manda el codigo de operacion
			paquete->buffer->stream = NULL; //malloc flashero para que no rompa despues con el free
			paquete->buffer->size = 0;
			size_ya_armado = sizeof(tipoMensaje);
			break;

		case SELECCIONAR_RESTAURANTE:
			//paquete->buffer->stream = malloc(sizeof(seleccionar_restaurante));
			size_ya_armado = serializar_paquete_seleccionar_restaurante(paquete, mensaje);
			break;

		case OBTENER_RESTAURANTE:
			//paquete->buffer->stream = malloc(sizeof(obtener_restaurante));
			size_ya_armado = serializar_paquete_obtener_restaurante(paquete, mensaje);
			break;

		case CONSULTAR_PLATOS:
			//paquete->buffer->stream = malloc(sizeof(obtener_restaurante));
			size_ya_armado = serializar_paquete_consultar_platos(paquete, mensaje);
			break;

		case GUARDAR_PLATO:
			//paquete->buffer->stream = malloc(sizeof(guardar_plato));
			size_ya_armado = serializar_paquete_guardar_plato(paquete, mensaje);
			break;

		case A_PLATO:
			size_ya_armado = serializar_paquete_aniadir_plato(paquete, mensaje);
			break;

		case PLATO_LISTO:
			size_ya_armado = serializar_paquete_plato_listo(paquete, mensaje);
			break;

		case CREAR_PEDIDO: //este se pasa el mensaje por el culo, solo manda el codigo de operacion
			size_ya_armado = serializar_paquete_crear_pedido(paquete, mensaje);
			break;

		case GUARDAR_PEDIDO:
		//paquete->buffer->stream = malloc(sizeof(guardar_pedido));
			size_ya_armado = serializar_paquete_guardar_pedido(paquete, mensaje);
			break;

		//ATENCION!!! REUTILIZAMOS SERIALIZAR DE GUARDAR_PEDIDO
		case CONFIRMAR_PEDIDO:
			size_ya_armado = serializar_paquete_guardar_pedido(paquete, mensaje);
			break;

		case CONSULTAR_PEDIDO:
            size_ya_armado = serializar_paquete_consultar_pedido(paquete, mensaje);
			break;

	    //ATENCION!!! REUTILIZAMOS SERIALIZAR DE GUARDAR_PEDIDO
		case OBTENER_PEDIDO:
			size_ya_armado = serializar_paquete_guardar_pedido(paquete, mensaje);
			break;

		//ATENCION!!! REUTILIZAMOS SERIALIZAR DE GUARDAR_PEDIDO
		case FINALIZAR_PEDIDO:
			size_ya_armado = serializar_paquete_guardar_pedido(paquete, mensaje);
			break;

		//ATENCION!!! REUTILIZAMOS SERIALIZAR DE GUARDAR_PEDIDO
		case TERMINAR_PEDIDO:
			size_ya_armado = serializar_paquete_guardar_pedido(paquete, mensaje);
			break;

		case OBTENER_RECETA:
			size_ya_armado = serializar_paquete_obtener_receta(paquete, mensaje);
			break;

		case HANDSHAKE:
			size_ya_armado = serializar_paquete_handshake(paquete, mensaje);
			break;

		case RESPUESTA_CONSULTAR_R:
	        size_ya_armado = serializar_paquete_respuesta_consultar_restaurantes(paquete, mensaje);
	        break;

		case RESPUESTA_SELECCIONAR_R:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_OBTENER_REST:
			//paquete->buffer->stream = malloc(sizeof(respuesta_obtener_restaurante));
			size_ya_armado = serializar_paquete_respuesta_obtener_restaurante(paquete, mensaje);
			break;

		case RESPUESTA_CONSULTAR_PLATOS:
			//paquete->buffer->stream = malloc(sizeof(respuesta_consultar_platos));
			size_ya_armado = serializar_paquete_respuesta_consultar_platos(paquete, mensaje);
			break;

		case RESPUESTA_GUARDAR_PLATO:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_A_PLATO:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_PLATO_LISTO:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_CREAR_PEDIDO:
			//paquete->buffer->stream = malloc(sizeof(respuesta_crear_pedido));
			size_ya_armado = serializar_paquete_respuesta_crear_pedido(paquete, mensaje);
			break;

		case RESPUESTA_GUARDAR_PEDIDO:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_CONFIRMAR_PEDIDO:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_CONSULTAR_PEDIDO:
            size_ya_armado = serializar_paquete_respuesta_consultar_pedido(paquete, mensaje);
			break;

		case RESPUESTA_OBTENER_PEDIDO:
			size_ya_armado = serializar_paquete_respuesta_obtener_pedido(paquete, mensaje);
			break;

		case RESPUESTA_FINALIZAR_PEDIDO:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_TERMINAR_PEDIDO:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		case RESPUESTA_OBTENER_RECETA:
            size_ya_armado = serializar_paquete_respuesta_obtener_receta(paquete, mensaje);
			break;

		case AGREGAR_RESTAURANTE:
			size_ya_armado = serializar_paquete_agregar_restaurante(paquete, mensaje);
			break;

		case RESPUESTA_AGREGAR_RESTAURANTE:
			paquete->buffer->stream = malloc(sizeof(respuesta_ok_error));
			size_ya_armado = serializar_paquete_ok_fail(paquete, mensaje);
			break;

		default:
			puts("\n\n\nATENCION: Switch de serializar_paquete pasó por el caso default.\n\n\n");
			break;
	}

	//ahora me preparo para meter en el buffer "posta", el choclo que va a enviar mandar_mensaje
	//printf("El size que tiene armado es: %i \n", size_ya_armado);
	buffer_serializar = malloc(size_ya_armado);
	uint32_t desplazamiento = 0;

	//meto en el buffer el tipo de mensaje que voy a mandar
	memcpy(buffer_serializar  + desplazamiento, &(paquete->codigo_op), sizeof(paquete->codigo_op));
	desplazamiento += sizeof(paquete->codigo_op);

	//meto en el buffer el tamaño de lo que meti en el buffer del paquete que voy a mandar
	memcpy(buffer_serializar  + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento += sizeof(paquete->buffer->size);
	//printf("El tamaño del Payload es: %i \n", paquete->buffer->size);

	//por ultimo meto en el buffer lo que meti en el buffer del paquete
	memcpy(buffer_serializar  + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	(*size_serializado) = size_ya_armado;
	return buffer_serializar; //devuelvo el mensaje listo para enviar
}

uint32_t serializar_paquete_seleccionar_restaurante(t_paquete* paquete, seleccionar_restaurante* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*2
			     + strlen(estructura->idCliente)+1
				 + strlen(estructura->nombreRestaurante)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el largo del identificador del cliente
	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoIDCliente), sizeof(estructura->largoIDCliente));
	desplazamiento += sizeof(estructura->largoIDCliente);

	memcpy(streamAuxiliar + desplazamiento, estructura->idCliente, estructura->largoIDCliente+1);
	desplazamiento += estructura->largoIDCliente+1;

	//meto el largo del nombre del Restaurante
	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	memcpy(streamAuxiliar + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoIDCliente) + estructura->largoIDCliente+1 + sizeof(estructura->largoNombreRestaurante) + estructura->largoNombreRestaurante+1;

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

//este juega doble, tanto para OBTENER_RESTAURANTE como para CONSULTAR_PLATOS (usan exactamente los mismos parametros)
uint32_t serializar_paquete_obtener_restaurante(t_paquete* paquete, obtener_restaurante* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

    //reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)
				 + strlen(estructura->nombreRestaurante)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el largo del nombre del Restaurante
	memcpy(streamAuxiliar, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	//memcpy(paquete->buffer->stream + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	memcpy(streamAuxiliar + desplazamiento, estructura->nombreRestaurante, strlen(estructura->nombreRestaurante)+1);
	desplazamiento += strlen(estructura->nombreRestaurante)+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante) + estructura->largoNombreRestaurante+1;

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
  //NEW el puntero al stream del buffer reservado con malloc va a ser streamAuxiliar con el payload procesado con memcpy sin padding
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

uint32_t serializar_paquete_consultar_platos(t_paquete* paquete, consultar_platos* estructura){
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

    //reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*2
				 + strlen(estructura->nombreResto)+1
				 + strlen(estructura->id)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el largo del nombre del Restaurante
	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeNombre), sizeof(estructura->sizeNombre));
	desplazamiento += sizeof(estructura->sizeNombre);

	//meto el nombre del restaurante
	//memcpy(paquete->buffer->stream + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	memcpy(streamAuxiliar + desplazamiento, estructura->nombreResto, estructura->sizeNombre+1);
	desplazamiento += estructura->sizeNombre+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeId), sizeof(estructura->sizeId));
	desplazamiento += sizeof(estructura->sizeId);

	memcpy(streamAuxiliar + desplazamiento, estructura->id, estructura->sizeId+1);
	desplazamiento += estructura->sizeId+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->sizeNombre) + estructura->sizeNombre+1 + sizeof(estructura->sizeId) + estructura->sizeId+1;

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
  //NEW el puntero al stream del buffer reservado con malloc va a ser streamAuxiliar con el payload procesado con memcpy sin padding
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

uint32_t serializar_paquete_guardar_plato(t_paquete* paquete, guardar_plato* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	if(strlen(estructura->nombreRestaurante) != estructura->largoNombreRestaurante){
			   printf("Error en la serializacion de longitudes, sos pollo\n");
			   return -1;
			}

	if(strlen(estructura->nombrePlato) != estructura->largoNombrePlato){
			   printf("Error en la serializacion de longitudes, sos pollo\n");
			   return -1;
			}


	 //reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*4
				 + strlen(estructura->nombreRestaurante)+1
				 + strlen(estructura->nombrePlato)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el largo del nombre del Restaurante
	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	memcpy(streamAuxiliar + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	//meto la ID del pedido
	memcpy(streamAuxiliar + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	//meto el largo del nombre del plato a agregar al pedido
	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombrePlato), sizeof(estructura->largoNombrePlato));
	desplazamiento += sizeof(estructura->largoNombrePlato);

	//meto el nombre del plato a agregar al pedido
	memcpy(streamAuxiliar + desplazamiento, estructura->nombrePlato, estructura->largoNombrePlato+1);
	desplazamiento += estructura->largoNombrePlato+1;

	//meto la cantidad de platos a agregar al pedido
	memcpy(streamAuxiliar + desplazamiento, &(estructura->cantidadPlatos), sizeof(estructura->cantidadPlatos));
	desplazamiento += sizeof(estructura->cantidadPlatos);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante) + estructura->largoNombreRestaurante+1 + sizeof(estructura->idPedido) + sizeof(estructura->largoNombrePlato) + estructura->largoNombrePlato+1 + sizeof(estructura->cantidadPlatos);

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

uint32_t serializar_paquete_aniadir_plato(t_paquete* paquete, a_plato* estructura){

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*3
			     + strlen(estructura->nombrePlato)+1
	             + strlen(estructura->id)+1;

	void* streamAuxiliar = malloc(buffer->size);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombrePlato), sizeof(estructura->largoNombrePlato));
	desplazamiento += sizeof(estructura->largoNombrePlato);

	memcpy(streamAuxiliar + desplazamiento, estructura->nombrePlato, strlen(estructura->nombrePlato)+1);
	desplazamiento += strlen(estructura->nombrePlato)+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeId), sizeof(estructura->sizeId));
	desplazamiento += sizeof(estructura->sizeId);

	memcpy(streamAuxiliar + desplazamiento, estructura->id, strlen(estructura->id)+1);
    desplazamiento += strlen(estructura->id)+1;


	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombrePlato)
			               + strlen(estructura->nombrePlato)+1
						   + sizeof(estructura->idPedido)
						   + sizeof(estructura->sizeId)
						   + strlen(estructura->id)+1;


		if(desplazamiento != pesoDeElementosAEnviar)
		{
			puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
			abort();
		}
		else
		{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

			//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

uint32_t serializar_paquete_plato_listo(t_paquete* paquete, plato_listo* estructura){

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*3
			     + estructura->largoNombreRestaurante+1
				 + estructura->largoNombrePlato+1;

	void* streamAuxiliar = malloc(buffer->size);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	memcpy(streamAuxiliar + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombrePlato), sizeof(estructura->largoNombrePlato));
	desplazamiento += sizeof(estructura->largoNombrePlato);

	memcpy(streamAuxiliar + desplazamiento, estructura->nombrePlato, estructura->largoNombrePlato+1);
	desplazamiento += estructura->largoNombrePlato+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante)
							+ estructura->largoNombreRestaurante+1
							+ sizeof(estructura->idPedido)
							+ sizeof(estructura->largoNombrePlato)
		     			    + estructura->largoNombrePlato+1;


		if(desplazamiento != pesoDeElementosAEnviar)
		{
			puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
			abort();
		}
		else
		{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}


uint32_t serializar_paquete_guardar_pedido(t_paquete* paquete, guardar_pedido* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	 //reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*2
				 + strlen(estructura->nombreRestaurante)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el largo del nombre del Restaurante
	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombreRestaurante),  sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	memcpy(streamAuxiliar + desplazamiento, estructura->nombreRestaurante, strlen(estructura->nombreRestaurante)+1);
	desplazamiento += strlen(estructura->nombreRestaurante)+1;

	//meto la ID del pedido
	memcpy(streamAuxiliar + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante)
			               + estructura->largoNombreRestaurante+1
						   + sizeof(estructura->idPedido);

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

uint32_t serializar_paquete_consultar_pedido(t_paquete* paquete, consultar_pedido* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;


	 //reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*2
			     + strlen(estructura->id)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto la ID del pedido
	memcpy(streamAuxiliar + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeId), sizeof(estructura->sizeId));
	desplazamiento += sizeof(estructura->sizeId);

	memcpy(streamAuxiliar + desplazamiento, estructura->id, strlen(estructura->id)+1);
	desplazamiento += strlen(estructura->id)+1;


	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = buffer->size;

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

uint32_t serializar_paquete_obtener_receta(t_paquete* paquete, obtener_receta* estructura){

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	if(strlen(estructura->nombreReceta) != estructura->largoNombreReceta){
		   printf("Error en la serializacion de longitudes, sos pollo\n");
		   return -1;
		}

	 //reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)
				 + strlen(estructura->nombreReceta)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el largo del nombre de la receta
	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombreReceta),  sizeof(estructura->largoNombreReceta));
	desplazamiento += sizeof(estructura->largoNombreReceta);

	//meto el nombre de la receta
	memcpy(streamAuxiliar + desplazamiento, estructura->nombreReceta, strlen(estructura->nombreReceta)+1);
	desplazamiento += strlen(estructura->nombreReceta)+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreReceta) + estructura->largoNombreReceta +1;

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}
	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}


uint32_t serializar_paquete_handshake(t_paquete* paquete, handshake* estructura){

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	if(strlen(estructura->id) != estructura->longitudIDCliente){
		   printf("Error en la serializacion de longitudes, sos pollo\n");
		   return -1;
		}

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*3
				 + strlen(estructura->id)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el largo del id del cliente
	memcpy(streamAuxiliar + desplazamiento, &(estructura->longitudIDCliente),  sizeof(estructura->longitudIDCliente));
	desplazamiento += sizeof(estructura->longitudIDCliente);

	//meto el id
	memcpy(streamAuxiliar + desplazamiento, estructura->id, strlen(estructura->id)+1);
	desplazamiento += strlen(estructura->id)+1;

	//meto la posicion en x
	memcpy(streamAuxiliar + desplazamiento,  &(estructura->posX), sizeof(estructura->posX));
	desplazamiento += sizeof(estructura->posX);

	//meto la posicion en y
	memcpy(streamAuxiliar + desplazamiento,  &(estructura->posY), sizeof(estructura->posY));
	desplazamiento += sizeof(estructura->posY);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->longitudIDCliente) + strlen(estructura->id)+1 + sizeof(estructura->posX) + sizeof(estructura->posY);

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}
	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}


uint32_t serializar_paquete_respuesta_consultar_restaurantes(t_paquete* paquete, respuesta_consultar_restaurantes* estructura){
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*2
				 + strlen(estructura->listaRestaurantes)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el numero que representa cuantos restaurantes tengo en la cadena del mensaje
	memcpy(streamAuxiliar + desplazamiento, &(estructura->cantRestaurantes),  sizeof(estructura->cantRestaurantes));
	desplazamiento += sizeof(estructura->cantRestaurantes);

	//meto el numero que representa la longitud en bytes de la lista con los corchetes puestos
	memcpy(streamAuxiliar + desplazamiento, &(estructura->longitudListaRestaurantes),  sizeof(estructura->longitudListaRestaurantes));
	desplazamiento += sizeof(estructura->longitudListaRestaurantes);

	//meto la lista
	memcpy(streamAuxiliar + desplazamiento, estructura->listaRestaurantes, strlen(estructura->listaRestaurantes)+1);
	desplazamiento += strlen(estructura->listaRestaurantes)+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->cantRestaurantes) + sizeof(estructura->longitudListaRestaurantes) + 1 + strlen(estructura->listaRestaurantes);

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}


}


uint32_t serializar_paquete_respuesta_obtener_restaurante(t_paquete* paquete, respuesta_obtener_restaurante* estructura){

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;



    //reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*8
				 + strlen(estructura->afinidades)+1
	             + strlen(estructura->platos)+1
	             + strlen(estructura->precioPlatos)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//paso la cantidad de cocineros
	memcpy(streamAuxiliar, &(estructura->cantidadCocineros), sizeof(estructura->cantidadCocineros));
	desplazamiento += sizeof(estructura->cantidadCocineros);

	//paso las coordenadas del restau en el mapa
	memcpy(streamAuxiliar + desplazamiento, &(estructura->posX), sizeof(estructura->posX));
	desplazamiento += sizeof(estructura->posX);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->posY), sizeof(estructura->posY));
	desplazamiento += sizeof(estructura->posY);

	//paso la cant de hornos
	memcpy(streamAuxiliar + desplazamiento, &(estructura->cantHornos), sizeof(estructura->cantHornos));
	desplazamiento += sizeof(estructura->cantHornos);

	//paso la cant de pedidos ya existentes
	memcpy(streamAuxiliar + desplazamiento, &(estructura->cantPedidos), sizeof(estructura->cantPedidos));
	desplazamiento += sizeof(estructura->cantPedidos);

	//paso la longitud del char* afinidades (de cocineros)
	memcpy(streamAuxiliar + desplazamiento, &(estructura->longitudAfinidades), sizeof(estructura->longitudAfinidades));
	desplazamiento += sizeof(estructura->longitudAfinidades);

	memcpy(streamAuxiliar + desplazamiento, estructura->afinidades, strlen(estructura->afinidades)+1);
	desplazamiento += strlen(estructura->afinidades)+1;


	//paso la longitud del char* platos
	memcpy(streamAuxiliar + desplazamiento, &(estructura->longitudPlatos), sizeof(estructura->longitudPlatos));
	desplazamiento += sizeof(estructura->longitudPlatos);

	memcpy(streamAuxiliar + desplazamiento, estructura->platos, strlen(estructura->platos)+1);
	desplazamiento += strlen(estructura->platos)+1;

	//paso la longitud del char* precioPlatos
	memcpy(streamAuxiliar + desplazamiento, &(estructura->longitudPrecioPlatos), sizeof(estructura->longitudPrecioPlatos));
	desplazamiento += sizeof(estructura->longitudPrecioPlatos);

	memcpy(streamAuxiliar + desplazamiento, estructura->precioPlatos, strlen(estructura->precioPlatos)+1);
	desplazamiento += strlen(estructura->precioPlatos)+1;



	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->cantidadCocineros)
						   + sizeof(estructura->posX)
						   + sizeof(estructura->posX)
						   + sizeof(estructura->cantHornos)
						   + sizeof(estructura->cantPedidos)
						   + sizeof(estructura->longitudAfinidades)
						   + estructura->longitudAfinidades+1
						   + sizeof(estructura->longitudPlatos)
						   + estructura->longitudPlatos+1
						   + sizeof(estructura->longitudPrecioPlatos)
						   + estructura->longitudPrecioPlatos+1;



	if(desplazamiento != pesoDeElementosAEnviar)
		{
			puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
			abort();
		}

		else
		{
			buffer->stream = streamAuxiliar;
			paquete->buffer = buffer;
			//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
			paquete->buffer->size = desplazamiento;

			//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
			size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

			//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
			return size;
		}

}


uint32_t serializar_paquete_respuesta_consultar_platos(t_paquete* paquete, respuesta_consultar_platos* estructura)
{

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)
			     + strlen(estructura->nombresPlatos)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//paso la longitud del char* platos
	memcpy(streamAuxiliar, &(estructura->longitudNombresPlatos), sizeof(estructura->longitudNombresPlatos));
	desplazamiento += sizeof(estructura->longitudNombresPlatos);

	memcpy(streamAuxiliar + desplazamiento, estructura->nombresPlatos, strlen(estructura->nombresPlatos)+1);
	desplazamiento += strlen(estructura->nombresPlatos)+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->longitudNombresPlatos)
						   + estructura->longitudNombresPlatos+1;



		if(desplazamiento != pesoDeElementosAEnviar)
			{
				puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
				abort();
			}

			else
			{
				buffer->stream = streamAuxiliar;
				paquete->buffer = buffer;
				//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
				paquete->buffer->size = desplazamiento;

				//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
				size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

				//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
				return size;
			}
}



uint32_t serializar_paquete_respuesta_crear_pedido(t_paquete* paquete, respuesta_crear_pedido* estructura){

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t);

	void* streamAuxiliar = malloc(buffer->size);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->idPedido);


		if(desplazamiento != pesoDeElementosAEnviar)
		{
				puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
				abort();
		}

		else
		{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
	return size;
	}
}

uint32_t serializar_paquete_respuesta_consultar_pedido(t_paquete* paquete, respuesta_consultar_pedido* estructura){
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*4
				 + sizeof(estructura->estado)
				 + estructura->largoNombreRestaurante+1
				 + estructura->sizeComidas+1
				 + estructura->sizeCantTotales+1
				 + estructura->sizeCantListas+1;

	void* streamAuxiliar = malloc(buffer->size);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	memcpy(streamAuxiliar + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->estado), sizeof(estructura->estado));
	desplazamiento += sizeof(estructura->estado);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeComidas), sizeof(estructura->sizeComidas));
	desplazamiento += sizeof(estructura->sizeComidas);

	memcpy(streamAuxiliar + desplazamiento, estructura->comidas, estructura->sizeComidas+1);
	desplazamiento += estructura->sizeComidas+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeCantTotales), sizeof(estructura->sizeCantTotales));
	desplazamiento += sizeof(estructura->sizeCantTotales);

	memcpy(streamAuxiliar + desplazamiento, estructura->cantTotales, estructura->sizeCantTotales+1);
	desplazamiento += estructura->sizeCantTotales+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeCantListas), sizeof(estructura->sizeCantListas));
	desplazamiento += sizeof(estructura->sizeCantListas);

	memcpy(streamAuxiliar + desplazamiento, estructura->cantListas, estructura->sizeCantListas+1);
	desplazamiento += estructura->sizeCantListas+1;

	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante)
		                   + estructura->largoNombreRestaurante+1
						   + sizeof(estructura->estado)
						   + sizeof(estructura->sizeComidas)
						   + estructura->sizeComidas+1
						   + sizeof(estructura->sizeCantTotales)
						   + estructura->sizeCantTotales+1
				     	   + sizeof(estructura->sizeCantListas)
						   + estructura->sizeCantTotales+1;


				if(desplazamiento != pesoDeElementosAEnviar)
				{
						puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
						abort();
				}

				else
				{
				buffer->stream = streamAuxiliar;
				paquete->buffer = buffer;
				//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
				paquete->buffer->size = desplazamiento;

				//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
				size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

				//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
			return size;
				}
}

uint32_t serializar_paquete_respuesta_obtener_pedido(t_paquete* paquete, respuesta_obtener_pedido* estructura){
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*3
				 + sizeof(estructura->estado)
				 + estructura->sizeComidas+1
				 + estructura->sizeCantTotales+1
				 + estructura->sizeCantListas+1;

	void* streamAuxiliar = malloc(buffer->size);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->estado), sizeof(estructura->estado));
	desplazamiento += sizeof(estructura->estado);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeComidas), sizeof(estructura->sizeComidas));
	desplazamiento += sizeof(estructura->sizeComidas);

	memcpy(streamAuxiliar + desplazamiento, estructura->comidas, estructura->sizeComidas+1);
	desplazamiento += estructura->sizeComidas+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeCantTotales), sizeof(estructura->sizeCantTotales));
	desplazamiento += sizeof(estructura->sizeCantTotales);

	memcpy(streamAuxiliar + desplazamiento, estructura->cantTotales, estructura->sizeCantTotales+1);
	desplazamiento += estructura->sizeCantTotales+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeCantListas), sizeof(estructura->sizeCantListas));
	desplazamiento += sizeof(estructura->sizeCantListas);

	memcpy(streamAuxiliar + desplazamiento, estructura->cantListas, estructura->sizeCantListas+1);
	desplazamiento += estructura->sizeCantListas+1;


	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->estado)
			               + sizeof(estructura->sizeComidas)
						   + estructura->sizeComidas+1
						   + sizeof(estructura->sizeCantTotales)
						   + estructura->sizeCantTotales+1
						   + sizeof(estructura->sizeCantListas)
						   + estructura->sizeCantListas+1;


			if(desplazamiento != pesoDeElementosAEnviar)
			{
					puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
					abort();
			}

			else
			{
			buffer->stream = streamAuxiliar;
			paquete->buffer = buffer;
			//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
			paquete->buffer->size = desplazamiento;

			//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
			size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

			//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
		}



}


uint32_t serializar_paquete_respuesta_obtener_receta(t_paquete* paquete, respuesta_obtener_receta* estructura){

	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*2
			     + estructura->sizePasos+1
	             + estructura->sizeTiempoPasos+1;

	void* streamAuxiliar = malloc(buffer->size);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizePasos), sizeof(estructura->sizePasos));
	desplazamiento += sizeof(estructura->sizePasos);

	memcpy(streamAuxiliar + desplazamiento, estructura->pasos, estructura->sizePasos+1);
	desplazamiento += estructura->sizePasos+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeTiempoPasos), sizeof(estructura->sizeTiempoPasos));
	desplazamiento += sizeof(estructura->sizePasos);

	memcpy(streamAuxiliar + desplazamiento, estructura->tiempoPasos, estructura->sizeTiempoPasos+1);
	desplazamiento += estructura->sizeTiempoPasos+1;



	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->sizePasos)
			               + sizeof(estructura->sizeTiempoPasos)
						   + estructura->sizePasos+1
						   + estructura->sizeTiempoPasos+1;


			if(desplazamiento != pesoDeElementosAEnviar)
			{
					puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
					abort();
			}

			else
			{
			buffer->stream = streamAuxiliar;
			paquete->buffer = buffer;
			//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
			paquete->buffer->size = desplazamiento;

			//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
			size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

			//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}


uint32_t serializar_paquete_ok_fail(t_paquete* paquete, respuesta_ok_error* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//meto el booleano con el resultado
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->respuesta), sizeof(estructura->respuesta));
	desplazamiento += sizeof(estructura->respuesta);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->respuesta);

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}
}

uint32_t serializar_paquete_crear_pedido(t_paquete* paquete, crear_pedido* estructura){
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//reservo memoria ESPECIFICAMENTE para el buffer de bytes (payload) que mi querido paquete va a contener
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)
				 + strlen(estructura->id)+1;

	void* streamAuxiliar = malloc(buffer->size);

	//meto el numero que representa cuantos restaurantes tengo en la cadena del mensaje
	memcpy(streamAuxiliar + desplazamiento, &(estructura->sizeId),  sizeof(estructura->sizeId));
	desplazamiento += sizeof(estructura->sizeId);

	//meto la lista
	memcpy(streamAuxiliar + desplazamiento, estructura->id, strlen(estructura->id)+1);
	desplazamiento += strlen(estructura->id)+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->sizeId) + estructura->sizeId + 1;

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}


}

uint32_t serializar_paquete_agregar_restaurante(t_paquete* paquete, agregar_restaurante* estructura){
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*5
				 + estructura->largoNombreRestaurante+1
	             + estructura->largoIp+1
	             + estructura->largoPuerto+1;

	void* streamAuxiliar = malloc(buffer->size);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoNombreRestaurante),  sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	memcpy(streamAuxiliar + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoIp),  sizeof(estructura->largoIp));
	desplazamiento += sizeof(estructura->largoIp);

	memcpy(streamAuxiliar + desplazamiento, estructura->ip, estructura->largoIp+1);
	desplazamiento += estructura->largoIp+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->largoPuerto),  sizeof(estructura->largoPuerto));
	desplazamiento += sizeof(estructura->largoPuerto);

	memcpy(streamAuxiliar + desplazamiento, estructura->puerto, estructura->largoPuerto+1);
	desplazamiento += estructura->largoPuerto+1;

	memcpy(streamAuxiliar + desplazamiento, &(estructura->posX),  sizeof(estructura->posX));
	desplazamiento += sizeof(estructura->posX);

	memcpy(streamAuxiliar + desplazamiento, &(estructura->posY),  sizeof(estructura->posY));
	desplazamiento += sizeof(estructura->posY);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = buffer->size;


	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo.\n");
		abort();
	}

	else
	{
		buffer->stream = streamAuxiliar;
		paquete->buffer = buffer;
		//le meto al size del buffer el tamaño de lo que acabo de meter en el buffer
		paquete->buffer->size = desplazamiento;

		//el tamaño del mensaje entero es el codigo de operacion + la variable donde me guarde el size del buffer + lo que pesa el buffer
		size = sizeof(codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;

		//devuelvo el tamaño de lo que meti en el paquete para poder hacer el malloc
		return size;
	}


}


void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

/*void recibir_mensaje(void* estructura, codigo_operacion tipoMensaje, int32_t socket_cliente)
{
//	int32_t size;
//	bytesRecibidos(recv(socket_cliente, &size, sizeof(size), MSG_WAITALL)); //saca el tamaño de lo que sigue en el buffer
//
//	printf("Tamaño de lo que sigue en el buffer: %u.\n", size);
//
//	*sizeDeLosDatos = size;

	desserializar_mensaje(estructura, tipoMensaje, socket_cliente);
}
*/

void recibir_mensaje (void* estructura, codigo_operacion tipoMensaje, int32_t socket_cliente)
{
	switch(tipoMensaje)
	{
	    case CONSULTAR_RESTAURANTES:
		    //no se hace nada, consultar restaurantes no posee parametros.
		    break;

		case SELECCIONAR_RESTAURANTE:
			desserializar_seleccionar_restaurante(estructura, socket_cliente);
			break;

		case OBTENER_RESTAURANTE:
			desserializar_obtener_restaurante(estructura, socket_cliente);
			break;

		case CONSULTAR_PLATOS:
			desserializar_consultar_platos(estructura, socket_cliente);
			break;

		case GUARDAR_PLATO:
			desserializar_guardar_plato(estructura, socket_cliente);
			break;

		case A_PLATO:
			desserializar_aniadir_plato(estructura, socket_cliente);
			break;

		case PLATO_LISTO:
            desserializar_plato_listo(estructura, socket_cliente);
			break;

		case CREAR_PEDIDO:
			desserializar_crear_pedido(estructura, socket_cliente);
			break;

		case GUARDAR_PEDIDO:
			desserializar_guardar_pedido(estructura, socket_cliente);
			break;

		//usa exactamente la misma que guardar pedido (salvo al mandarse al restaurante ? ToDo CHEQUEAR)
		case CONFIRMAR_PEDIDO:
            desserializar_guardar_pedido(estructura, socket_cliente);
			break;

		//usa exactamente la misma que respuesta_crear_pedido
		case CONSULTAR_PEDIDO:
            desserializar_consultar_pedido(estructura, socket_cliente);
			break;

		//usa exactamente la misma que guardar pedido
		case OBTENER_PEDIDO:
			desserializar_guardar_pedido(estructura, socket_cliente);
			break;

        //usa exactamente la misma que guardar pedido
		case FINALIZAR_PEDIDO:
			desserializar_guardar_pedido(estructura, socket_cliente);
			break;

		//usa exactamente la misma que guardar pedido
		case TERMINAR_PEDIDO:
			desserializar_guardar_pedido(estructura, socket_cliente);
			break;

		case OBTENER_RECETA:
            desserializar_obtener_receta(estructura, socket_cliente);
			break;

		case HANDSHAKE:
			desserializar_handshake(estructura, socket_cliente);
			break;

		case RESPUESTA_CONSULTAR_R:
			desserializar_respuesta_consultar_restaurantes(estructura, socket_cliente);
			break;

		case RESPUESTA_SELECCIONAR_R:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_OBTENER_REST:
			desserializar_respuesta_obtener_restaurante(estructura,socket_cliente);
			break;

		case RESPUESTA_CONSULTAR_PLATOS:
			desserializar_respuesta_consultar_platos(estructura,socket_cliente);
			break;

		case RESPUESTA_GUARDAR_PLATO:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_A_PLATO:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_PLATO_LISTO:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_CREAR_PEDIDO:
			desserializar_respuesta_crear_pedido(estructura,socket_cliente);
			break;

		case RESPUESTA_GUARDAR_PEDIDO:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_CONFIRMAR_PEDIDO:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_CONSULTAR_PEDIDO:
			desserializar_respuesta_consultar_pedido(estructura, socket_cliente);
			break;

		case RESPUESTA_OBTENER_PEDIDO:
            desserializar_respuesta_obtener_pedido(estructura, socket_cliente);
			break;

		case RESPUESTA_FINALIZAR_PEDIDO:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_TERMINAR_PEDIDO:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		case RESPUESTA_OBTENER_RECETA:
            desserializar_respuesta_obtener_receta(estructura, socket_cliente);
			break;

		case AGREGAR_RESTAURANTE:
			desserializar_agregar_restaurante(estructura, socket_cliente);
			break;

		case RESPUESTA_AGREGAR_RESTAURANTE:
			desserializar_ok_fail(estructura, socket_cliente);
			break;

		default:
			puts("\n\n\nATENCION: Switch de desserializar_paquete pasó por el caso default.\n\n\n");
			break;
	}
}

void desserializar_seleccionar_restaurante(seleccionar_restaurante* estructura, int32_t socket_cliente)
{
	//saco el largo del ID del cliente
	bytesRecibidos(recv(socket_cliente, &(estructura->largoIDCliente), sizeof(estructura->largoIDCliente), MSG_WAITALL));

	estructura->idCliente = malloc(estructura->largoIDCliente+1);
	bytesRecibidos(recv(socket_cliente, estructura->idCliente, estructura->largoIDCliente+1, MSG_WAITALL));

	//saco el largo del nombre del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante), MSG_WAITALL));

	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1, MSG_WAITALL));
}

void desserializar_obtener_restaurante(obtener_restaurante* estructura, int32_t socket_cliente)
{
	//saco el largo del nombre del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);

	//saco el nombre del restaurante en si
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1, MSG_WAITALL));

}

void desserializar_consultar_platos(consultar_platos* estructura, int32_t socket_cliente){
	//saco el largo del nombre del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->sizeNombre), sizeof(estructura->sizeNombre), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombreResto = malloc(estructura->sizeNombre+1);
	//saco el nombre del restaurante en si
	bytesRecibidos(recv(socket_cliente, estructura->nombreResto, estructura->sizeNombre+1, MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeId), sizeof(estructura->sizeId), MSG_WAITALL));

	estructura->id = malloc(estructura->sizeId+1);

	bytesRecibidos(recv(socket_cliente, estructura->id, estructura->sizeId+1, MSG_WAITALL));

}

void desserializar_crear_pedido(crear_pedido* estructura, int32_t socket_cliente){
	bytesRecibidos(recv(socket_cliente, &(estructura->sizeId), sizeof(estructura->sizeId), MSG_WAITALL));

	estructura->id = malloc(estructura->sizeId+1);

	bytesRecibidos(recv(socket_cliente, estructura->id, estructura->sizeId+1, MSG_WAITALL));

}

void desserializar_guardar_plato(guardar_plato* estructura, int32_t socket_cliente)
{
	//saco el largo del nombre del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);

	//saco el nombre del restaurante en si
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1, MSG_WAITALL));

	//saco la ID del pedido
	bytesRecibidos(recv(socket_cliente, &(estructura->idPedido), sizeof(estructura->idPedido), MSG_WAITALL));

	//saco el largo del nombre del plato
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombrePlato), sizeof(estructura->largoNombrePlato), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombrePlato = malloc(estructura->largoNombrePlato+1);

	//saco el nombre del plato
	bytesRecibidos(recv(socket_cliente, estructura->nombrePlato, estructura->largoNombrePlato+1, MSG_WAITALL));

	//saco la cantidad de platos a agregar al pedido
	bytesRecibidos(recv(socket_cliente, &(estructura->cantidadPlatos), sizeof(estructura->cantidadPlatos), MSG_WAITALL));
}

void desserializar_ok_fail(respuesta_ok_error* estructura, int32_t socket_cliente)
{
	//saco el resultado de la consulta
	bytesRecibidos(recv(socket_cliente, &(estructura->respuesta), sizeof(estructura->respuesta), MSG_WAITALL));

	printf("El resultado de la consulta es: %u.\n", estructura->respuesta);
}

void desserializar_guardar_pedido(guardar_pedido* estructura, int32_t socket_cliente)
{
	//saco el largo del nombre del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);

	//saco el nombre del restaurante en si
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1, MSG_WAITALL));

	//saco la ID del pedido
	bytesRecibidos(recv(socket_cliente, &(estructura->idPedido), sizeof(estructura->idPedido), MSG_WAITALL));
}

void desserializar_consultar_pedido(consultar_pedido* estructura, int32_t socket_cliente)
{
	//saco el id del pedido
	bytesRecibidos(recv(socket_cliente, &(estructura->idPedido), sizeof(estructura->idPedido), MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeId), sizeof(estructura->sizeId), MSG_WAITALL));

	estructura->id = malloc(estructura->sizeId+1);

	bytesRecibidos(recv(socket_cliente, estructura->id, estructura->sizeId+1, MSG_WAITALL));
}


void desserializar_aniadir_plato(a_plato* estructura, int32_t socket_cliente){

	//saco el largo del nombre del plato
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombrePlato), sizeof(estructura->largoNombrePlato), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombrePlato = malloc(estructura->largoNombrePlato+1);
	//saco el nombre del restaurante en si
	bytesRecibidos(recv(socket_cliente, estructura->nombrePlato, estructura->largoNombrePlato+1, MSG_WAITALL));

	//saco la ID del pedido
	bytesRecibidos(recv(socket_cliente, &(estructura->idPedido), sizeof(estructura->idPedido), MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeId), sizeof(estructura->sizeId), MSG_WAITALL));

	estructura->id = malloc(estructura->sizeId+1);

	bytesRecibidos(recv(socket_cliente, estructura->id, estructura->sizeId+1, MSG_WAITALL));

}

void desserializar_plato_listo(plato_listo* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante), MSG_WAITALL));

	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1, MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->idPedido), sizeof(estructura->idPedido), MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombrePlato), sizeof(estructura->largoNombrePlato), MSG_WAITALL));

	estructura->nombrePlato = malloc(estructura->largoNombrePlato+1);
	bytesRecibidos(recv(socket_cliente, estructura->nombrePlato, estructura->largoNombrePlato+1, MSG_WAITALL));
}

void desserializar_obtener_receta(obtener_receta* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreReceta), sizeof(estructura->largoNombreReceta), MSG_WAITALL));

	estructura->nombreReceta = malloc(estructura->largoNombreReceta+1);
	bytesRecibidos(recv(socket_cliente, estructura->nombreReceta, estructura->largoNombreReceta+1, MSG_WAITALL));

}

void desserializar_handshake(handshake* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->longitudIDCliente), sizeof(estructura->longitudIDCliente), MSG_WAITALL));

	estructura->id = malloc(estructura->longitudIDCliente+1);
	bytesRecibidos(recv(socket_cliente, estructura->id, estructura->longitudIDCliente+1, MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->posX), sizeof(estructura->posX), MSG_WAITALL));
	bytesRecibidos(recv(socket_cliente, &(estructura->posY), sizeof(estructura->posX), MSG_WAITALL));
}

void desserializar_respuesta_consultar_restaurantes(respuesta_consultar_restaurantes* estructura, int32_t socket_cliente){

	//obtengo la cantidad de restaurantes
	bytesRecibidos(recv(socket_cliente, &(estructura->cantRestaurantes), sizeof(estructura->cantRestaurantes), MSG_WAITALL));

	//obtengo la longitud de la cadena con los restaurantes
	bytesRecibidos(recv(socket_cliente, &(estructura->longitudListaRestaurantes), sizeof(estructura->longitudListaRestaurantes), MSG_WAITALL));

	//reservo memoria para la cadena y la recibo
	estructura->listaRestaurantes = malloc(estructura->longitudListaRestaurantes+1);
	bytesRecibidos(recv(socket_cliente, estructura->listaRestaurantes, estructura->longitudListaRestaurantes+1, MSG_WAITALL));

}

void desserializar_respuesta_obtener_restaurante(respuesta_obtener_restaurante* estructura, int32_t socket_cliente){

	//obtengo la cantidad de cocineros
	bytesRecibidos(recv(socket_cliente, &(estructura->cantidadCocineros), sizeof(estructura->cantidadCocineros), MSG_WAITALL));

    //obtengo posiciones del restau en el mapa
	bytesRecibidos(recv(socket_cliente, &(estructura->posX), sizeof(estructura->posX), MSG_WAITALL));
	bytesRecibidos(recv(socket_cliente, &(estructura->posY), sizeof(estructura->posY), MSG_WAITALL));

	//obtengo la cantidad de hornos
	bytesRecibidos(recv(socket_cliente, &(estructura->cantHornos), sizeof(estructura->cantHornos), MSG_WAITALL));

	//recibo la cantidad de pedidos ya existentes
	bytesRecibidos(recv(socket_cliente, &(estructura->cantPedidos), sizeof(estructura->cantPedidos), MSG_WAITALL));

	//recibo la longitud del char* afinidades
	bytesRecibidos(recv(socket_cliente, &(estructura->longitudAfinidades), sizeof(estructura->longitudAfinidades), MSG_WAITALL));

	estructura->afinidades = malloc(estructura->longitudAfinidades+1);
	bytesRecibidos(recv(socket_cliente, estructura->afinidades, estructura->longitudAfinidades+1,  MSG_WAITALL));

	//recibo la longitud del char* platos
	bytesRecibidos(recv(socket_cliente, &(estructura->longitudPlatos), sizeof(estructura->longitudPlatos), MSG_WAITALL));

	estructura->platos = malloc(estructura->longitudPlatos+1);
	bytesRecibidos(recv(socket_cliente, estructura->platos, estructura->longitudPlatos+1,  MSG_WAITALL));


	//recibo la longitud del char* precioPlatos
	bytesRecibidos(recv(socket_cliente, &(estructura->longitudPrecioPlatos), sizeof(estructura->longitudPrecioPlatos), MSG_WAITALL));

	estructura->precioPlatos = malloc(estructura->longitudPrecioPlatos+1);
	bytesRecibidos(recv(socket_cliente, estructura->precioPlatos, estructura->longitudPrecioPlatos+1,  MSG_WAITALL));
}


void desserializar_respuesta_consultar_platos(respuesta_consultar_platos* estructura, int32_t socket_cliente){


	//recibo la longitud del char* nombres de los platos del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->longitudNombresPlatos), sizeof(estructura->longitudNombresPlatos), MSG_WAITALL));

	estructura->nombresPlatos = malloc(estructura->longitudNombresPlatos+1);
	bytesRecibidos(recv(socket_cliente, estructura->nombresPlatos, estructura->longitudNombresPlatos+1,  MSG_WAITALL));


}


void desserializar_respuesta_crear_pedido(respuesta_crear_pedido* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->idPedido), sizeof(estructura->idPedido),  MSG_WAITALL));

}

void desserializar_respuesta_consultar_pedido(respuesta_consultar_pedido* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante),  MSG_WAITALL));
	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->estado), sizeof(estructura->estado),  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeComidas), sizeof(estructura->sizeComidas),  MSG_WAITALL));
	estructura->comidas = malloc(estructura->sizeComidas+1);
	bytesRecibidos(recv(socket_cliente, estructura->comidas, estructura->sizeComidas+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeCantTotales), sizeof(estructura->sizeCantTotales),  MSG_WAITALL));
	estructura->cantTotales = malloc(estructura->sizeCantTotales+1);
	bytesRecibidos(recv(socket_cliente, estructura->cantTotales, estructura->sizeCantTotales+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeCantListas), sizeof(estructura->sizeCantListas),  MSG_WAITALL));
	estructura->cantListas = malloc(estructura->sizeCantListas+1);
	bytesRecibidos(recv(socket_cliente, estructura->cantListas, estructura->sizeCantListas+1,  MSG_WAITALL));

}

void desserializar_respuesta_obtener_pedido(respuesta_obtener_pedido* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->estado), sizeof(estructura->estado),  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeComidas), sizeof(estructura->sizeComidas),  MSG_WAITALL));

	estructura->comidas = malloc(estructura->sizeComidas+1);
	bytesRecibidos(recv(socket_cliente, estructura->comidas, estructura->sizeComidas+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeCantTotales), sizeof(estructura->sizeCantTotales),  MSG_WAITALL));

	estructura->cantTotales = malloc(estructura->sizeCantTotales+1);
	bytesRecibidos(recv(socket_cliente, estructura->cantTotales, estructura->sizeCantTotales+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeCantListas), sizeof(estructura->sizeCantListas),  MSG_WAITALL));

	estructura->cantListas = malloc(estructura->sizeCantListas+1);
	bytesRecibidos(recv(socket_cliente, estructura->cantListas, estructura->sizeCantListas+1,  MSG_WAITALL));
}

void desserializar_respuesta_obtener_receta(respuesta_obtener_receta* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->sizePasos), sizeof(estructura->sizePasos),  MSG_WAITALL));

	estructura->pasos = malloc(estructura->sizePasos+1);
	bytesRecibidos(recv(socket_cliente, estructura->pasos, estructura->sizePasos+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->sizeTiempoPasos), sizeof(estructura->sizeTiempoPasos),  MSG_WAITALL));

	estructura->tiempoPasos = malloc(estructura->sizeTiempoPasos+1);
	bytesRecibidos(recv(socket_cliente, estructura->tiempoPasos, estructura->sizeTiempoPasos+1,  MSG_WAITALL));
}

void desserializar_agregar_restaurante(agregar_restaurante* estructura, int32_t socket_cliente){

	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante),  MSG_WAITALL));

	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->largoIp), sizeof(estructura->largoIp),  MSG_WAITALL));

	estructura->ip = malloc(estructura->largoIp+1);
	bytesRecibidos(recv(socket_cliente, estructura->ip, estructura->largoIp+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->largoPuerto), sizeof(estructura->largoPuerto),  MSG_WAITALL));

	estructura->puerto = malloc(estructura->largoPuerto+1);
	bytesRecibidos(recv(socket_cliente, estructura->puerto, estructura->largoPuerto+1,  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->posX), sizeof(estructura->posX),  MSG_WAITALL));

	bytesRecibidos(recv(socket_cliente, &(estructura->posY), sizeof(estructura->posY),  MSG_WAITALL));
}




