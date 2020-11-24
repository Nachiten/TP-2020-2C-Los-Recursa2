/*
 * planificacion.c
 *
 *  Created on: 28 sep. 2020
 *      Author: utnso
 */

#include "planificacion.h"

/*
 * Semaforo execDisponible -> Empieza en 3
 * se hace wait cuando se ocupa un exec
 * se hace signal cuando se libera
 *
 * Ready hace wait para mandar pedido nuevo a exec
 *
 *
 */

void iniciarPlanificacion(){
	printf("Iniciando Planificacion.\n");

	colaNew = queue_create();
	colaReady = list_create();
    colaBlock = list_create();
    pedidosListos = list_create();

	//sleep(7);

    //------------------------------------PRUEBAS-------------------------------------------------------
/*
	pcb_pedido* unPedido = malloc(sizeof(pcb_pedido));
	// 7 distancia hacia restaurant
	unPedido->posRestauranteX = 8;
	unPedido->posRestauranteY = 8;
	// 5 distancia hacia cliente
	unPedido->posClienteX = 11;
	unPedido->posClienteY = 11;
	unPedido->pedidoID = 1;

	agregarANew(unPedido);

	pcb_pedido* otroPedido = malloc(sizeof(pcb_pedido));
	otroPedido->posRestauranteX = 5;
	otroPedido->posRestauranteY = 5;
	otroPedido->posClienteX = 50;
	otroPedido->posClienteY = 50;
	otroPedido->pedidoID = 2;

    agregarANew(otroPedido);

    sleep(4);

    pcb_pedido* unPedido3 = malloc(sizeof(pcb_pedido));
	unPedido3->posRestauranteX = 2;
	unPedido3->posRestauranteY = 2;
	unPedido3->posClienteX = 20;
	unPedido3->posClienteY = 20;
	unPedido3->pedidoID = 3;

	agregarANew(unPedido3);
*/
	pthread_t hiloBlockReady;
	pthread_create(&hiloBlockReady, NULL, (void*)hiloBlock_Ready, NULL);

	pthread_t hiloNewReady;
	pthread_create(&hiloNewReady, NULL, (void*)hiloNew_Ready, NULL);

	pthread_t threadHilosMaestro;
	pthread_create(&threadHilosMaestro, NULL, (void*)hiloCiclosMaestro, NULL);

	int i;
	// Genero un hilo por cada estado exec
	for (i = 0; i < GRADO_MULTIPROCE; i++){
		pthread_t hiloColaExec;

		int* numeroHilo = malloc(sizeof(int));

		*numeroHilo = i;

		pthread_create(&hiloColaExec, NULL, (void*)hiloExec, numeroHilo);
	}

	/*
	sleep(25);

	guardarPedidoListo(1);
	guardarPedidoListo(2);

	guardarPedidoListo(3);
	guardarPedidoListo(4);
*/

//	pcb_pedido* unPedido2 = malloc(sizeof(pcb_pedido));
//	unPedido2->posRestauranteX = 3;
//	unPedido2->posRestauranteY = 5;
//	unPedido2->posClienteX = 3;
//	unPedido2->posClienteY = 5;
//	unPedido2->pedidoID = 2;
//
//	agregarANew(unPedido2);
//
//	pcb_pedido* unPedido3 = malloc(sizeof(pcb_pedido));
//	unPedido3->posRestauranteX = 7;
//	unPedido3->posRestauranteY = 8;
//	unPedido3->posClienteX = 3;
//	unPedido3->posClienteY = 5;
//	unPedido3->pedidoID = 3;
//
//	agregarANew(unPedido3);
//
//	sleep(20);
//
//	guardarPedidoListo(2);
//
//	sleep(20);
//
//	guardarPedidoListo(3);
//
//	pcb_pedido* unPedido4 = malloc(sizeof(pcb_pedido));
//	unPedido4->posRestauranteX = 10;
//	unPedido4->posRestauranteY = 11;
//	unPedido4->posClienteX = 2;
//	unPedido4->posClienteY = 3;
//	unPedido4->pedidoID = 4;
//
//	agregarANew(unPedido4);
//
//	pcb_pedido* unPedido5 = malloc(sizeof(pcb_pedido));
//	unPedido5->posRestauranteX = 12;
//	unPedido5->posRestauranteY = 9;
//	unPedido5->posClienteX = 1;
//	unPedido5->posClienteY = 2;
//	unPedido5->pedidoID = 5;
//
//	agregarANew(unPedido5);
//
//	guardarPedidoListo(4);
//	guardarPedidoListo(5);
    sem_post(planificacionInicializada);
	pthread_join(hiloNewReady, NULL);
}

