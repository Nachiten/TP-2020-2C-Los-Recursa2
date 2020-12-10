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
	//pthread_join(hiloNewReady, NULL);
}

void guardarPedidoListo(uint32_t idGlobalDelPedidoQueFinalizo){
	int* pedidoAGuardar = malloc(sizeof(int));

	*pedidoAGuardar = idGlobalDelPedidoQueFinalizo;

	// Agrego el pedido finalizado a la lista.
	sem_wait(mutexPedidosListos);
	list_add(pedidosListos, pedidoAGuardar);
    sem_post(mutexPedidosListos);
	sem_wait(semLog);
	log_trace(logger, "[APP-GuardarPedido] Se guarda como listo el pedido de IDGlobal: <%d>.",
			idGlobalDelPedidoQueFinalizo);
	sem_post(semLog);
}

void eliminarPedidoListo(int idGlobalDelPedidoQueFinalizo){
	sem_wait(mutexPedidosListos);
	int i;
	// Recorro la lista de pedidos listos a ver si encuentro el que necesito
	for (i = 0; i< list_size(pedidosListos); i++){
		int* idGlobalPedidoCandidato = list_get(pedidosListos, i);

		if (*idGlobalPedidoCandidato == idGlobalDelPedidoQueFinalizo){
			int* pedidoAEliminar = list_remove(pedidosListos, i);
			free(pedidoAEliminar);
			sem_wait(semLog);
			log_trace(logger, "[APP-PedidosListos] Eliminado pedido de IDGlobal: <%d> de la lista pedidos listos.", idGlobalDelPedidoQueFinalizo);
		    sem_post(semLog);
		}

	}
	sem_post(mutexPedidosListos);
}


