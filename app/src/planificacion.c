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

	pcb_pedido* unPedido = malloc(sizeof(pcb_pedido));
	unPedido->posRestauranteX = 7;
	unPedido->posRestauranteY = 4;
	unPedido->posClienteX = 3;
	unPedido->posClienteY = 5;
	unPedido->pedidoID = 1;

	agregarANew(unPedido);

	sleep(3);

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

	sleep(25);

	guardarPedidoListo(1);

	pcb_pedido* unPedido2 = malloc(sizeof(pcb_pedido));
	unPedido2->posRestauranteX = 3;
	unPedido2->posRestauranteY = 5;
	unPedido2->posClienteX = 3;
	unPedido2->posClienteY = 5;
	unPedido2->pedidoID = 2;

	agregarANew(unPedido2);

	pcb_pedido* unPedido3 = malloc(sizeof(pcb_pedido));
	unPedido3->posRestauranteX = 7;
	unPedido3->posRestauranteY = 8;
	unPedido3->posClienteX = 3;
	unPedido3->posClienteY = 5;
	unPedido3->pedidoID = 3;

	agregarANew(unPedido3);

	sleep(20);

	guardarPedidoListo(2);

	sleep(20);

	guardarPedidoListo(3);

	pcb_pedido* unPedido4 = malloc(sizeof(pcb_pedido));
	unPedido4->posRestauranteX = 10;
	unPedido4->posRestauranteY = 11;
	unPedido4->posClienteX = 2;
	unPedido4->posClienteY = 3;
	unPedido4->pedidoID = 4;

	pcb_pedido* unPedido5 = malloc(sizeof(pcb_pedido));
	unPedido5->posRestauranteX = 12;
	unPedido5->posRestauranteY = 9;
	unPedido5->posClienteX = 1;
	unPedido5->posClienteY = 2;
	unPedido5->pedidoID = 5;

	guardarPedidoListo(4);
	guardarPedidoListo(5);

	pthread_join(hiloNewReady, NULL);
}