void guardarPedidoListo(int idPedido){
	int* pedidoAGuardar = malloc(sizeof(int));

	*pedidoAGuardar = idPedido;

	sem_wait(mutexPedidosListos);
	// Agrego un nuevo pedido listo a la lista
	list_add(pedidosListos, pedidoAGuardar);
	sem_post(mutexPedidosListos);

	log_trace(logger, "[GuardarPedido] Se guarda como listo el pedido %i\n", idPedido);
}

void eliminarPedidoListo(int idPedido){
	sem_wait(mutexPedidosListos);
	int i;
	// Recorro la lista de pedidos listos a ver si encuentro el que necesito
	for (i = 0; i< list_size(pedidosListos); i++){
		int* pedidoCandidato = list_get(pedidosListos, i);

		if (*pedidoCandidato == idPedido){
			int* pedidoAEliminar = list_remove(pedidosListos, i);
			free(pedidoAEliminar);
			log_trace(logger, "[Pedidos] Eliminado pedido %i de pedidos listos.\n", idPedido);
		}
	}
	sem_post(mutexPedidosListos);
}


int checkearPedidoListo(int idPedido){

	int retorno = 0;

	sem_wait(mutexPedidosListos);
	int i;
	// Recorro la lista de pedidos listos a ver si encuentro el que necesito
	for (i = 0; i< list_size(pedidosListos); i++){
		int* pedidoCandidato = list_get(pedidosListos, i);

		if (*pedidoCandidato == idPedido){
			retorno = 1;
			break;
		}
	}
	sem_post(mutexPedidosListos);

	return retorno;
}

void pedido_entregado(int32_t id_pedido){
	int numCliente = buscar_pedido_por_id(id_pedido);
	perfil_cliente* cliente = list_get(listaPedidos,numCliente);
	int32_t nuevoSocketComanda, sizeAAllocar = 0;
	uint32_t exito = 0;

	guardar_pedido* pedidoFinalizado;
	pedidoFinalizado = malloc(sizeof(finalizar_pedido));
	pedidoFinalizado->idPedido = id_pedido;
	pedidoFinalizado->nombreRestaurante = malloc(strlen(cliente->nombre_resto)+1);
	strcpy(pedidoFinalizado->nombreRestaurante, cliente->nombre_resto);
	pedidoFinalizado->largoNombreRestaurante = strlen(cliente->nombre_resto);

	/* esto no seria necesario, cliente ya sabe desserializar finalizar_pedido
	pedido_finalizado* aMandar = malloc(sizeof(pedido_finalizado));
	aMandar->mensaje = "Pedido Finalizado";
	aMandar->sizeMensaje = strlen("Pedido Finalizado");
    */

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	mandar_mensaje(pedidoFinalizado, FINALIZAR_PEDIDO, nuevoSocketComanda);

	respuesta_ok_error* respuestaConfirmacionComanda = malloc(sizeof(respuesta_ok_error));
	los_recv_repetitivos(nuevoSocketComanda, &exito, &sizeAAllocar);


		if(exito == 1)
		{
			recibir_mensaje(respuestaConfirmacionComanda,RESPUESTA_FINALIZAR_PEDIDO, nuevoSocketComanda);
			sem_wait(semLog);
			log_info(logger, "El intento de confirmar el pedido con comanda fue: %i.",
					resultadoDeRespuesta(respuestaConfirmacionComanda->respuesta));
			sem_post(semLog);
		}
		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de FINALIZAR Pedido a comanda.\n");
		}

	close(nuevoSocketComanda);
	free(respuestaConfirmacionComanda);

	mandar_mensaje(pedidoFinalizado, FINALIZAR_PEDIDO, cliente->socket_cliente);

	respuesta_ok_error* respuestaConfirmacionCliente = malloc(sizeof(respuesta_ok_error));
	los_recv_repetitivos(nuevoSocketComanda, &exito, &sizeAAllocar);

	    if(exito == 1)
		{
			recibir_mensaje(respuestaConfirmacionCliente,RESPUESTA_FINALIZAR_PEDIDO, cliente->socket_cliente);
			sem_wait(semLog);
			log_info(logger, "El intento de confirmar el pedido con el cliente fue: %i",
					resultadoDeRespuesta(respuestaConfirmacionCliente->respuesta));
			sem_post(semLog);
		}
		else
		{
			printf("Ocurrió un error al intentar recibir la respuesta de FINALIZAR Pedido a cliente.\n");
		}

	 free(respuestaConfirmacionCliente);
	 free(pedidoFinalizado->nombreRestaurante);
	 free(pedidoFinalizado);

}


