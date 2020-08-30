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
		case SELECCIONAR_RESTAURANTE:
				paquete->buffer->stream = malloc(sizeof(seleccionar_restaurante));
				size_ya_armado = serializar_paquete_seleccionar_restaurante(paquete, mensaje);
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

	//meto el PID del Restaurante
	memcpy(paquete->buffer->stream + desplazamiento, &(estructura->restaurante), sizeof(estructura->restaurante));
	desplazamiento += sizeof(estructura->restaurante);

	//controlo que el desplazamiento sea = al peso de lo que mando
	pesoDeElementosAEnviar = sizeof(estructura->cliente) + sizeof(estructura->restaurante);

	if(desplazamiento != pesoDeElementosAEnviar)
	{
		puts("Hubo un error al serializar un mensaje, se pudre todo\n.");
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

//ToDo hablar con el resto sobre la idea de hacer que "desserializar_mensaje" pase a ser directamente recibir_mensaje

void recibir_mensaje(void* estructura, codigo_operacion tipoMensaje, int32_t socket_cliente)
{
//	int32_t size;
//	bytesRecibidos(recv(socket_cliente, &size, sizeof(size), MSG_WAITALL)); //saca el tamaño de lo que sigue en el buffer
//
//	printf("Tamaño de lo que sigue en el buffer: %u.\n", size);
//
//	*sizeDeLosDatos = size;

	desserializar_mensaje(estructura, tipoMensaje, socket_cliente);
}

void desserializar_mensaje (void* estructura, codigo_operacion tipoMensaje, int32_t socket_cliente)
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

	//saco el PID del restaurante
	bytesRecibidos(recv(socket_cliente, &(estructura->restaurante), sizeof(estructura->restaurante), MSG_WAITALL));

	printf("el PID del cliente es: %u\n", estructura->cliente);
	printf("el PID del restaurante es: %u\n", estructura->restaurante);
}

