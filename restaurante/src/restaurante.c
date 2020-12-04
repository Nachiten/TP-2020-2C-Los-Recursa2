/*
 * restaurante.c
 *
 *  Created on: 29 ago. 2020
 *      Author: utnso
 */


#include <stdlib.h>
#include <stdio.h>
#include "restaurante.h"

int main(int cantArg, char* argumentos[]){

	char* pathConfig;

	// Si no hay ningun parametro del path de config genero valor default
	if (cantArg < 2){
		printf("El path de la config no está especificado, tomando valor default.\nValor default: /home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/restaurante1.config\n");
		pathConfig = "/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/restaurante1.config";
	// Si hay un parametro asumo que es el path de la config y lo uso
	} else {
		pathConfig = argumentos[1];
	}

	inicializarRestaurante(pathConfig);

	//tendriamo que usar hilo detacheado para esto
	//inicializar_colas();
	inicializar_planificacion();
	//pthread_detach(planificacion);

	printf("Termine de ejecutar el inicializar planif\n");

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
    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
    uint32_t sizePayload;
    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    recibir_mensaje(platos,RESPUESTA_CONSULTAR_PLATOS,nuevoSocketSindicato);

    mandar_mensaje(platos,RESPUESTA_CONSULTAR_PLATOS,socket_cliente);

    close(nuevoSocketSindicato);

    free(consulta->nombreResto);
    free(consulta);
    free(platos->nombresPlatos);
    free(platos);
}

void crear_Pedido(crear_pedido* solicitudCrear, int32_t socket_cliente){
	int32_t nuevoSocketSindicato;
	guardar_pedido* pedida_a_guardar;
	respuesta_ok_error* resultado_guardar_pedido;
	respuesta_crear_pedido* respuesta;
	perfil_pedido* pedido = malloc(sizeof(perfil_pedido));
	pedido->socket_cliente = socket_cliente;
	pedido->numPedido = crear_id_pedidos();

	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
	    exit(-2);
	}

	sem_wait(semListaPedidos);
	list_add(listaPedidos,pedido);
	sem_post(semListaPedidos);

	pedida_a_guardar = malloc(sizeof(guardar_pedido));
	pedida_a_guardar->idPedido = pedido->numPedido;
	pedida_a_guardar->largoNombreRestaurante = strlen(nombreRestaurante);
	pedida_a_guardar->nombreRestaurante = malloc(sizeof(strlen(nombreRestaurante) + 1));
	strcpy(pedida_a_guardar->nombreRestaurante,nombreRestaurante);

	mandar_mensaje(pedida_a_guardar, GUARDAR_PEDIDO, nuevoSocketSindicato);

	respuesta = malloc(sizeof(respuesta_crear_pedido));
	respuesta->idPedido = pedido->numPedido;

	codigo_operacion codigoRecibido;
    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    uint32_t sizePayload;
	bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

	if(codigoRecibido != 24){
		printf("problemas al recibir respuesta de guardar pedido en sindic");
	} else {
		resultado_guardar_pedido = malloc(sizeof(respuesta_ok_error));
		recibir_mensaje(resultado_guardar_pedido, RESPUESTA_GUARDAR_PEDIDO, nuevoSocketSindicato);

		if(resultado_guardar_pedido->respuesta == 1)
		{
			mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socket_cliente);
		}

		else
		{
			respuesta->idPedido = 0;
			mandar_mensaje(respuesta,RESPUESTA_CREAR_PEDIDO,socket_cliente);
		}
		free(resultado_guardar_pedido);
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

	//sem_wait(semListaPedidos);
	if(buscar_pedido_por_id(recibidoAPlato->idPedido) != -2){

		printf("Encontre el pedido buscado.\n");
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

	    free(pasamanosGuardarPlato->nombrePlato);
	    free(pasamanosGuardarPlato->nombreRestaurante);
	    free(pasamanosGuardarPlato);

	}else{
		respuesta->respuesta = 0;
		mandar_mensaje(respuesta,RESPUESTA_A_PLATO,socket_cliente);
	}
	//sem_post(semListaPedidos);
	free(respuesta);
}