void moverPedidoDeBlockAReady(int indicePedido){
	sem_wait(mutexBlock);
	pcb_pedido* pedidoAReady = list_remove(colaBlock, indicePedido);
	sem_post(mutexBlock);

	pedidoAReady->accionBlocked = NO_BLOCK;

	agregarAReady(pedidoAReady);
}

void moverPedidoDeBlockAExit(int indicePedido){
	sem_wait(mutexBlock);
	pcb_pedido* pedidoAExit = list_remove(colaBlock, indicePedido);
	sem_post(mutexBlock);

	pedidoAExit->accionBlocked = NO_BLOCK;

	agregarAExit(pedidoAExit);
}

void hiloBlock_Ready(){

	while(1){

		sem_wait(habilitarCicloBlockReady);

		if(algoritmo == HRRN){
			int i;
			int elementosEnReady = list_size(colaReady);
			sem_wait(mutexReady);

			// Escaneo los elementos de ready para sumarles tiempo de espera
			for (i = 0; i < elementosEnReady; i++){
				pcb_pedido* pedidoActual = list_get(colaReady, i);
				pedidoActual->tiempoEspera += 1;
			}
			sem_post(mutexReady);
		}

		int elementosEnBlock = list_size(colaBlock);

		if(elementosEnBlock == 0)
			log_trace(logger, "[BLOCK] No hay pedidos en block.");

		int indicePedido;
		// Escaneo todos los elementos en block para sumar 1 ciclo de descanso a cada proceso
		for (indicePedido = 0; indicePedido < elementosEnBlock; indicePedido++){

			sem_wait(mutexBlock);
			pcb_pedido* pedidoActual = list_get(colaBlock, indicePedido);
			sem_post(mutexBlock);

			repartidor* repartidorActual = pedidoActual->repartidorAsignado;

			switch(pedidoActual->accionBlocked){
				case DESCANSANDO:

					repartidorActual->tiempoDescansado++;

					log_trace(logger, "[BLOCK] El pedido %i ya descanso %i ciclos.", pedidoActual->pedidoID, repartidorActual->tiempoDescansado);

					// Ya descanso lo que tenia que descansar
					if (repartidorActual->tiempoDescansado == repartidorActual->tiempoDescanso){

						log_trace(logger, "[BLOCK] El pedido %i ya descanso todos los %i ciclos que necesitaba.", pedidoActual->pedidoID, repartidorActual->tiempoDescansado);

						if (pedidoActual->proximoEstado == READY){
							// Cuando termina de descansar y tiene que continuar rafagas

							pedidoActual->repartidorAsignado->tiempoDescansado = 0;
							pedidoActual->repartidorAsignado->cansado = 0;

							log_info(logger, "[READY] Ingresa pedido %i por terminar de descansar.", pedidoActual->pedidoID);
							moverPedidoDeBlockAReady(indicePedido);

						} else if (pedidoActual->proximoEstado == EXIT){

							// Termina de descansar y ya esta terminado
							moverPedidoDeBlockAExit(indicePedido);

						} else {
							printf("BLOCK | ERROR: El proximo estado tiene un valor invalido.\n");
							exit(7);
						}

					}

					break;
				case ESPERANDO_MSG:

					// Ya esta listo el pedido
					if (checkearPedidoListo(pedidoActual->pedidoID)){
						log_trace(logger, "[BLOCK] El pedido %i ya esta listo.\n", pedidoActual->pedidoID);

						// Debe descansar antes de volver a ready
						if (pedidoActual->repartidorAsignado->cansado){
							pedidoActual->accionBlocked = DESCANSANDO;
							pedidoActual->proximoEstado = READY;

						// No esta cansado, va a ready
						} else {
							log_info(logger, "[READY] Ingresa pedido %i por ya estar listo.", pedidoActual->pedidoID);
							moverPedidoDeBlockAReady(indicePedido);
						}


					// No esta listo
					} else {
						log_trace(logger, "[BLOCK] El pedido %i todavia no esta listo.\n", pedidoActual->pedidoID);
					}

					break;

				case ESPERANDO_EXIT:

					// Se entrega el pedido
					log_info(logger, "[BLOCK] Repartidor %i entrega pedido %i.", pedidoActual->repartidorAsignado->numeroRepartidor, pedidoActual->pedidoID);
					pedido_entregado(pedidoActual->pedidoID);

					// Debe descansar antes de volver a ready
					if (pedidoActual->repartidorAsignado->cansado){
						pedidoActual->accionBlocked = DESCANSANDO;
						pedidoActual->proximoEstado = EXIT;

					// No esta cansado, va a exit
					} else {
						moverPedidoDeBlockAExit(indicePedido);
					}

					break;

				default:
					printf("[BLOCK | ERROR] El pedido %i esta en blocked pero no se le asigno por que esta bloqueado.\n", pedidoActual->pedidoID);
					exit(5);

					break;
			}
		}

		sem_post(finalizarCicloBlockReady);

	}
}

