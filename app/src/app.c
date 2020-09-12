#include "app.h"

int main(){
	listaRestos = list_create(); //inicializo la lista de restaurantes
	listaPedidos = list_create(); //inicializo la lista de pedidos
	return 0;
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

void seleccionarRestaurante(char* nombreResto, int32_t socket_cliente){
	int restoBuscado;
	respuesta_ok_error* respuesta;
	perfil_cliente* perfil;

	restoBuscado = buscar_restaurante(nombreResto);

	if(restoBuscado == 1){
		perfil = malloc(sizeof(perfil_cliente));
		perfil->socket_cliente = socket_cliente;
		perfil->nombre_resto = nombreResto;
		perfil->id_pedido = 0;
		list_add(listaPedidos,perfil);
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
			seleccionarRestaurante(recibidoSeleccionarRestaurante->restaurante,socket_cliente);
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

void iniciar_server(char* ip, char* puerto)
{
	int32_t socket_servidor;
	socket_servidor = crearSocketServidor(ip, puerto);

    while(1)
    {
    	esperar_cliente(socket_servidor);
    }

}

