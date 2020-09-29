/*
 * planificacion.h
 *
 *  Created on: 28 sep. 2020
 *      Author: utnso
 */

#ifndef SRC_PLANIFICACION_H_
#define SRC_PLANIFICACION_H_

#include "app.h"

t_list* repartidores;
t_queue* colaNew;

sem_t* alguienEnNew;
sem_t* mutexNew;

typedef struct{
	int numeroRepartidor;
	int posX;
	int posY;
	int frecuenciaDescanso;
	int tiempoDescanso;
	int asignado;
}repartidor;

typedef struct{
	int instruccionesRestantes;
	repartidor* repartidorAsignado;
	int posObjetivoX;
	int posObjetivoY;
}pcb_plato;

void leerRepartidoresDeConfig(t_config*);
void iniciarPlanificacion();
void freeDeArray(char**);
void agregarANew(pcb_plato* unPlato);
void iniciarSemaforosPlanificacion();
void asignarRepartidorAPlato(pcb_plato*);
int modulo(int);
int distanciaDeRepartidorAObjetivo(int, int, int, int);

#endif /* SRC_PLANIFICACION_H_ */