void hiloExec(int* numHiloExecPuntero){

	int numHiloExec = *numHiloExecPuntero;

	while(1){

		//printf("Viendo si hay alguien en ready.\n");
		pcb_pedido* pedidoAEjecutar = obtenerSiguienteDeReady();

		if (pedidoAEjecutar != NULL){

			log_info(logger, "[EXEC-%i] Ingresa pedido %i. Instruciones restantes: %i",
					numHiloExec, pedidoAEjecutar->pedidoID, pedidoAEjecutar->instruccionesTotales);

			int cantidadCiclos = 1;

			// Calculo si tiene que desalojar o no y por que razon
			while (sigoEjecutando(pedidoAEjecutar)){
				waitSemaforoHabilitarCicloExec(numHiloExec);
				log_trace(logger, "[EXEC-%i] Ejecuto ciclo numero: %i.", numHiloExec, cantidadCiclos);

				pedidoAEjecutar->instruccionesRealizadas++;
				pedidoAEjecutar->repartidorAsignado->instruccionesRealizadas++;

				log_trace(logger, "[EXEC-%i] Instrucciones restantes: %i.", numHiloExec, pedidoAEjecutar->instruccionesTotales - pedidoAEjecutar->instruccionesRealizadas);

				signalSemaforoFinalizarCicloExec(numHiloExec);

				cantidadCiclos++;
			}

			agregarABlock(pedidoAEjecutar);

		} else {
			waitSemaforoHabilitarCicloExec(numHiloExec);
			log_trace(logger, "[EXEC-%i] Desperdicio un ciclo porque no hay nadie en ready.", numHiloExec);
			signalSemaforoFinalizarCicloExec(numHiloExec);
		}

	}
}

// 0: No se desaloja, 1: Se desaloja por estar cansado, 2: Se desaloja porque termino la rafaga
//int codigoDesalojo(pcb_pedido* unPedido){
//
//	// Si termino de reliazar toda la rafaga
//	if (unPedido->instruccionesRealizadas == unPedido->instruccionesTotales){
//		return 2;
//	}
//
//	// Si se canso ya y debe ir a blocked
//	if (unPedido->instruccionesRealizadas == unPedido->repartidorAsignado->frecuenciaDescanso){
//		return 1;
//	}
//
//	// Ninguna (sigue ejecutando)
//	return 0;
//}