void confirmar_Pedido(int32_t id, int32_t socket_cliente){
	int32_t nuevoSocketSindicato, nuevoSocketSindicato2;
	perfil_pedido* elPedidoQueFueConfirmado;
	int32_t indiceDelPedidoQueFueConfirmado;
	codigo_operacion codigoRecibido;
	uint32_t sizePayload;
	int respuesta;

	indiceDelPedidoQueFueConfirmado = buscar_pedido_por_id(id);
		if(indiceDelPedidoQueFueConfirmado == -2){
			log_error(logger, "[RESTAURANTE] Se desea confirmar el pedido < %d >"
			   ", pero dicho pedido no se encuentra en los registros del restaurante. Safaste porque lo tiene Sindicato."
				, id);
		}

	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
		if(nuevoSocketSindicato < 0){
			sem_wait(semLog);
			log_info(logger, "Sindicato esta muerto, me muero yo tambien");
			sem_post(semLog);
			exit(-2);
	}

	guardar_pedido* datosPedido = malloc(sizeof(guardar_pedido));
	datosPedido->idPedido = id;
	datosPedido->largoNombreRestaurante = strlen(nombreRestaurante);
	datosPedido->nombreRestaurante = malloc(strlen(nombreRestaurante) +1);
	strcpy(datosPedido->nombreRestaurante,nombreRestaurante);

	mandar_mensaje(datosPedido,OBTENER_PEDIDO,nuevoSocketSindicato);

	respuesta_obtener_pedido* pedido = malloc(sizeof(respuesta_obtener_pedido));
	int i = 0;

    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
    recibir_mensaje(pedido,RESPUESTA_OBTENER_PEDIDO,nuevoSocketSindicato);

    close(nuevoSocketSindicato);

    respuesta = 0;
    respuesta_ok_error* respuestaAMandar = malloc(sizeof(respuesta_ok_error));

    if(pedido->estado == 0 || pedido->estado == 2 || pedido->estado == 3){
    //el pedido ya esta confirmado, finalizado de antes o directamente no existe, hay q denegar y frenar la operacion
    	respuestaAMandar->respuesta = respuesta;

    	mandar_mensaje(respuestaAMandar,RESPUESTA_CONFIRMAR_PEDIDO,socket_cliente);

    	free(datosPedido->nombreRestaurante);
		free(datosPedido);
		free(pedido->cantTotales);
		free(pedido->cantListas);
		free(pedido->comidas);
		free(pedido);
		free(respuestaAMandar);

    	return;
    }
    //el estado es 1, PENDIENTE, por lo que me conecto a SINDICATO DE NUEVO para pedirle que
    //actualice el estado de pedido a CONFIRMADO
    nuevoSocketSindicato2 = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato2 < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}
    guardar_pedido* datosPedidoAConfirmar = malloc(sizeof(guardar_pedido));
    datosPedidoAConfirmar->idPedido = id;
    datosPedidoAConfirmar->largoNombreRestaurante = strlen(nombreRestaurante);
    datosPedidoAConfirmar->nombreRestaurante = malloc(strlen(nombreRestaurante) +1);
	strcpy(datosPedidoAConfirmar->nombreRestaurante,nombreRestaurante);

	mandar_mensaje(datosPedido, CONFIRMAR_PEDIDO,nuevoSocketSindicato2);

	respuesta_ok_error* respuestaConfirmacionSindicato = malloc(sizeof(respuesta_ok_error));

	bytesRecibidos(recv(nuevoSocketSindicato2, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));
	bytesRecibidos(recv(nuevoSocketSindicato2, &sizePayload, sizeof(uint32_t), MSG_WAITALL));
    recibir_mensaje(respuestaConfirmacionSindicato, RESPUESTA_CONFIRMAR_PEDIDO, nuevoSocketSindicato2);

	close(nuevoSocketSindicato2);

    if(respuestaConfirmacionSindicato->respuesta == 0){
    	//fracaso la confirmacion final
		respuestaAMandar->respuesta = respuesta;

		mandar_mensaje(respuestaAMandar,RESPUESTA_CONFIRMAR_PEDIDO,socket_cliente);

		free(datosPedido->nombreRestaurante);
		free(datosPedido);
		free(pedido->cantTotales);
		free(pedido->cantListas);
		free(pedido->comidas);
		free(pedido);
		free(respuestaAMandar);
		free(datosPedidoAConfirmar->nombreRestaurante);
		free(datosPedidoAConfirmar);
		free(respuestaConfirmacionSindicato);

		return;
    }
    //la confirmacion final fue exitosa, comienzo a preparar los pcbs para planificarlos

	char** listaComidas = string_get_string_as_array(pedido->comidas);
	char** listaComidasTotales = string_get_string_as_array(pedido->cantTotales);

	while(listaComidas[i] != NULL){
		respuesta = preparar_pcb_plato(id, listaComidas[i],listaComidasTotales[i]);
		if(respuesta == 0){
			break;
		}
		i++;
	}

	//todos los pcbs fueron cargados correctamente, seteo el socket actual para mandar las notificaciones posteriores
	if(respuesta == 1){
		sem_wait(semListaPedidos);
		elPedidoQueFueConfirmado = list_get(listaPedidos, indiceDelPedidoQueFueConfirmado);
		elPedidoQueFueConfirmado->socket_cliente = socket_cliente;
		sem_post(semListaPedidos);
	}

	//mando la respuesta definitiva con el flujo completo
	//sera 0 si fracaso la preparacion de algun pcb, caso contrario sera 1 (exitosa)
	respuestaAMandar->respuesta = respuesta;

	mandar_mensaje(respuestaAMandar,RESPUESTA_CONFIRMAR_PEDIDO,socket_cliente);

	free(datosPedido->nombreRestaurante);
	free(datosPedido);
	free(pedido->cantTotales);
	free(pedido->cantListas);
	free(pedido->comidas);
	free(pedido);
	free(respuestaAMandar);
	free(datosPedidoAConfirmar->nombreRestaurante);
	free(datosPedidoAConfirmar);
	free(respuestaConfirmacionSindicato);
}