int checkearPedidoListo(int idPedidoGlobalACheckear){

	int retorno = 0;

	sem_wait(mutexPedidosListos);
	int i;
	// Recorro la lista de pedidos listos a ver si encuentro el que necesito
	for (i = 0; i< list_size(pedidosListos); i++){
		int* pedidoCandidato = list_get(pedidosListos, i);

		if (*pedidoCandidato == idPedidoGlobalACheckear){
			retorno = 1;
			break;
		}
	}
	sem_post(mutexPedidosListos);

	return retorno;
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
			sem_wait(semLog);
			log_trace(logger, "[BLOCK] No hay pedidos en block.");
			sem_post(semLog);

		int indicePedido;
		// Escaneo todos los elementos en block para sumar 1 ciclo de descanso a cada proceso
		for (indicePedido = 0; indicePedido < elementosEnBlock; indicePedido++){

			sem_wait(mutexBlock);
			pcb_pedido* pedidoActual = list_get(colaBlock, indicePedido);
			sem_post(mutexBlock);

			if (pedidoActual == NULL){
				break;
			}

			repartidor* repartidorActual = pedidoActual->repartidorAsignado;

			switch(pedidoActual->accionBlocked){
				case DESCANSANDO:

					repartidorActual->tiempoDescansado++;
                    sem_wait(semLog);
					log_trace(logger, "[BLOCK] El repartidor del pedido %d ya descanso %d ciclos, le faltan %d."
							, pedidoActual->pedidoIDGlobal, repartidorActual->tiempoDescansado
							, repartidorActual->tiempoDescanso-repartidorActual->tiempoDescansado);
                    sem_post(semLog);
					// Ya descanso lo que tenia que descansar
					if (repartidorActual->tiempoDescansado == repartidorActual->tiempoDescanso){
						sem_wait(semLog);
						log_trace(logger, "[BLOCK] El repartidor del pedido %d ya descanso todos los %d ciclos que necesitaba."
								, pedidoActual->pedidoIDGlobal, repartidorActual->tiempoDescansado);
						sem_post(semLog);
						if (pedidoActual->proximoEstado == READY){
							// Cuando termina de descansar y tiene que continuar rafagas

							pedidoActual->repartidorAsignado->tiempoDescansado = 0;
							pedidoActual->repartidorAsignado->cansado = 0;
							sem_wait(semLog);
							log_info(logger, "[READY] Ingresa pedido %d por terminar de descansar.", pedidoActual->pedidoIDGlobal);
							sem_post(semLog);
							moverPedidoDeBlockAReady(indicePedido);
							indicePedido--;

						} else if (pedidoActual->proximoEstado == EXIT){

							// Termina de descansar y ya esta terminado
							moverPedidoDeBlockAExit(indicePedido);
							indicePedido--;

						} else {
							sem_wait(semLog);
							log_error(logger, "BLOCK | ERROR: El proximo estado del pedido %d tiene un valor invalido.", pedidoActual->pedidoIDGlobal);
							sem_post(semLog);
							exit(7);
						}
					}
					break;

				case ESPERANDO_MSG:

					// Ya esta listo el pedido
					if (checkearPedidoListo(pedidoActual->pedidoIDGlobal)){
						sem_wait(semLog);
						log_trace(logger, "[BLOCK] El pedido de IDGlobal: %d ya esta listo.", pedidoActual->pedidoIDGlobal);
                        sem_post(semLog);

						// Debe descansar antes de volver a ready
						if (pedidoActual->repartidorAsignado->cansado){
							pedidoActual->accionBlocked = DESCANSANDO;
							pedidoActual->proximoEstado = READY;

						// No esta cansado, va a ready
						} else {
							sem_wait(semLog);
							log_info(logger, "[READY] Ingresa pedido %d por ya estar listo el mensaje que esperaba.", pedidoActual->pedidoIDGlobal);
							sem_post(semLog);
							moverPedidoDeBlockAReady(indicePedido);
							indicePedido--;
						}


					// No esta listo
					} else {
						sem_wait(semLog);
						log_trace(logger, "[BLOCK] El pedido %d todavia no esta listo.", pedidoActual->pedidoIDGlobal);
						sem_post(semLog);
					}

					break;

				case ESPERANDO_EXIT:

					// Se entrega el pedido
					sem_wait(semLog);
					log_info(logger, "[BLOCK] Repartidor %d entrega pedido %d en la posicion del cliente.", pedidoActual->repartidorAsignado->numeroRepartidor, pedidoActual->pedidoIDGlobal);
					sem_post(semLog);
					//pedido_entregado(pedidoActual); esto ya no seria necesario broder

					// Debe descansar antes de volver a ready
					if (pedidoActual->repartidorAsignado->cansado){
						pedidoActual->accionBlocked = DESCANSANDO;
						pedidoActual->proximoEstado = EXIT;

					// No esta cansado, va a exit
					} else {
						moverPedidoDeBlockAExit(indicePedido);
						indicePedido--;
					}
					break;

				default:
					sem_wait(semLog);
					log_error(logger, "[BLOCK | ERROR] El pedido %d esta en block pero no se le asigno el por que esta bloqueado.", pedidoActual->pedidoIDGlobal);
					sem_post(semLog);
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
            sem_wait(semLog);
			log_info(logger, "[EXEC-%d] Ingresa pedido %d. Instruciones restantes: %d",
					numHiloExec, pedidoAEjecutar->pedidoIDGlobal, pedidoAEjecutar->instruccionesTotales);
            sem_post(semLog);
			int cantidadCiclos = 1;

			// Calculo si tiene que desalojar o no y por que razon
			while (sigoEjecutando(pedidoAEjecutar)){
				waitSemaforoHabilitarCicloExec(numHiloExec);
				sem_wait(semLog);
				log_trace(logger, "[EXEC-%d] Ejecuto ciclo numero: %d.", numHiloExec, cantidadCiclos);
                sem_post(semLog);
				pedidoAEjecutar->instruccionesRealizadas++;
				pedidoAEjecutar->repartidorAsignado->instruccionesRealizadas++;
                sem_wait(semLog);
				log_trace(logger, "[EXEC-%d] Instrucciones restantes: %d.", numHiloExec, pedidoAEjecutar->instruccionesTotales - pedidoAEjecutar->instruccionesRealizadas);
                sem_post(semLog);
				signalSemaforoFinalizarCicloExec(numHiloExec);

				cantidadCiclos++;
			}

			agregarABlock(pedidoAEjecutar);

		} else {
			waitSemaforoHabilitarCicloExec(numHiloExec);
			sem_wait(semLog);
			log_trace(logger, "[EXEC-%d] Desperdicio un ciclo porque no hay nadie en ready.", numHiloExec);
			sem_post(semLog);
			signalSemaforoFinalizarCicloExec(numHiloExec);
		}

	}
}

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
		sem_wait(semLog);
		log_info(logger, "[BLOCK] Ingresa pedido %i por estar cansado.", pedidoEnEjecucion->pedidoIDGlobal);
		sem_post(semLog);
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
			sem_wait(semLog);
			log_info(logger, "[BLOCK] Ingresa pedido %i para esperar mensaje.", pedidoEnEjecucion->pedidoID);
			sem_post(semLog);
		} else if (pedidoEnEjecucion->objetivo == CLIENTE){
			pedidoEnEjecucion->accionBlocked = ESPERANDO_EXIT;
			//ajusto las estimaciones aca tambien por si se quisiera establecer alguna metrica, pero a fines del tp
			//no seria necesario ya que el que entro por aca no vuelve a ready
			pedidoEnEjecucion->instruccionesAnteriores = pedidoEnEjecucion->instruccionesRealizadas;
			pedidoEnEjecucion->estimacionAnterior = pedidoEnEjecucion->estimacionActual;
			pedidoEnEjecucion->estimacionActual = alpha*pedidoEnEjecucion->instruccionesAnteriores
												+ (1-alpha)*pedidoEnEjecucion->estimacionAnterior;
			sem_wait(semLog);
			log_info(logger, "[BLOCK] Pedido %d llega al cliente en posicion [%d-%d].", pedidoEnEjecucion->pedidoIDGlobal, pedidoEnEjecucion->posClienteX, pedidoEnEjecucion->posClienteY);
			sem_post(semLog);
		} else {
			sem_wait(semLog);
			log_error(logger, "[ERROR | EXEC] El objetivo del pedido %d es invalido.", pedidoEnEjecucion->pedidoIDGlobal);
			sem_post(semLog);
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
    sem_wait(semLog);
	log_info(logger, "[NEW] Se asigna el repartidor: %d al pedido: %d. Posicion repartidor: [%d-%d]. Posicion Restaurant: [%d-%d].",
			mejorRepartidor->numeroRepartidor, unPedido->pedidoIDGlobal, mejorRepartidor->posX, mejorRepartidor->posY, unPedido->posRestauranteX, unPedido->posRestauranteY);
    sem_post(semLog);
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

	sem_wait(semLog);
	log_info(logger,"[READY] Ingresa pedido %d desde new.", unPedido->pedidoIDGlobal);
	sem_post(semLog);
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

	sem_wait(semLog);
	log_info(logger, "[NEW] Entra el nuevo pedido %d a la cola.", unPedido->pedidoIDGlobal);
	sem_post(semLog);

	sem_post(mutexNew);

	// Le envio la señal al hilo new-ready
	sem_post(contadorProcesosEnNew);
}

