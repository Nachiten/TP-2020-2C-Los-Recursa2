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

	//sleep(7);

	pcb_pedido* unPedido = malloc(sizeof(pcb_pedido));
	unPedido->posObjetivoX = 10;
	unPedido->posObjetivoY = 15;
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

	sleep(3);

	pcb_pedido* unPedido2 = malloc(sizeof(pcb_pedido));
	unPedido2->posObjetivoX = 3;
	unPedido2->posObjetivoY = 5;
	unPedido2->pedidoID = 2;

	agregarANew(unPedido2);

	pcb_pedido* unPedido3 = malloc(sizeof(pcb_pedido));
	unPedido3->posObjetivoX = 7;
	unPedido3->posObjetivoY = 8;
	unPedido3->pedidoID = 3;

	agregarANew(unPedido3);

	pthread_join(hiloNewReady, NULL);
}

void hiloBlock_Ready(){

	while(1){

		sem_wait(habilitarCicloBlockReady);

		// TODO
		// En caso de HRRN se debe escanear cola ready para sumar 1 de tiempo de espera

		sem_wait(mutexBlock);

		int elementosEnBlock = list_size(colaBlock);

		if(elementosEnBlock == 0)
			printf("[HiloBlock] No hay pedidos en block.\n");

		int i;
		// Escaneo todos los elementos en block para sumar 1 ciclo de descanso a cada proceso
		for (i = 0; i < elementosEnBlock; i++){

			pcb_pedido* pedidoActual = list_get(colaBlock, i);

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
					agregarAReady(pedidoAReady);
				}

			} else if (pedidoActual->estadoBlocked == ESPERANDO_MSG){
				// Se debe corroborar si el mensaje ya llego, caso afirmativo mover a ready
				printf("[HiloBlock] El pedido %i esta esperando msg.\n", pedidoActual->pedidoID);
			} else if (pedidoActual->estadoBlocked == NO){
				printf("[HiloBlock | ERROR] El pedido %i esta en blocked pero no se le asigno por que esta bloqueado.\n", pedidoActual->pedidoID);
				exit(5);
			}

			// Si esta descansando sumo 1 al tiempo descansado

			// Si esta en no descansando tiro un error
		}

		sem_post(mutexBlock);

		sem_post(finalizarCicloBlockReady);

	}
}

void hiloExec(int* numHiloExecPuntero){

	int numHiloExec = *numHiloExecPuntero;

	// OLD
	//sem_wait(contadorProcesosEnReady);

	while(1){

		//printf("Viendo si hay alguien en ready.\n");
		pcb_pedido* pedidoAEjecutar = obtenerSiguienteDeReady();

		if (pedidoAEjecutar != NULL){

			printf("Al hilo exec numero: %i se le asigna el pedido %i\n",
					numHiloExec, pedidoAEjecutar->pedidoID);

			int desalojoCode;
			int cantidadCiclos = 0;

			// Calculo si tiene que desalojar o no y por que razon
			while ( (desalojoCode = codigoDesalojo(pedidoAEjecutar) ) == 0){
				waitSemaforoHabilitarCicloExec(numHiloExec);
				printf("[Hilo%i] Ejecuto ciclo numero: %i\n", numHiloExec, cantidadCiclos);

				pedidoAEjecutar->instruccionesRealizadas++;

				printf("[Hilo%i] Instrucciones restantes: %i\n", numHiloExec, pedidoAEjecutar->instruccionesTotales - pedidoAEjecutar->instruccionesRealizadas);

				signalSemaforoFinalizarCicloExec(numHiloExec);

				cantidadCiclos++;
			}

			if (desalojoCode == 1){
				printf("[Hilo%i] Estoy cansado.\n", numHiloExec);

				// Instrucciones totales: 10 -> 4
				// Instrucciones realizadas: 6 -> 0
				// Ahora las instrucciones totales reflejan las que faltan
				pedidoAEjecutar->instruccionesTotales -= pedidoAEjecutar->instruccionesRealizadas;
				pedidoAEjecutar->instruccionesRealizadas = 0;
				pedidoAEjecutar->estadoBlocked = DESCANSANDO;

			} else if (desalojoCode == 2){
				printf("[Hilo%i] Termine la rafaga. (espero mensaje)\n", numHiloExec);

				pedidoAEjecutar->instruccionesRealizadas = 0;
				pedidoAEjecutar->repartidorAsignado->posX = pedidoAEjecutar->posObjetivoX;
				pedidoAEjecutar->repartidorAsignado->posY = pedidoAEjecutar->posObjetivoY;
				pedidoAEjecutar->estadoBlocked = ESPERANDO_MSG;
			} else {
				printf("[Hilo%i | ERROR] El desalojo code tiene un valor invalido.\n");
				exit(6);
			}

			agregarABlock(pedidoAEjecutar);

		} else {
			waitSemaforoHabilitarCicloExec(numHiloExec);
			printf("[Hilo%i] Desperdicio un ciclo porque no hay nadie en ready.\n", numHiloExec);
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
	printf("[BLOCK] Ingresa el pedido %i.\n", elPedido->pedidoID);

	sem_post(mutexBlock);

	//printf("Pude insertar elemento en bloqueados..\n");
}



pcb_pedido* obtenerSiguienteDeReady(){

	// Solo para FIFO
	// [1,2,3,4] (en ese orden) -> Sale 1

	pcb_pedido* pedidoADevolver = NULL;

	sem_wait(mutexReady);

	if (list_size(colaReady) > 0){
		// CASO FIFO
		pedidoADevolver = list_remove(colaReady, 0);
	}

	sem_post(mutexReady);

	return pedidoADevolver;
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

	printf("[NEW] Se asigna el repartidor numero: %i al pedido numero: %i.\n", mejorRepartidor->numeroRepartidor, unPedido->pedidoID);

	// Asigno el repartidor al pedido, ahora está ocupado
	mejorRepartidor->asignado = 1;
	unPedido->repartidorAsignado = mejorRepartidor;
	unPedido->instruccionesTotales = mejorDistancia;
	unPedido->instruccionesRealizadas = 0;

	agregarAReady(unPedido);

	// Debug
}

// Calculo la distancia entre dos puntos
int distanciaDeRepartidorAObjetivo(repartidor* unRepartidor, pcb_pedido* elPedido){

	int posXRepartidor = unRepartidor->posX;
	int posYRepartidor = unRepartidor->posY;
	int posXObjetivo = elPedido->posObjetivoX;
	int posYObjetivo = elPedido->posObjetivoY;

	int distanciaX = modulo(posXRepartidor - posXObjetivo);
	int distanciaY = modulo(posYRepartidor - posYObjetivo);

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

// Agrega el nuevo pedido a la cola NEW
void agregarANew(pcb_pedido* unPedido)
{
	sem_wait(mutexNew);

	queue_push(colaNew, unPedido);

	printf("[NEW] Entra el nuevo pedido %i.\n", unPedido->pedidoID);

	sem_post(mutexNew);

	// Le envio la señal al hilo new-ready
	sem_post(contadorProcesosEnNew);
}

void agregarAReady(pcb_pedido* unPedido){

	//printf("Intentando insertar elemento en ready.\n");

	sem_wait(mutexReady);

	list_add(colaReady, unPedido);

	sem_post(mutexReady);

	printf("[READY] Ingresa pedido %i.\n", unPedido->pedidoID);

	// OLD
	//sem_post(contadorProcesosEnReady);
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
    int numCiclo = 0;

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