// 1 Si sigue ejecutando 0 Si desalojo
int sigoEjecutando(pcb_pedido* pedidoEnEjecucion){
	int retorno = 1;

	repartidor* repartidorActual = pedidoEnEjecucion->repartidorAsignado;

	// Se canso
	if (repartidorActual->instruccionesRealizadas == repartidorActual->frecuenciaDescanso){
		pedidoEnEjecucion->accionBlocked = DESCANSANDO;
		repartidorActual->cansado = 1;

		// Acciones al cansarse
		pedidoEnEjecucion->instruccionesAnteriores = pedidoEnEjecucion->instruccionesRealizadas;
		pedidoEnEjecucion->instruccionesTotales -= pedidoEnEjecucion->instruccionesRealizadas;
		pedidoEnEjecucion->instruccionesRealizadas = 0;
		pedidoEnEjecucion->estimacionAnterior = pedidoEnEjecucion->estimacionActual;
		pedidoEnEjecucion->estimacionActual = alpha*pedidoEnEjecucion->instruccionesAnteriores
				                            + (1-alpha)*pedidoEnEjecucion->estimacionAnterior;
		repartidorActual->instruccionesRealizadas = 0;

		log_info(logger, "[BLOCK] Ingresa pedido %i por estar cansado.", pedidoEnEjecucion->pedidoID);

		retorno = 0;
	}

	// Termino la rafaga
	if (pedidoEnEjecucion->instruccionesRealizadas == pedidoEnEjecucion->instruccionesTotales){

		// Me fijo cual era el objetivo
		if (pedidoEnEjecucion->objetivo == RESTAURANTE){
			// La accoin en block es esperar el mensaje de pedido listo
			pedidoEnEjecucion->accionBlocked = ESPERANDO_MSG;

			//en el caso de que sea solo termino de rafaga, pero el vago no se canso tengo que ajustar aca tambien mis estimaciones
			//caso contrario dejo la estimacion como estaba, es correcta la que ya fue calculada arriba
			if(repartidorActual->cansado == 0){
				pedidoEnEjecucion->instruccionesAnteriores = pedidoEnEjecucion->instruccionesRealizadas;
				pedidoEnEjecucion->estimacionAnterior = pedidoEnEjecucion->estimacionActual;
				pedidoEnEjecucion->estimacionActual = alpha*pedidoEnEjecucion->instruccionesAnteriores
								                    + (1-alpha)*pedidoEnEjecucion->estimacionAnterior;
			}

			// Acciones al llegar al resutante
			// Cuando se inicie la nueva rafaga tendra 0 instrucciones ejecutadas
			pedidoEnEjecucion->instruccionesRealizadas = 0;
			// Mi nueva posicion es la del restaurant
			pedidoEnEjecucion->repartidorAsignado->posX = pedidoEnEjecucion->posRestauranteX;
			pedidoEnEjecucion->repartidorAsignado->posY = pedidoEnEjecucion->posRestauranteY;
			// El nuevo objetivo es el cliente
			pedidoEnEjecucion->objetivo = CLIENTE;
			// Genero la nueva rafaga con la distancia hacia el cliente
			pedidoEnEjecucion->instruccionesAnteriores = pedidoEnEjecucion->instruccionesTotales;
			pedidoEnEjecucion->instruccionesTotales = distanciaDeRepartidorAObjetivo(pedidoEnEjecucion->repartidorAsignado,pedidoEnEjecucion);

			log_info(logger, "[BLOCK] Ingresa pedido %i para esperar mensaje.", pedidoEnEjecucion->pedidoID);

		} else if (pedidoEnEjecucion->objetivo == CLIENTE){
			pedidoEnEjecucion->accionBlocked = ESPERANDO_EXIT;
			//ajusto las estimaciones aca tambien por si se quisiera establecer alguna metrica, pero a fines del tp
			//no seria necesario ya que el que entro por aca no vuelve a ready
			pedidoEnEjecucion->instruccionesAnteriores = pedidoEnEjecucion->instruccionesRealizadas;
			pedidoEnEjecucion->estimacionAnterior = pedidoEnEjecucion->estimacionActual;
			pedidoEnEjecucion->estimacionActual = alpha*pedidoEnEjecucion->instruccionesAnteriores
												+ (1-alpha)*pedidoEnEjecucion->estimacionAnterior;

			log_info(logger, "[BLOCK] Pedido %i llega al cliente en posicion %i-%i.", pedidoEnEjecucion->pedidoID, pedidoEnEjecucion->posClienteX, pedidoEnEjecucion->posClienteY);

		} else {
			printf("ERROR | El objetivo del pedido es invalido");
			exit(6);
		}

		retorno = 0;
	}

	return retorno;
}

void agregarABlock(pcb_pedido* elPedido){
	//printf("Intentando insertar elemento en bloqueados..\n");

	//printearValorSemaforo(mutexBlock);

	sem_wait(mutexBlock);

	list_add(colaBlock, elPedido);
	//printf("[BLOCK] Ingresa el pedido %i.\n", elPedido->pedidoID);

	sem_post(mutexBlock);

	//printf("Pude insertar elemento en bloqueados..\n");
}



pcb_pedido* obtenerSiguienteDeReady(){

	// Solo para FIFO
	// [1,2,3,4] (en ese orden) ----> Sale 1
	// Solo para SJF
	// [1,2] pcb1->instruccionesTotales1 = 5, pcb2->instruccionesTotales1 = 9 ----> Sale 1
	// Solo para HRRN
	// [1,2,3] responseRatioDePCB1 > responseRatioDePCB2 PERO responseRatioDePCB3 > responseRatioDePCB1 ----> Sale 3

	pcb_pedido* pedidoPlanificado = NULL;

	if (list_size(colaReady) > 0){

		// Aca dentro un SWITCH para los distintos algoritmos q llama a una funcion para cada uno
	  switch(algoritmo){

		// CASO FIFO
		case FIFO:
		sem_wait(mutexReady);
		pedidoPlanificado = list_remove(colaReady, 0);
		sem_post(mutexReady);
		break;

		//CASO HRRN
		case HRRN:
		pedidoPlanificado = obtenerSiguienteHRRN();
		break;

		//CASO SJF sin desalojo
		case SJFSD:
		pedidoPlanificado = obtenerSiguienteSJFSD();
		break;

	  }
	}

	// Devuelve NULL si no hay nada en ready
	// Caso contrario devuelve el que tiene mas prioridad segun el algoritmo que se este empleando
	return pedidoPlanificado;
}


