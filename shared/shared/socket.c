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
	struct addrinfo *server_info;

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
	t_paquete* paquete_por_armar = malloc (sizeof(t_paquete));
	paquete_por_armar->buffer = malloc(sizeof(t_buffer));
	uint32_t size_serializado = 0;

	printf("Voy a mandar un mensaje del tipo %i.\n", tipoMensaje);

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
			paquete->buffer->stream = malloc(0); //malloc flashero para que no rompa despues con el free, ToDo ver si funciona o rompe
			paquete->buffer->size = 0;
			size_ya_armado = sizeof(tipoMensaje);
			break;

		case SELECCIONAR_RESTAURANTE:
				paquete->buffer->stream = malloc(sizeof(seleccionar_restaurante));
				size_ya_armado = serializar_paquete_seleccionar_restaurante(paquete, mensaje);
			break;

		case OBTENER_RESTAURANTE:
				paquete->buffer->stream = malloc(sizeof(obtener_restaurante));
				size_ya_armado = serializar_paquete_obtener_restaurante(paquete, mensaje);
			break;

		case GUARDAR_PLATO:
			paquete->buffer->stream = malloc(sizeof(guardar_plato));
			size_ya_armado = serializar_paquete_guardar_plato(paquete, mensaje);
			break;
		//ir agregando mas a medida que necesitemos

		case GUARDAR_PEDIDO:
			    paquete->buffer->stream = malloc(sizeof(guardar_pedido));
			    size_ya_armado = serializar_paquete_guardar_pedido(paquete, mensaje);
			break;

		default:
			puts("\n\n\nATENCION: Switch de serializar_paquete pasó por el caso default.\n\n\n");
			break;
	}

	//ahora me preparo para meter en el buffer "posta", el choclo que va a enviar mandar_mensaje
	printf("El size que tiene armado es: %i \n", size_ya_armado);
	buffer_serializar = malloc(size_ya_armado);
	uint32_t desplazamiento = 0;

	//meto en el buffer el tipo de mensaje que voy a mandar
	memcpy(buffer_serializar  + desplazamiento, &(paquete->codigo_op), sizeof(paquete->codigo_op));
	desplazamiento += sizeof(paquete->codigo_op);

	//meto en el buffer el tamaño de lo que meti en el buffer del paquete que voy a mandar
	memcpy(buffer_serializar  + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento += sizeof(paquete->buffer->size);
	printf("El tamaño del Payload es: %i \n", paquete->buffer->size);

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

	//meto el PID del Cliente
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->cliente), sizeof(estructura->cliente));
	desplazamiento += sizeof(estructura->cliente);

	//meto el largo del nombre del Restaurante
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	memcpy(paquete->buffer->stream + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->cliente) + sizeof(estructura->largoNombreRestaurante) + estructura->largoNombreRestaurante + 1;

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

uint32_t serializar_paquete_obtener_restaurante(t_paquete* paquete, obtener_restaurante* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//meto el largo del nombre del Restaurante
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	memcpy(paquete->buffer->stream + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante) + estructura->largoNombreRestaurante+1;

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

uint32_t serializar_paquete_guardar_plato(t_paquete* paquete, guardar_plato* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//meto el largo del nombre del Restaurante
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	memcpy(paquete->buffer->stream + desplazamiento, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1);
	desplazamiento += estructura->largoNombreRestaurante+1;

	//meto la ID del pedido
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	//meto el largo del nombre del plato a agregar al pedido
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->largonombrePlato), sizeof(estructura->largonombrePlato));
	desplazamiento += sizeof(estructura->largonombrePlato);

	//meto el nombre del plato a agregar al pedido
	memcpy(paquete->buffer->stream + desplazamiento, estructura->nombrePlato, estructura->largonombrePlato+1);
	desplazamiento += estructura->largonombrePlato+1;

	//meto la cantidad de platos a agregar al pedido
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->cantidadPlatos), sizeof(estructura->cantidadPlatos));
	desplazamiento += sizeof(estructura->cantidadPlatos);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante) + estructura->largoNombreRestaurante+1 + sizeof(estructura->idPedido) + sizeof(estructura->largonombrePlato) + estructura->largonombrePlato+1 + sizeof(estructura->cantidadPlatos);

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