void consultar_Pedido(int32_t id, int32_t socket_cliente){
	int32_t nuevoSocketSindicato, indiceDelPedidoQueFueConsultado;

	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	indiceDelPedidoQueFueConsultado = buscar_pedido_por_id(id);
	if(indiceDelPedidoQueFueConsultado == -2){
		log_error(logger, "[RESTAURANTE] Se desea consultar el pedido < %d >"
		   ", pero dicho pedido no se encuentra en los registros del restaurante. Safaste porque lo tiene Sindicato."
			, id);
	}

	guardar_pedido* datosPedido = malloc(sizeof(guardar_pedido));
	datosPedido->idPedido = id;
	datosPedido->largoNombreRestaurante = strlen(nombreRestaurante);
	datosPedido->nombreRestaurante = malloc(strlen(nombreRestaurante) +1);
	strcpy(datosPedido->nombreRestaurante,nombreRestaurante);

	mandar_mensaje(datosPedido, OBTENER_PEDIDO, nuevoSocketSindicato);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

//  printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

//  printf("El size del buffer/payload es: %u", sizePayload);

    respuesta_obtener_pedido* pedidoObtenido = malloc(sizeof(respuesta_obtener_pedido));

    recibir_mensaje(pedidoObtenido, RESPUESTA_OBTENER_PEDIDO, nuevoSocketSindicato);

    respuesta_consultar_pedido* pedidoConsultado = malloc(sizeof(respuesta_consultar_pedido));

    //si quisieramos validar el estado para ver si le contestamos al cliente una excepcion, aca iria un if
    pedidoConsultado->estado = pedidoObtenido->estado;
    pedidoConsultado->largoNombreRestaurante = strlen(nombreRestaurante);
    pedidoConsultado->nombreRestaurante = malloc(strlen(nombreRestaurante)+1);
    strcpy(pedidoConsultado->nombreRestaurante, nombreRestaurante);

    pedidoConsultado->sizeComidas = pedidoObtenido->sizeComidas;
    pedidoConsultado->sizeCantListas = pedidoObtenido->sizeCantListas;
    pedidoConsultado->sizeCantTotales = pedidoObtenido->sizeCantTotales;
    pedidoConsultado->comidas = malloc(pedidoConsultado->sizeComidas+1);
    pedidoConsultado->cantListas = malloc(pedidoConsultado->sizeCantListas+1);
    pedidoConsultado->cantTotales = malloc(pedidoConsultado->sizeCantTotales+1);
    strcpy(pedidoConsultado->comidas, pedidoObtenido->comidas);
    strcpy(pedidoConsultado->cantListas, pedidoObtenido->cantListas);
    strcpy(pedidoConsultado->cantTotales, pedidoObtenido->cantTotales);

    mandar_mensaje(pedidoConsultado, RESPUESTA_CONSULTAR_PEDIDO, socket_cliente);

    close(nuevoSocketSindicato);

    free(pedidoObtenido->comidas);
    free(pedidoObtenido->cantListas);
    free(pedidoObtenido->cantTotales);
    free(pedidoObtenido);
    free(pedidoConsultado->nombreRestaurante);
    free(pedidoConsultado->comidas);
    free(pedidoConsultado->cantListas);
    free(pedidoConsultado->cantTotales);
    free(pedidoConsultado);
}