pcb_pedido* obtenerSiguienteHRRN(){

	pcb_pedido* pedidoPlanificado = NULL;
	pcb_pedido* pedidoAux = NULL;
    int i;
    int elMejorResponseRatio;
    int responseRatioAux;
	int indexARemover;

	sem_wait(mutexReady);

	pedidoAux = list_get(colaReady,0);
	elMejorResponseRatio = (pedidoAux->tiempoEspera
						 + pedidoAux->estimacionActual)
			   	   	   	 / pedidoAux->estimacionActual;
	indexARemover = 0;

	//itero buscando al pedido con mayor response ratio, cada vez que encuentro uno que supere al mejor,
	//piso el valor ancla (indice del pedido en la lista ready)
	//que sera removido para ser planificado con repartidor
    for(i=1;i<list_size(colaReady);i++){
    	pedidoAux = list_get(colaReady,i);
    	responseRatioAux = (pedidoAux->tiempoEspera
    				     + pedidoAux->estimacionActual)
    				     / pedidoAux->estimacionActual;
    	//piso el valor ancla (indice del pedido en la lista ready)
    	//que sera removido para ser planificado con repartidor solo si el iterado supera al mejor
    	if(elMejorResponseRatio < responseRatioAux){
    		elMejorResponseRatio = responseRatioAux;
   		    indexARemover = i;
    	}

    }

    pedidoPlanificado = list_remove(colaReady, indexARemover);
    sem_post(mutexReady);

	return pedidoPlanificado;
}


pcb_pedido* obtenerSiguienteSJFSD(){

	pcb_pedido* pedidoPlanificado = NULL;
	pcb_pedido* pedidoAux = NULL;
    int i;
	int indexARemover;
	int shortestJob;

	sem_wait(mutexReady);
	pedidoAux = list_get(colaReady,0);
	indexARemover = 0;
	shortestJob = pedidoAux->estimacionActual;

	//itero por la lista de Ready
    for(i=1;i<list_size(colaReady);i++){
    	pedidoAux = list_get(colaReady,i);
    	//idem HRRN pero en vez de response ratio solo comparo las estimaciones de las rafagas a realizar
    	if(shortestJob > pedidoAux->estimacionActual){
    		shortestJob = pedidoAux->estimacionActual;
    		indexARemover = i;
    	}

    }
    pedidoPlanificado = list_remove(colaReady, indexARemover);
    sem_post(mutexReady);

	return pedidoPlanificado;
}


// Hilo que maneja pasar los procesos de new a ready (asignar los repartidores a los pedidos)
void hiloNew_Ready(){

	while(1){

		//printf("Esperando a que entre alguien a new..\n");
		// Espero a que me manden la señal que entro alguien nuevo
		sem_wait(contadorProcesosEnNew);

		sem_wait(mutexNew);

		// Region critica de tocar la lista new
		pcb_pedido* unPedido = queue_pop(colaNew);

		sem_post(mutexNew);

		//printf("Esperando a que haya repartidores disponibles..\n");

		//printearValorSemaforo(contadorRepartidoresDisp, "ContadorRepartidoresDisp");
		sem_wait(contadorRepartidoresDisp);

		//printf("Hay repartidores disponibles..\n");
		asignarRepartidorAPedido(unPedido);
	}
}

// 1 si esta desocupado | 0 si esta ocupado
int estaDesocupado(repartidor* unRepartidor){
	return !unRepartidor->asignado;
}


void asignarRepartidorAPedido(pcb_pedido* unPedido){
	// Filtro la lista y solo dejo los repartidores que no estan ocupados
	t_list* repartidoresDisponibles = list_filter(repartidores, (void*)estaDesocupado);

	printf("Hay %i repartidores disponibles\n", list_size(repartidoresDisponibles));

	int mejorDistancia;
	repartidor* mejorRepartidor;
	unPedido->objetivo = RESTAURANTE;

	int i;
	// Recorro la lista de disponibles
	for (i = 0; i < list_size(repartidoresDisponibles); i++){
		// Calculo la distancia hasta el objetivo, si es menor a la actual piso hasta encontrar el menor

		// Obtengo repartidor de la lista de disponibles
		repartidor* miRepartidor = list_get(repartidoresDisponibles, i);

		// Calculo la distancia del repartidor hasta el objetivo
		int distanciaActual = distanciaDeRepartidorAObjetivo(miRepartidor, unPedido);

		//printf("La distancia del repartidor numero %i es %i\n", miRepartidor->numeroRepartidor, distanciaActual);

		// Si la distancia es menor a la mejor entonces piso (para encontrar el menor)
		if (i == 0 || distanciaActual < mejorDistancia){
			mejorDistancia = distanciaActual;
			mejorRepartidor = miRepartidor;
		}

	}

	log_info(logger, "[NEW] Se asigna el repartidor: %i al pedido: %i. Posicion repartidor: %i-%i. Posicion Restaurant: %i-%i.",
			mejorRepartidor->numeroRepartidor, unPedido->pedidoID, mejorRepartidor->posX, mejorRepartidor->posY, unPedido->posRestauranteX, unPedido->posRestauranteY);

	// Asigno el repartidor al pedido, ahora está ocupado
	mejorRepartidor->asignado = 1;
	unPedido->repartidorAsignado = mejorRepartidor;
	unPedido->instruccionesTotales = mejorDistancia;
	unPedido->instruccionesRealizadas = 0;
	unPedido->instruccionesAnteriores = 0;
	unPedido->estimacionAnterior = 0;
	unPedido->estimacionActual = estimacion_inicial;
	unPedido->accionBlocked = NO_BLOCK;
	unPedido->tiempoEspera = 0;
	unPedido->proximoEstado = READY;

	log_info(logger,"[READY] Ingresa pedido %i desde new.", unPedido->pedidoID);
	agregarAReady(unPedido);

	// Debug
}

