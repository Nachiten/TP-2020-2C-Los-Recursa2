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

	pthread_t hiloNewReady;

	pthread_create(&hiloNewReady, NULL, (void*)hiloNew_Ready, NULL);

	//sleep(3);

	pcb_pedido* unPlato = malloc(sizeof(pcb_pedido));
	unPlato->posObjetivoX = 10;
	unPlato->posObjetivoY = 15;

	agregarANew(unPlato);

	//sleep(3);

	pcb_pedido* unPlato2 = malloc(sizeof(pcb_pedido));
	unPlato2->posObjetivoX = 3;
	unPlato2->posObjetivoY = 5;

	agregarANew(unPlato2);

	//sleep(3);

	pcb_pedido* unPlato3 = malloc(sizeof(pcb_pedido));
	unPlato3->posObjetivoX = 7;
	unPlato3->posObjetivoY = 8;

	agregarANew(unPlato3);

	//sleep(3);

//	pcb_pedido* unPlato4 = malloc(sizeof(pcb_pedido));
//	unPlato4->posObjetivoX = 3;
//	unPlato4->posObjetivoY = 5;
//
//	agregarANew(unPlato4);

	sleep(5);

	int i;
	for (i = 0; i < GRADO_MULTIPROCE; i++){
		pthread_t hiloColaExec;

		int* numeroHilo = malloc(sizeof(int));

		*numeroHilo = i + 1;

		pthread_create(&hiloColaExec, NULL, (void*)hiloExec, numeroHilo);
	}

	pthread_join(hiloNewReady, NULL);
}

void hiloExec(int* numeroHilo){
	printf("ESPERANDO A QUE APAREZCA ALGUIEN EN READY\n");

	sem_wait(contadorProcesosEnReady);

	printf("HAY UN PROCESO EN READY.\n");

	pcb_pedido* pedidoAEjecutar = obtenerSiguienteDeReady();

	printf("Al hilo exec numero: %i se le asigna el pedido con objetivo %i-%i\n",
			*numeroHilo, pedidoAEjecutar->posObjetivoX, pedidoAEjecutar->posObjetivoY);

	// TODO | El hilo exec debe comenzar a ejecutar el pedido
}

pcb_pedido* obtenerSiguienteDeReady(){
	sem_wait(mutexReady);

	// Solo para FIFO
	// [1,2,3,4] (en ese orden) -> Sale 1

	pcb_pedido* pedidoADevolver = list_remove(colaReady, 0);

	sem_post(mutexReady);

	return pedidoADevolver;
}

// Hilo que maneja pasar los procesos de new a ready (asignar los repartidores a los platos)
void hiloNew_Ready(){

	while(1){

		printf("Esperando a que entre alguien a new..\n");
		// Espero a que me manden la señal que entro alguien nuevo
		sem_wait(contadorProcesosEnNew);

		printf("Alguien entro a new.\n");

		sem_wait(mutexNew);

		// Region critica de tocar la lista new
		pcb_pedido* unPlato = queue_pop(colaNew);

		sem_post(mutexNew);

		printf("Esperando a que haya repartidores disponibles..\n");

		printearValorSemaforo(contadorRepartidoresDisp, "ContadorRepartidoresDisp");
		sem_wait(contadorRepartidoresDisp);

		printf("Hay repartidores disponibles..\n");
		asignarRepartidorAPedido(unPlato);
	}
}

// 1 si esta desocupado | 0 si esta ocupado
int estaDesocupado(repartidor* unRepartidor){
	return !unRepartidor->asignado;
}

void asignarRepartidorAPedido(pcb_pedido* unPlato){
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
		int distanciaActual = distanciaDeRepartidorAObjetivo(miRepartidor->posX, miRepartidor->posY, unPlato->posObjetivoX, unPlato->posObjetivoY);

		printf("La distancia del repartidor numero %i es %i\n", miRepartidor->numeroRepartidor, distanciaActual);

		// Si la distancia es menor a la mejor entonces piso (para encontrar el menor)
		if (i == 0 || distanciaActual < mejorDistancia){
			mejorDistancia = distanciaActual;
			mejorRepartidor = miRepartidor;
		}

	}

	printf("Se asigna el repartidor numero: %i\n", mejorRepartidor->numeroRepartidor);

	// Asigno el repartidor al plato, ahora está ocupado
	mejorRepartidor->asignado = 1;
	unPlato->repartidorAsignado = mejorRepartidor;

	agregarAReady(unPlato);

	// Debug
}

// Calculo la distancia entre dos puntos
int distanciaDeRepartidorAObjetivo(int posXRepartidor, int posYRepartidor, int posXObjetivo, int posYObjetivo){
	int distanciaX = modulo(posXRepartidor - posXObjetivo);
	int distanciaY = modulo(posYRepartidor - posYObjetivo);

	return distanciaX + distanciaY;
}

// Valor absoluto del num
int modulo(int num){
	if (num < 0) return -num;
	else return num;
}

void printearValorSemaforo(sem_t* unSemaforo, char* nombre){
	int* valorSemaforo = malloc(sizeof(int));
	sem_getvalue(unSemaforo, valorSemaforo);
	printf("Valor semaforo %s es: %i\n", nombre, *valorSemaforo);
}

// Agrega el nuevo plato a la cola NEW
void agregarANew(pcb_pedido* unPedido){
	printf("Intentando insertar elemento en new..\n");

	//printearValorSemaforo(mutexNew);

	sem_wait(mutexNew);

	queue_push(colaNew, unPedido);

	sem_post(mutexNew);

	printf("Pude insertar elemento en new..\n");
	// Le envio la señal al hilo new-ready
	sem_post(contadorProcesosEnNew);
}

void agregarAReady(pcb_pedido* unPedido){

	printf("Intentando insertar elemento en ready.\n");

	sem_wait(mutexReady);

	list_add(colaReady, unPedido);

	sem_post(mutexReady);

	printf("Pude insertar elemento en ready.\n");

	sem_post(contadorProcesosEnReady);
}

// Inicializacion de semaforos necesarios
void iniciarSemaforosPlanificacion(){
	contadorProcesosEnNew = malloc(sizeof(sem_t));
	sem_init(contadorProcesosEnNew, 0, 0);

	contadorProcesosEnReady = malloc(sizeof(sem_t));
	sem_init(contadorProcesosEnReady, 0, 0);

	mutexNew = malloc(sizeof(sem_t));
	sem_init(mutexNew, 0, 1);

	mutexReady = malloc(sizeof(sem_t));
	sem_init(mutexReady, 0, 1);

	contadorRepartidoresDisp = malloc(sizeof(sem_t));
	sem_init(contadorRepartidoresDisp, 0, 0);
}

// Cargo la lista repartidores con la lista de todos los repartidores de la config
void leerPlanificacionConfig(t_config* config){

	repartidores = list_create();

	GRADO_MULTIPROCE = config_get_int_value(config, "GRADO_DE_MULTIPROCESAMIENTO");

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

		freeDeArray(posiciones);

		list_add(repartidores, miRepartidor);

		sem_post(contadorRepartidoresDisp);

	}

	freeDeArray(posicionesRepartidores);
	freeDeArray(frecuenciasDescansos);
	freeDeArray(tiemposDescanso);
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