void agregarAReady(pcb_pedido* unPedido){
	sem_wait(mutexReady);

	list_add(colaReady, unPedido);

	sem_post(mutexReady);
}

void agregarAExit(pcb_pedido* elPCBQueFinalizo){
	guardar_pedido* notificacionFinalizarPedido;
	respuesta_ok_error* respuestaNotificacion;
	perfil_pedido* elPedidoQueFinalizo;
	int indicePedidoQueFinalizo, recibidos, sizeAAllocar, recibidosSize = 0;
	codigo_operacion cod_op;
	int32_t nuevoSocketComanda;

	sem_wait(semLog);
	log_info(logger, "[EXIT] Pedido de IDGlobal: <%d> e IDResto: <%d> esta en exit por haber llegado a la posicion del cliente."
			, elPCBQueFinalizo->pedidoIDGlobal, elPCBQueFinalizo->pedidoID);
	sem_post(semLog);

	eliminarPedidoListo(elPCBQueFinalizo->pedidoIDGlobal);
	elPCBQueFinalizo->repartidorAsignado->asignado = 0;

	//antes de hacerle free al pcb y al perfil_pedido, tengo que hacer DOS COSAS (maybe 3):
	//-> Informar a comanda un FINALIZAR_PEDIDO del pedido que acaba de llegar a exit.
	//-> Informar al cliente solicitante un FINALIZAR_PEDIDO porque el pedido llego a su posicion

	nuevoSocketComanda = establecer_conexion(ip_commanda,puerto_commanda);
	if(nuevoSocketComanda < 0){
		sem_wait(semLog);
		log_info(logger, "Comanda esta muerta, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

	sem_wait(mutexListaPedidos);
	indicePedidoQueFinalizo = buscarPedidoPorIDGlobal(elPCBQueFinalizo->pedidoIDGlobal);
	elPedidoQueFinalizo = list_remove(listaPedidos, indicePedidoQueFinalizo);
	sem_post(mutexListaPedidos);

    notificacionFinalizarPedido = malloc(sizeof(guardar_pedido));
    notificacionFinalizarPedido->idPedido = elPedidoQueFinalizo->id_pedido_resto;
    notificacionFinalizarPedido->largoNombreRestaurante = strlen(elPedidoQueFinalizo->nombreRestaurante);
    notificacionFinalizarPedido->nombreRestaurante = malloc(strlen(elPedidoQueFinalizo->nombreRestaurante)+1);
    strcpy(notificacionFinalizarPedido->nombreRestaurante, elPedidoQueFinalizo->nombreRestaurante);

    mandar_mensaje(notificacionFinalizarPedido, FINALIZAR_PEDIDO, nuevoSocketComanda);
    respuestaNotificacion = malloc(sizeof(respuesta_ok_error));

    recibidos = recv(nuevoSocketComanda, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
	if(recibidos >= 1){
		recibidosSize = recv(nuevoSocketComanda, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
		bytesRecibidos(recibidosSize);
		recibir_mensaje(respuestaNotificacion, RESPUESTA_FINALIZAR_PEDIDO, nuevoSocketComanda);
		if(respuestaNotificacion->respuesta == 0){
			//comanda fallo en finalizarlo, lo logueo por las moscas
			sem_wait(semLog);
			log_error(logger, "[EXIT] Comanda fracaso en finalizar el pedido.");
			sem_post(semLog);
		}
	} else {
		sem_wait(semLog);
		log_error(logger, "[EXIT] Comanda se murio en el proceso de responder a un Finalizar_pedido.");
		sem_post(semLog);
	}

    //uso la misma estructura de antes, pero con IDGlobal, para dirigirme al cliente
	notificacionFinalizarPedido->idPedido = elPedidoQueFinalizo->id_pedido_global;
	mandar_mensaje(notificacionFinalizarPedido, FINALIZAR_PEDIDO, elPedidoQueFinalizo->socket_cliente);
	recibidos = recv(elPedidoQueFinalizo->socket_cliente, &cod_op, sizeof(codigo_operacion), MSG_WAITALL);
    if(recibidos >= 1){
		recibidosSize = recv(elPedidoQueFinalizo->socket_cliente, &sizeAAllocar, sizeof(sizeAAllocar), MSG_WAITALL); //saca el tamaño de lo que sigue en el buffer
		bytesRecibidos(recibidosSize);
		recibir_mensaje(respuestaNotificacion, RESPUESTA_FINALIZAR_PEDIDO, elPedidoQueFinalizo->socket_cliente);
		if(respuestaNotificacion->respuesta == 1){
			sem_wait(semLog);
			log_trace(logger, "[EXIT] El cliente fue notificado satisfactoriamente de la finalizacion del pedido.");
			sem_post(semLog);
		}
	} else {
		sem_wait(semLog);
		log_error(logger, "[EXIT] El cliente se murio en el proceso de responder a un Finalizar_pedido.");
		sem_post(semLog);
	}

    //libero tutti
    free(notificacionFinalizarPedido->nombreRestaurante);
    free(notificacionFinalizarPedido);
    free(respuestaNotificacion);
    free(elPedidoQueFinalizo->nombreRestaurante);
    free(elPedidoQueFinalizo->idCliente);
    free(elPedidoQueFinalizo);
	free(elPCBQueFinalizo);
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

/*
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
*/