uint32_t valor_para_switch_case_planificacion(char* algoritmo) {

	uint32_t switcher;

	//FIFO
	if (strcmp(algoritmo,"FIFO") == 0)
	{
		switcher = 1;
	}

	//HRRN
	if (strcmp(algoritmo,"HRRN") == 0)
	{
		switcher = 2;
	}

	//SFJ SIN DESALOJO
	if (strcmp(algoritmo,"SJF-SD") == 0)
	{
		switcher = 3;
	}
	return switcher;
}

// Calculo la distancia entre dos puntos
int distanciaDeRepartidorAObjetivo(repartidor* unRepartidor, pcb_pedido* elPedido){

	int posObjetivoX;
	int posObjetivoY;

	if(elPedido->objetivo == RESTAURANTE){
		posObjetivoX = elPedido->posRestauranteX;
		posObjetivoY = elPedido->posRestauranteY;
	} else {
		posObjetivoX = elPedido->posClienteX;
		posObjetivoY = elPedido->posClienteY;
	}

	int posXRepartidor = unRepartidor->posX;
	int posYRepartidor = unRepartidor->posY;


	int distanciaX = modulo(posXRepartidor - posObjetivoX);
	int distanciaY = modulo(posYRepartidor - posObjetivoY);

	return distanciaX + distanciaY;
}

// Valor absoluto del num
int modulo(int num){
	if (num < 0)
		return -num;
	else
		return num;
}

void printearValorSemaforo(sem_t* unSemaforo, char* nombre){
	int* valorSemaforo = malloc(sizeof(int));
	sem_getvalue(unSemaforo, valorSemaforo);
	printf("Valor semaforo %s es: %i\n", nombre, *valorSemaforo);
}

/*
 * Al generar el pcb_pedido deben estar fijados estos valores:
 * posObjetivoX, posObjetivoY = Posicion restaurant
 * pedidoID,
 *
 */

// Agrega el nuevo pedido a la cola NEW
void agregarANew(pcb_pedido* unPedido)
{
	sem_wait(mutexNew);

	queue_push(colaNew, unPedido);

	log_info(logger, "[NEW] Entra el nuevo pedido %i.", unPedido->pedidoID);

	sem_post(mutexNew);

	// Le envio la señal al hilo new-ready
	sem_post(contadorProcesosEnNew);
}

void agregarAReady(pcb_pedido* unPedido){
	sem_wait(mutexReady);

	list_add(colaReady, unPedido);

	sem_post(mutexReady);
}

void agregarAExit(pcb_pedido* unPedido){
	log_info(logger, "[EXIT] Pedido %i esta en exit por haber terminado.", unPedido->pedidoID);

	eliminarPedidoListo(unPedido->pedidoID);
	unPedido->repartidorAsignado->asignado = 0;
	free(unPedido);

	sem_post(contadorRepartidoresDisp);
}

// Inicializacion de semaforos necesarios
void iniciarSemaforosPlanificacion(){
	contadorProcesosEnNew = malloc(sizeof(sem_t));
	sem_init(contadorProcesosEnNew, 0, 0);

	// OLD
	//contadorProcesosEnReady = malloc(sizeof(sem_t));
	//sem_init(contadorProcesosEnReady, 0, 0);

	mutexNew = malloc(sizeof(sem_t));
	sem_init(mutexNew, 0, 1);

	mutexReady = malloc(sizeof(sem_t));
	sem_init(mutexReady, 0, 1);

	mutexBlock = malloc(sizeof(sem_t));
	sem_init(mutexBlock, 0, 1);

	mutexPedidosListos = malloc(sizeof(sem_t));
	sem_init(mutexPedidosListos, 0, 1);

	contadorRepartidoresDisp = malloc(sizeof(sem_t));
	sem_init(contadorRepartidoresDisp, 0, 0);
}