uint32_t serializar_paquete_guardar_pedido(t_paquete* paquete, guardar_pedido* estructura)
{
	uint32_t size = 0;
	uint32_t desplazamiento = 0;
	uint32_t pesoDeElementosAEnviar = 0;

	//meto el largo del nombre del Restaurante
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->largoNombreRestaurante),  sizeof(estructura->largoNombreRestaurante));
	desplazamiento += sizeof(estructura->largoNombreRestaurante);

	//meto el nombre del restaurante
	memcpy(paquete->buffer->stream + desplazamiento, estructura->nombreRestaurante, strlen(estructura->nombreRestaurante)+1);
	desplazamiento += strlen(estructura->nombreRestaurante)+1;

	//meto la ID del pedido
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->idPedido), sizeof(estructura->idPedido));
	desplazamiento += sizeof(estructura->idPedido);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->largoNombreRestaurante) + estructura->largoNombreRestaurante + 1 + sizeof(estructura->idPedido);

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



//Todo faltan meter todas las otras serializaciones*************************


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
		case SELECCIONAR_RESTAURANTE:
			desserializar_seleccionar_restaurante(estructura, socket_cliente);
			break;

		default:
			puts("\n\n\nATENCION: Switch de desserializar_paquete pasó por el caso default.\n\n\n");
			break;
	}
}

void desserializar_seleccionar_restaurante(seleccionar_restaurante* estructura, int32_t socket_cliente)
{
	//saco el PID del cliente
	bytesRecibidos(recv(socket_cliente, &(estructura->cliente), sizeof(estructura->cliente), MSG_WAITALL));

	//saco el largo del nombre del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);

	//saco el nombre del restaurante en si
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1, MSG_WAITALL));

	printf("el PID del cliente es: %u\n", estructura->cliente);
	printf("el largo del nombre del restaurante es: %u\n", estructura->largoNombreRestaurante);
	printf("el nombre del restaurante es: %s.\n", estructura->nombreRestaurante);
}

void desserializar_obtener_restaurante(obtener_restaurante* estructura, int32_t socket_cliente)
{
	//saco el largo del nombre del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->largoNombreRestaurante), sizeof(estructura->largoNombreRestaurante), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);

	//saco el nombre del restaurante en si
	bytesRecibidos(recv(socket_cliente, estructura->nombreRestaurante, estructura->largoNombreRestaurante+1, MSG_WAITALL));

	printf("el largo del nombre del restaurante es: %u\n", estructura->largoNombreRestaurante);
	printf("el nombre del restaurante es: %s.\n", estructura->nombreRestaurante);

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
	bytesRecibidos(recv(socket_cliente, &(estructura->largonombrePlato), sizeof(estructura->largonombrePlato), MSG_WAITALL));

	//preparo un espacio de memoria del tamaño del nombre para poder guardarlo
	estructura->nombrePlato = malloc(estructura->largonombrePlato+1);

	//saco el nombre del plato
	bytesRecibidos(recv(socket_cliente, estructura->nombrePlato, estructura->largonombrePlato+1, MSG_WAITALL));

	//saco la cantidad de platos a agregar al pedido
	bytesRecibidos(recv(socket_cliente, &(estructura->cantidadPlatos), sizeof(estructura->cantidadPlatos), MSG_WAITALL));

	printf("el largo del nombre del restaurante es: %u\n", estructura->largoNombreRestaurante);
	printf("el nombre del restaurante es: %s.\n", estructura->nombreRestaurante);
	printf("la ID del pedido es: %u.\n", estructura->idPedido);
	printf("el largo del nombre del plato es: %u.\n", estructura->largonombrePlato);
	printf("el nombre del plato es: %s.\n", estructura->nombrePlato);
	printf("la cantidad de platos es: %u.\n", estructura->cantidadPlatos);
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

	printf("el largo del nombre del restaurante es: %u\n", estructura->largoNombreRestaurante);
	printf("el nombre del restaurante es: %s.\n", estructura->nombreRestaurante);
	printf("la ID del pedido es: %u.\n", estructura->idPedido);
}










