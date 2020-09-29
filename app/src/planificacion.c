/*
 * planificacion.c
 *
 *  Created on: 28 sep. 2020
 *      Author: utnso
 */

#include "planificacion.h"

// Hilo que maneja pasar los procesos de new a ready (asignar los repartidores a los platos)
void hiloNew_Ready(){

	while(1){

		printf("Esperando a que entre alguien a new..\n");
		// Espero a que me manden la señal que entro alguien nuevo
		sem_wait(alguienEnNew);

		printf("Alguien entro a new.\n");

		sem_wait(mutexNew);

		// Region critica de tocar la lista new
		pcb_plato* unPlato = queue_pop(colaNew);

		sem_post(mutexNew);

		asignarRepartidorAPlato(unPlato);
	}
}

// 1 si esta desocupado | 0 si esta ocupado
int estaDesocupado(repartidor* unRepartidor){
	return !unRepartidor->asignado;
}

void asignarRepartidorAPlato(pcb_plato* unPlato){
	// Filtro la lista y solo dejo los repartidores que no estan ocupados
	t_list* repartidoresDisponibles = list_filter(repartidores, (void*)estaDesocupado);

	// Checkeo si hay algun repartidor disponible
	if (list_size(repartidoresDisponibles) == 0){
		printf("No hay repartidores disponibles.\n");
	}

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

void iniciarPlanificacion(){
	printf("Iniciando Planificacion.\n");

	colaNew = queue_create();

	iniciarSemaforosPlanificacion();

	pthread_t hiloNewReady;

	pthread_create(&hiloNewReady, NULL, (void*)hiloNew_Ready, NULL);

	sleep(5);

	pcb_plato* unPlato = malloc(sizeof(pcb_plato));

	unPlato->posObjetivoX = 10;
	unPlato->posObjetivoY = 15;

	pcb_plato* unPlato2 = malloc(sizeof(pcb_plato));

	unPlato2->posObjetivoX = 3;
	unPlato2->posObjetivoY = 5;

	agregarANew(unPlato);
	agregarANew(unPlato2);

	pthread_join(hiloNewReady, NULL);
}

// Agrega el nuevo plato a la cola NEW
void agregarANew(pcb_plato* unPlato){
	printf("Intentando insertar elemento en new..\n");
	sem_wait(mutexNew);

	queue_push(colaNew, unPlato);

	sem_post(mutexNew);

	printf("Pude insertar elemento en new..\n");
	// Le envio la señal al hilo new-ready
	sem_post(alguienEnNew);
}

// Inicializacion de semaforos necesarios
void iniciarSemaforosPlanificacion(){
	alguienEnNew = malloc(sizeof(sem_t));
	sem_init(alguienEnNew, 0, 0);

	mutexNew = malloc(sizeof(sem_t));
	sem_init(mutexNew, 0, 1);
}

// Cargo la lista repartidores con la lista de todos los repartidores de la config
void leerRepartidoresDeConfig(t_config* config){

	repartidores = list_create();

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

		if (i % 2 == 1)
			miRepartidor->asignado = 1;
		else
			miRepartidor->asignado = 0;

		freeDeArray(posiciones);

		list_add(repartidores, miRepartidor);

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