void guardarPedidoListo(int idPedido){
	int* pedidoAGuardar = malloc(sizeof(int));

	*pedidoAGuardar = idPedido;

	sem_wait(mutexPedidosListos);
	// Agrego un nuevo pedido listo a la lista
	list_add(pedidosListos, pedidoAGuardar);
	sem_post(mutexPedidosListos);

	printf("[GuardarPedido] Se guarda como listo el pedido %i\n", idPedido);
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
			printf("[Pedidos] Eliminado pedido %i de pedidos listos.\n", idPedido);
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

void hiloBlock_Ready(){

	while(1){

		sem_wait(habilitarCicloBlockReady);

		// TODO
		// En caso de HRRN se debe escanear cola READY	 para sumar 1 de tiempo de espera

		if(algoritmo == HRRN){
			int i;
			int elementosEnReady = list_size(colaReady);
			sem_wait(mutexReady);
			//itero por cada elemento presente en READY
			for (i = 0; i < elementosEnReady; i++){

			pcb_pedido* pedidoActual = list_get(colaReady, i);
			pedidoActual->tiempoEspera += 1;

			}
			sem_post(mutexReady);
		}

		int elementosEnBlock = list_size(colaBlock);

		if(elementosEnBlock == 0)
			printf("[HiloBlock] No hay pedidos en block.\n");

		int i;
		// Escaneo todos los elementos en block para sumar 1 ciclo de descanso a cada proceso
		for (i = 0; i < elementosEnBlock; i++){

			sem_wait(mutexBlock);
			pcb_pedido* pedidoActual = list_get(colaBlock, i);
			sem_post(mutexBlock);

			repartidor* repartidorActual = pedidoActual->repartidorAsignado;

			if (pedidoActual->estadoBlocked == DESCANSANDO){
				repartidorActual->tiempoDescansado++;

				printf("[HiloBlock] El pedido %i ya descanso %i ciclos.\n", pedidoActual->pedidoID, repartidorActual->tiempoDescansado);

				// Ya descanso lo que tenia que descansar
				if (repartidorActual->tiempoDescansado == repartidorActual->tiempoDescanso){
					pcb_pedido* pedidoAReady = list_remove(colaBlock, i);

					printf("[HiloBlock] El pedido %i ya descanso todos los %i ciclos que necesitaba.\n", pedidoActual->pedidoID, repartidorActual->tiempoDescansado);

					pedidoAReady->estadoBlocked = NO;
					pedidoAReady->repartidorAsignado->tiempoDescansado = 0;

					// Se va a ready porque termino de descansar
					printf("[READY] Ingresa pedido %i por terminar de descansar.\n", pedidoAReady->pedidoID);
					agregarAReady(pedidoAReady);
				}

			} else if (pedidoActual->estadoBlocked == ESPERANDO_MSG){
				// Se debe corroborar si el mensaje ya llego, caso afirmativo mover a ready

				// Ya esta listo el pedido
				if (checkearPedidoListo(pedidoActual->pedidoID)){
					printf("[HiloBlock] El pedido %i ya esta listo.\n", pedidoActual->pedidoID);
					pcb_pedido* pedidoAReady = list_remove(colaBlock, i);
					pedidoAReady->estadoBlocked = NO;

					log_info(logger, "[READY] Ingresa pedido %i por ya estar listo.", pedidoAReady->pedidoID);
					agregarAReady(pedidoActual);
				} else {
					printf("[HiloBlock] El pedido %i todavia no esta listo.\n", pedidoActual->pedidoID);
				}



			} else if (pedidoActual->estadoBlocked == NO){
				printf("[HiloBlock | ERROR] El pedido %i esta en blocked pero no se le asigno por que esta bloqueado.\n", pedidoActual->pedidoID);
				exit(5);
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

			int desalojoCode;
			int cantidadCiclos = 1;

			// Calculo si tiene que desalojar o no y por que razon
			while ( (desalojoCode = codigoDesalojo(pedidoAEjecutar) ) == 0){
				waitSemaforoHabilitarCicloExec(numHiloExec);
				printf("[EXEC-%i] Ejecuto ciclo numero: %i\n", numHiloExec, cantidadCiclos);

				pedidoAEjecutar->instruccionesRealizadas++;

				printf("[EXEC-%i] Instrucciones restantes: %i\n", numHiloExec, pedidoAEjecutar->instruccionesTotales - pedidoAEjecutar->instruccionesRealizadas);

				signalSemaforoFinalizarCicloExec(numHiloExec);

				cantidadCiclos++;
			}

			// Esta cansado
			if (desalojoCode == 1){
				printf("[EXEC-%i] Estoy cansado.\n", numHiloExec);

				// Ahora las instrucciones totales reflejan las que faltan
				pedidoAEjecutar->instruccionesTotales -= pedidoAEjecutar->instruccionesRealizadas;
				pedidoAEjecutar->instruccionesRealizadas = 0;
				pedidoAEjecutar->estadoBlocked = DESCANSANDO;

				log_info(logger, "[BLOCK] Ingresa pedido %i por estar cansado.", pedidoAEjecutar->pedidoID);
				agregarABlock(pedidoAEjecutar);
			// Termino rafaga
			} else if (desalojoCode == 2){
				if(pedidoAEjecutar->objetivo == RESTAURANTE){
					log_info(logger, "[EXEC-%i] Pedido %i llega al restaurant en posicion %i-%i.", numHiloExec, pedidoAEjecutar->pedidoID, pedidoAEjecutar->posRestauranteX, pedidoAEjecutar->posRestauranteY);

					pedidoAEjecutar->instruccionesRealizadas = 0;
					pedidoAEjecutar->repartidorAsignado->posX = pedidoAEjecutar->posRestauranteX;
					pedidoAEjecutar->repartidorAsignado->posY = pedidoAEjecutar->posRestauranteY;
					pedidoAEjecutar->estadoBlocked = ESPERANDO_MSG;
					pedidoAEjecutar->objetivo = CLIENTE;
					pedidoAEjecutar->instruccionesTotales = distanciaDeRepartidorAObjetivo(pedidoAEjecutar->repartidorAsignado,pedidoAEjecutar);

					log_info(logger, "[BLOCK] Ingresa pedido %i para esperar mensaje.", pedidoAEjecutar->pedidoID);
					agregarABlock(pedidoAEjecutar);
				} else if (pedidoAEjecutar->objetivo == CLIENTE){
					log_info(logger, "[EXEC-%i] Pedido %i llega al cliente en posicion %i-%i.", numHiloExec, pedidoAEjecutar->pedidoID, pedidoAEjecutar->posClienteX, pedidoAEjecutar->posClienteY);
					log_info(logger, "[EXEC-%i] Repartidor %i entrega pedido %i.", numHiloExec, pedidoAEjecutar->repartidorAsignado->numeroRepartidor, pedidoAEjecutar->pedidoID);

					// TODO | Se deben mandar todos los mensajes aclarados en consigna

					agregarAExit(pedidoAEjecutar);
					//ToDo se manda a exit y se liberan todos los recursos del pedido por puntero salvo el repartidor (pertenecen a una lista que no se puede tocar)
				} else {
					printf("[EXEC-%i] | [ERROR] El objetivo del pedido es invalido\n", numHiloExec);
					exit(6);
				}


			} else {
				printf("[EXEC-%i | ERROR] El desalojo code tiene un valor invalido.\n", numHiloExec);
				exit(6);
			}



		} else {
			waitSemaforoHabilitarCicloExec(numHiloExec);
			printf("[EXEC-%i] Desperdicio un ciclo porque no hay nadie en ready.\n", numHiloExec);
			signalSemaforoFinalizarCicloExec(numHiloExec);
		}

	}
}

// 0: No se desaloja, 1: Se desaloja por estar cansado, 2: Se desaloja porque termino la rafaga
int codigoDesalojo(pcb_pedido* unPedido){

	// Si termino de reliazar toda la rafaga
	if (unPedido->instruccionesRealizadas == unPedido->instruccionesTotales){
		return 2;
	}

	// Si se canso ya y debe ir a blocked
	if (unPedido->instruccionesRealizadas == unPedido->repartidorAsignado->frecuenciaDescanso){
		return 1;
	}

	// Ninguna (sigue ejecutando)
	return 0;
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
						 + pedidoAux->instruccionesTotales)
			   	   	   	 / pedidoAux->instruccionesTotales;
	indexARemover = 0;

	//itero buscando al pedido con mayor response ratio, cada vez que encuentro uno que supere al mejor,
	//piso el valor ancla (indice del pedido en la lista ready)
	//que sera removido para ser planificado con repartidor
    for(i=1;i<list_size(colaReady);i++){
    	pedidoAux = list_get(colaReady,i);
    	responseRatioAux = (pedidoAux->tiempoEspera
    				     + pedidoAux->instruccionesTotales)
    				     / pedidoAux->instruccionesTotales;
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
	shortestJob = pedidoAux->instruccionesTotales;

	//itero por la lista de Ready
    for(i=1;i<list_size(colaReady);i++){
    	pedidoAux = list_get(colaReady,i);
    	//idem HRRN pero en vez de response ratio solo comparo las rafagas a realizar
    	if(shortestJob > pedidoAux->instruccionesTotales){
    		shortestJob = pedidoAux->instruccionesTotales;
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
	unPedido->estadoBlocked = NO;
	unPedido->tiempoEspera = 0;

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
	//printf("[NEW] Entra el nuevo pedido %i.\n", unPedido->pedidoID);

	sem_post(mutexNew);

	// Le envio la señal al hilo new-ready
	sem_post(contadorProcesosEnNew);
}

void agregarAReady(pcb_pedido* unPedido){
	sem_wait(mutexReady);

	list_add(colaReady, unPedido);

	sem_post(mutexReady);

	//printf("[READY] Ingresa pedido %i.\n", unPedido->pedidoID);
}

void agregarAExit(pcb_pedido* unPedido){
	log_info(logger, "[EXIT] Pedido %i esta en exit por haber terminado.", unPedido->pedidoID);

	eliminarPedidoListo(unPedido->pedidoID);
	unPedido->repartidorAsignado->asignado = 0;
	free(unPedido);
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

		printf("[HCM] Habilitando ciclo: %i\n", numCiclo);

		for(i = 0; i < GRADO_MULTIPROCE; i++){
			signalSemaforoHabilitarCicloExec(i);
		}
		sem_post(habilitarCicloBlockReady);

		sleep(RETARDO_CICLO_CPU);

		printf("[HCM] Finalizando ciclo: %i\n", numCiclo);

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