// Cargo la lista repartidores con la lista de todos los repartidores de la config
void leerPlanificacionConfig(t_config* config){

	repartidores = list_create();

	GRADO_MULTIPROCE = config_get_int_value(config, "GRADO_DE_MULTIPROCESAMIENTO");
	RETARDO_CICLO_CPU = config_get_int_value(config, "RETARDO_CICLO_CPU");
	alpha = config_get_double_value(config,"ALPHA");
	estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");

	char** posicionesRepartidores = config_get_array_value(config, "REPARTIDORES");
	char** frecuenciasDescansos = config_get_array_value(config, "FRECUENCIA_DE_DESCANSO");
	char** tiemposDescanso = config_get_array_value(config, "TIEMPO_DE_DESCANSO");

	int cantidadRepartidores = cantidadDeElementosEnArray(posicionesRepartidores);

	int i;
	for (i = 0; i< cantidadRepartidores; i++){
		repartidor* miRepartidor = malloc(sizeof(repartidor));

		char** posiciones = string_split(posicionesRepartidores[i], "|");

		miRepartidor->numeroRepartidor = i + 1;
		miRepartidor->posX = atoi(posiciones[0]);
		miRepartidor->posY = atoi(posiciones[1]);
		miRepartidor->frecuenciaDescanso = atoi(frecuenciasDescansos[i]);
		miRepartidor->tiempoDescanso = atoi(tiemposDescanso[i]);
		miRepartidor->asignado = 0;
		miRepartidor->tiempoDescansado = 0;
		miRepartidor->cansado = 0;
		miRepartidor->instruccionesRealizadas = 0;

		freeDeArray(posiciones);

		list_add(repartidores, miRepartidor);

		sem_post(contadorRepartidoresDisp);

	}

	freeDeArray(posicionesRepartidores);
	freeDeArray(frecuenciasDescansos);
	freeDeArray(tiemposDescanso);
}

void iniciarSemaforosCiclos(){

	habilitarCicloBlockReady= malloc(sizeof(sem_t));
	sem_init(habilitarCicloBlockReady, 0, 0);
	finalizarCicloBlockReady= malloc(sizeof(sem_t));
	sem_init(finalizarCicloBlockReady, 0, 0);

	listaSemHabilitarCicloExec = list_create();
	listaSemFinalizarCicloExec = list_create();

	int i;
	for(i=0; i<GRADO_MULTIPROCE; i++){
		sem_t* semaforoHabilitarCiclo = malloc(sizeof(sem_t));
		sem_init(semaforoHabilitarCiclo, 0, 0);
		sem_t* semaforoFinalizarCiclo = malloc(sizeof(sem_t));
		sem_init(semaforoFinalizarCiclo, 0, 0);
		list_add(listaSemHabilitarCicloExec, semaforoHabilitarCiclo);
		list_add(listaSemFinalizarCicloExec, semaforoFinalizarCiclo);
	}
}


void hiloCiclosMaestro(){

    int i;
    int numCiclo = 1;

    while(1){

		log_trace(logger, "[HCM] Habilitando ciclo: %i\n", numCiclo);

		for(i = 0; i < GRADO_MULTIPROCE; i++){
			signalSemaforoHabilitarCicloExec(i);
		}
		sem_post(habilitarCicloBlockReady);

		sleep(RETARDO_CICLO_CPU);

		log_trace(logger, "[HCM] Finalizando ciclo: %i", numCiclo);

		for(i = 0; i < GRADO_MULTIPROCE; i++){
			waitSemaforoFinalizarCicloExec(i);
		}
		sem_wait(finalizarCicloBlockReady);

		numCiclo++;
    }
}

void waitSemaforoHabilitarCicloExec(uint32_t indice){
	sem_t* semaforoObjetivo = list_get(listaSemHabilitarCicloExec, indice);
	sem_wait(semaforoObjetivo);
}

void signalSemaforoHabilitarCicloExec(uint32_t indice){
	sem_t* semaforoObjetivo = list_get(listaSemHabilitarCicloExec, indice);
	sem_post(semaforoObjetivo);
}

void waitSemaforoFinalizarCicloExec(uint32_t indice){
	sem_t* semaforoObjetivo = list_get(listaSemFinalizarCicloExec, indice);
	sem_wait(semaforoObjetivo);
}

void signalSemaforoFinalizarCicloExec(uint32_t indice){
	sem_t* semaforoObjetivo = list_get(listaSemFinalizarCicloExec, indice);
	sem_post(semaforoObjetivo);
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

int buscar_pedido_por_id(uint32_t id_pedido){
	perfil_cliente* cliente;
	for(int i = 0; i < listaPedidos->elements_count; i++){
		cliente = list_get(listaPedidos,i);// conseguis el perfil del cliente

		if(cliente->id_global == id_pedido){
			return i;
		}
	}
	return -2;
}