//*******************FUNCIONES DE RESTO*******************
int preparar_pcb_plato(uint32_t idPedido, char* nombreComida, char* cantComida){
	int i = 0;

	int32_t nuevoSocketSindicato;
	obtener_receta* receta_a_buscar = malloc(sizeof(obtener_receta));
	receta_a_buscar->largoNombreReceta = strlen(nombreComida);
	receta_a_buscar->nombreReceta = malloc(strlen(nombreComida) +1);
	strcpy(receta_a_buscar->nombreReceta,nombreComida);


	nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	mandar_mensaje(receta_a_buscar,OBTENER_RECETA,nuevoSocketSindicato);

    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(nuevoSocketSindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    uint32_t sizePayload;
    bytesRecibidos(recv(nuevoSocketSindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    respuesta_obtener_receta* receta_obtenida = malloc(sizeof(respuesta_obtener_receta));

    recibir_mensaje(receta_obtenida,RESPUESTA_OBTENER_RECETA,nuevoSocketSindicato);

    close(nuevoSocketSindicato);

    if(strcmp(receta_obtenida->pasos,"[]") != 0 && strcmp(receta_obtenida->pasos,"[Error]") != 0){
    	char** listaPasos = string_get_string_as_array(receta_obtenida->pasos);
    	char** listaDuracion = string_get_string_as_array(receta_obtenida->tiempoPasos);

    	int cantPlatos = atoi(cantComida);

    	for(int j = 0;j<cantPlatos;j++){
    		i=0;
    		pcb_plato* plato = malloc(sizeof(pcb_plato));
    		plato->idPedido = idPedido;
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
    			i++;
    		}
    		agregarANew(plato);
    	}
    	freeDeArray(listaPasos);
    	freeDeArray(listaDuracion);
    	free(receta_a_buscar->nombreReceta);
		free(receta_a_buscar);
		free(receta_obtenida->pasos);
		free(receta_obtenida->tiempoPasos);
		free(receta_obtenida);

    	return 1;


    }else{
    	free(receta_a_buscar->nombreReceta);
		free(receta_a_buscar);
		free(receta_obtenida->pasos);
		free(receta_obtenida->tiempoPasos);
		free(receta_obtenida);

    	return 0;
    }



}


int32_t crear_id_pedidos(){
	sem_wait(semId);
	id_global += 1;
	sem_post(semId);
	return id_global;
}

int buscar_pedido_por_id(uint32_t id_pedido){
	perfil_pedido* pedido;
	int retorno = -2;
	sem_wait(semListaPedidos);
	for(int i = 0; i < listaPedidos->elements_count; i++){

		pedido = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if(pedido->numPedido == id_pedido){

			retorno = i;
		}
	}
	sem_post(semListaPedidos);
	return retorno;
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
		recibidoCrearPedido = malloc(sizeof(crear_pedido));
		recibir_mensaje(recibidoCrearPedido,CREAR_PEDIDO,socket_cliente);
		crear_Pedido(recibidoCrearPedido, socket_cliente);
		free(recibidoCrearPedido->id);
		free(recibidoCrearPedido);
		break;

	case A_PLATO:
		recibidoAPlato = malloc(sizeof(a_plato));
		recibir_mensaje(recibidoAPlato,A_PLATO,socket_cliente);
		aniadir_plato(recibidoAPlato,socket_cliente);
		free(recibidoAPlato->nombrePlato);
		free(recibidoAPlato->id);
		free(recibidoAPlato);
		break;

	case CONFIRMAR_PEDIDO:
		recibidoConfirmarPedido = malloc(sizeof(confirmar_pedido));
		recibir_mensaje(recibidoConfirmarPedido,CONFIRMAR_PEDIDO,socket_cliente);
		confirmar_Pedido(recibidoConfirmarPedido->idPedido,socket_cliente);
		free(recibidoConfirmarPedido->nombreRestaurante);
		free(recibidoConfirmarPedido);
		break;

	case CONSULTAR_PEDIDO:
		recibidoConsultarPedido = malloc(sizeof(consultar_pedido));
		recibir_mensaje(recibidoConsultarPedido,CONSULTAR_PEDIDO,socket_cliente);
		consultar_Pedido(recibidoConsultarPedido->idPedido, socket_cliente);
		free(recibidoConsultarPedido->id);
		free(recibidoConsultarPedido);
		break;

	case HANDSHAKE:
		recibidoHandshake = malloc(sizeof(handshake));
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

		if(cod_op != CONSULTAR_RESTAURANTES)
		{
			//si se cayo la conexion, basicamente no hacemos hada
			if(recibidos < 1)
			{
				cod_op = 0;
				sizeAAllocar = 0;
			}

			//si la conexion NO se cayo, intento recibir lo que sigue
			else
			{
				//recibo tamaño de lo que sigue
				recibidosSize = recv(*socket, &sizeAAllocar, sizeof(int32_t), MSG_WAITALL);
				bytesRecibidos(recibidosSize);

				//si se cayo la conexion, no se hace nada con esto
				if(recibidosSize < 1)
				{
					cod_op = 0;
					sizeAAllocar = 0;
				}

			}

			if(cod_op != 0)
			{
				printf("Tamaño de lo que sigue en el buffer: %u.\n", sizeAAllocar);
			}

			//mando lo que me llego para que lo procesen
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
