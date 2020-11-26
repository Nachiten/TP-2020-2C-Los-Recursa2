/*
 * restaurante.c
 *
 *  Created on: 29 ago. 2020
 *      Author: utnso
 */


#include <stdlib.h>
#include <stdio.h>
#include "restaurante.h"

// t_list lista_cola_afinidades = [t_cola_afinidad, t_cola__afinidad2]
// t_list cocineros = [t_cocinero_1, t_cocinero_2, t_cocinero3]
// t_list hornos = [t_horno]
// t_queue cola_horno = [t_plato]
// t_list milanesa
// t_list sandwich



//{
// t_list* cola
//char* afinidad


//}t_cola_afinidad

int main(){
	socket_sindicato = establecer_conexion(ip_sindicato,puerto_sindicato);
	if(socket_sindicato < 0){
			log_info(logger, "Sindicato esta muerto, me muero yo tambien");
	exit(-2);
	}

	inicializarRestaurante();

	//tendriamo que usar hilo detacheado para esto
	//inicializar_colas();
	pthread_create(&planificacion, NULL, (void*)inicializar_colas, NULL);
	pthread_detach(planificacion);

	iniciar_server(puerto_local);

	return 0;
}

//*******************MENSAJES DE RESTO*******************

void consultar_Platos(int32_t socket_cliente){
	respuesta_consultar_platos* platos = malloc(sizeof(respuesta_consultar_platos));
	consultar_platos* consulta;
	int32_t nuevoSocketSindicato;

	consulta = malloc(sizeof(consultar_platos));
	consulta->sizeNombre = strlen(nombreRestaurante);
	consulta->sizeId = strlen(nombreRestaurante);
	consulta->nombreResto = malloc(strlen(nombreRestaurante)+1);
	consulta->id = malloc(strlen(nombreRestaurante)+1);
	strcpy(consulta->id, nombreRestaurante);
	strcpy(consulta->nombreResto, nombreRestaurante);

	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	mandar_mensaje(consulta,CONSULTAR_PLATOS, nuevoSocketSindicato);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(socket_sindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

//    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

//    printf("El size del buffer/payload es: %u", sizePayload);

    recibir_mensaje(platos,RESPUESTA_CONSULTAR_PLATOS,nuevoSocketSindicato);

    mandar_mensaje(platos,RESPUESTA_CONSULTAR_PLATOS,socket_cliente);

    free(consulta->nombreResto);
    free(consulta);
    free(platos->nombresPlatos);
    free(platos);
}

void crear_Pedido(crear_pedido* solicitudCrear, int32_t socket_cliente){
	int32_t nuevoSocketSindicato;
	guardar_pedido* pedida_a_guardar;
	respuesta_crear_pedido* respuesta;
	Pedido* pedido = malloc(sizeof(Pedido));
	pedido->socket_cliente = socket_cliente;
	pedido->numPedido = crear_id_pedidos();

	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
	    exit(-2);
	}

	list_add(listaPedidos,pedido);

	pedida_a_guardar = malloc(sizeof(guardar_pedido));
	pedida_a_guardar->idPedido = pedido->numPedido;
	pedida_a_guardar->largoNombreRestaurante = strlen(nombreRestaurante);
	pedida_a_guardar->nombreRestaurante = malloc(sizeof(strlen(nombreRestaurante) + 1));
	strcpy(pedida_a_guardar->nombreRestaurante,nombreRestaurante);

	mandar_mensaje(pedida_a_guardar, GUARDAR_PEDIDO, nuevoSocketSindicato);

	respuesta = malloc(sizeof(respuesta_crear_pedido));
	respuesta->idPedido = pedido->numPedido;

	codigo_operacion codigoRecibido;
    bytesRecibidos(recv(socket_sindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    uint32_t sizePayload;
	bytesRecibidos(recv(socket_sindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

	if(codigoRecibido != 24){
		printf("problemas al recibir respuesta de guardar pedido en sindic");
	} else {
		recibir_mensaje(respuesta, RESPUESTA_GUARDAR_PEDIDO, socket_sindicato);

		mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socket_cliente);
	}
	close(nuevoSocketSindicato);
    free(pedida_a_guardar->nombreRestaurante);
    free(pedida_a_guardar);
	free(respuesta);
}

void aniadir_plato(a_plato* recibidoAPlato, int32_t socket_cliente){
	respuesta_ok_error* respuesta = malloc(sizeof(respuesta_ok_error));
	int32_t nuevoSocketSindicato;
	guardar_plato* pasamanosGuardarPlato;

	if(buscar_pedido_por_id(recibidoAPlato->idPedido) != -2){

		nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
		if(nuevoSocketSindicato < 0){
			sem_wait(semLog);
			log_info(logger, "Sindicato esta muerto, me muero yo tambien");
			sem_post(semLog);
			exit(-2);
		}
		pasamanosGuardarPlato = malloc(sizeof(guardar_plato));
		pasamanosGuardarPlato->nombrePlato = malloc(strlen(recibidoAPlato->nombrePlato)+1);
		pasamanosGuardarPlato->largoNombrePlato = strlen(recibidoAPlato->nombrePlato);
		pasamanosGuardarPlato->nombreRestaurante = malloc(strlen(nombreRestaurante)+1);
	    pasamanosGuardarPlato->largoNombreRestaurante = strlen(nombreRestaurante);
		strcpy(pasamanosGuardarPlato->nombrePlato, recibidoAPlato->nombrePlato);
		strcpy(pasamanosGuardarPlato->nombreRestaurante, nombreRestaurante);
		pasamanosGuardarPlato->idPedido = recibidoAPlato->idPedido;
		pasamanosGuardarPlato->cantidadPlatos = 1;

		mandar_mensaje(pasamanosGuardarPlato, GUARDAR_PLATO, nuevoSocketSindicato);

		codigo_operacion codigoRecibido;
	    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

//	    printf("El codigo recibido del emisor es: %d", codigoRecibido);

	    uint32_t sizePayload;
	    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

//	    printf("El size del buffer/payload es: %u", sizePayload);

	    recibir_mensaje(respuesta,RESPUESTA_GUARDAR_PLATO, nuevoSocketSindicato);

	    mandar_mensaje(respuesta,RESPUESTA_A_PLATO,socket_cliente);
	    close(nuevoSocketSindicato);
	}else{
		respuesta->respuesta = 0;
		mandar_mensaje(respuesta,RESPUESTA_A_PLATO,socket_cliente);
	}
    free(pasamanosGuardarPlato->nombrePlato);
    free(pasamanosGuardarPlato->nombreRestaurante);
    free(pasamanosGuardarPlato);
	free(respuesta);
}

void confirmar_Pedido(int32_t id, int32_t socket_cliente){
	int32_t nuevoSocketSindicato;

	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	obtener_pedido* datosPedido = malloc(sizeof(obtener_pedido));
	datosPedido->idPedido = id;
	datosPedido->largoNombreRestaurante = strlen(nombreRestaurante);
	datosPedido->nombreRestaurante = malloc(strlen(nombreRestaurante) +1);
	strcpy(datosPedido->nombreRestaurante,nombreRestaurante);

	respuesta_obtener_pedido* pedido = malloc(sizeof(respuesta_obtener_pedido));
	int i = 0;

	mandar_mensaje(datosPedido,OBTENER_PEDIDO,nuevoSocketSindicato);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

//    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

//    printf("El size del buffer/payload es: %u", sizePayload);

    recibir_mensaje(pedido,RESPUESTA_OBTENER_PEDIDO,nuevoSocketSindicato);

	char** listaComidas = string_get_string_as_array(pedido->comidas);
	char** listaComidasTotales = string_get_string_as_array(pedido->cantTotales);

	int respuesta;
	while(listaComidas[i] != NULL){
		respuesta = preparar_pcb_plato(listaComidas[i],listaComidasTotales[i]);
		if(respuesta == 0){
			break;
		}
	}

	respuesta_ok_error* respuestaAMandar = malloc(sizeof(respuesta_ok_error));
	respuestaAMandar->respuesta = respuesta;

	mandar_mensaje(respuestaAMandar,RESPUESTA_CONFIRMAR_PEDIDO,socket_cliente);

	free(datosPedido->nombreRestaurante);
	free(datosPedido);
	free(pedido->cantTotales);
	free(pedido->cantListas);
	free(pedido->comidas);
	free(pedido);
	free(respuestaAMandar);
	close(nuevoSocketSindicato);
}
void consultar_Pedido(int32_t id,int32_t socket_cliente){
	int32_t nuevoSocketSindicato;

	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	obtener_pedido* datosPedido = malloc(sizeof(obtener_pedido));
	datosPedido->idPedido = id;
	datosPedido->largoNombreRestaurante = strlen(nombreRestaurante);
	datosPedido->nombreRestaurante = malloc(strlen(nombreRestaurante) +1);
	strcpy(datosPedido->nombreRestaurante,nombreRestaurante);

	respuesta_obtener_pedido* pedido = malloc(sizeof(respuesta_obtener_pedido));

	mandar_mensaje(datosPedido, OBTENER_PEDIDO, nuevoSocketSindicato);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

//  printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

//  printf("El size del buffer/payload es: %u", sizePayload);

    recibir_mensaje(pedido,RESPUESTA_OBTENER_PEDIDO,nuevoSocketSindicato);

    mandar_mensaje(pedido,RESPUESTA_CONSULTAR_PEDIDO,socket_cliente);
}

//*******************FUNCIONES DE RESTO*******************
int preparar_pcb_plato(char* nombreComida, char* cantComida){
	int i = 0;

	obtener_receta* receta_a_buscar = malloc(sizeof(obtener_receta));
	receta_a_buscar->largoNombreReceta = strlen(nombreComida);
	receta_a_buscar->nombreReceta = malloc(strlen(nombreComida) +1);
	strcpy(receta_a_buscar->nombreReceta,nombreComida);

	mandar_mensaje(receta_a_buscar,OBTENER_RECETA,socket_sindicato);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(socket_sindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(socket_sindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    printf("El size del buffer/payload es: %u", sizePayload);

    respuesta_obtener_receta* receta_obtenida = malloc(sizeof(respuesta_obtener_receta));

    recibir_mensaje(receta_obtenida,RESPUESTA_OBTENER_PEDIDO,socket_sindicato);

    if(strcmp(receta_obtenida->pasos,"[]") != 0){
    	char** listaPasos = string_get_string_as_array(receta_obtenida->pasos);
    	char** listaDuracion = string_get_string_as_array(receta_obtenida->tiempoPasos);

    	int cantPlatos = atoi(cantComida);

    	for(int j = 0;j<cantPlatos;j++){
    		pcb_plato* plato = malloc(sizeof(pcb_plato));
    		plato->nombrePlato = malloc(strlen(nombreComida) +1);
    		strcpy(plato->nombrePlato,nombreComida);
    		plato->pasosReceta = list_create();

    		while(listaPasos[i] != NULL){
    			paso_receta* paso = malloc(sizeof(paso_receta));

    			if(strcmp(listaPasos[i],"Hornear") == 0){
    				paso->accion = HORNEAR;
    				paso->duracionAccion = atoi(listaDuracion[i]);

    			}else if(strcmp(listaPasos[i],"Reposar") == 0){
    				paso->accion = REPOSAR;
    				paso->duracionAccion = atoi(listaDuracion[i]);

    			}else{
    				paso->accion = OTRO;
    				paso->duracionAccion = atoi(listaDuracion[i]);
    			}
    			list_add(plato->pasosReceta,paso);
    		}
    		agregarANew(plato);
    	}
    	freeDeArray(listaPasos);
    	freeDeArray(listaDuracion);

    	return 1;

    }else{
    	return 0;
    }

}

void inicializar_semaforos(){
	semId = malloc(sizeof(sem_t));
	semLog = malloc(sizeof(sem_t));

	sem_init(semId, 0, 1);
	sem_init(semLog, 0, 1);
}

void inicializar_colas(){
	listaPedidos = list_create(); //inicializo la lista de pedidos
}

int32_t crear_id_pedidos(){
	sem_wait(semId);
	id_global += 1;
	sem_post(semId);
	return id_global;
}

int buscar_pedido_por_id(uint32_t id_pedido){
	Pedido* pedido;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		pedido = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if(pedido->numPedido == id_pedido){
			return i;
		}
	}
	return -2;
}

// Hacer free de un array con cosas
void freeDeArray(char** array){
    int cantidadElementosArray = cantidadDeElementosEnArray(array);

    int i;

    for (i = cantidadElementosArray; i>= 0; i--){
        free(array[i]);
    }

    free(array);
}

//*******************FUNCIONES DEL SERVER*******************

void process_request(codigo_operacion cod_op, int32_t socket_cliente, uint32_t sizeAAllocar)  {
	a_plato* recibidoAPlato;
	consultar_pedido* recibidoConsultarPedido;
	confirmar_pedido* recibidoConfirmarPedido;
	crear_pedido* recibidoCrearPedido;
	handshake* recibidoHandshake;

	switch(cod_op){

	case CONSULTAR_PLATOS:
		consultar_Platos(socket_cliente);
		break;

	case CREAR_PEDIDO:
		recibidoCrearPedido = malloc(sizeAAllocar);
		recibir_mensaje(recibidoCrearPedido,CREAR_PEDIDO,socket_cliente);
		crear_Pedido(socket_cliente);
		free(recibidoCrearPedido->id);
		free(recibidoCrearPedido);
		break;

	case A_PLATO:
		recibidoAPlato = malloc(sizeAAllocar);
		recibir_mensaje(recibidoAPlato,A_PLATO,socket_cliente);
		aniadir_plato(recibidoAPlato,socket_cliente);
		free(recibidoAPlato->nombrePlato);
		free(recibidoAPlato);
		break;

	case CONFIRMAR_PEDIDO:
		recibidoConfirmarPedido = malloc(sizeAAllocar);
		recibir_mensaje(recibidoConfirmarPedido,CONFIRMAR_PEDIDO,socket_cliente);
		confirmar_Pedido(recibidoConfirmarPedido->idPedido,socket_cliente);
		free(recibidoConfirmarPedido->nombreRestaurante);
		free(recibidoConfirmarPedido);
		break;

	case CONSULTAR_PEDIDO:
		recibidoConsultarPedido = malloc(sizeAAllocar);
		recibir_mensaje(recibidoConsultarPedido,CONSULTAR_PEDIDO,socket_cliente);
		consultar_Pedido(recibidoConsultarPedido->idPedido, socket_cliente);
		free(recibidoConsultarPedido);
		break;

	case HANDSHAKE:
		recibidoHandshake = malloc(sizeAAllocar);
		recibir_mensaje(recibidoHandshake,HANDSHAKE,socket_cliente);
		free(recibidoHandshake->id);
		free(recibidoHandshake);
		break;

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
	//}
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
