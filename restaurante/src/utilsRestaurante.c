#include "utilsRestaurante.h"


void inicializarRestaurante(){

	configuracion = leerConfiguracion("/home/utnso/workspace/tp-2020-2c-Los-Recursa2/configs/restaurante.config");
    LOG_PATH = config_get_string_value(configuracion,"LOG_FILE_PATH"); //cargo el path del archivo log
	ip_sindicato = config_get_string_value(configuracion,"IP_SINDICATO");
	puerto_sindicato = config_get_string_value(configuracion,"PUERTO_SINDICATO");
	puerto_local = config_get_string_value(configuracion,"PUERTO_ESCUCHA");
	ip_app = config_get_string_value(configuracion,"IP_APP");
	puerto_app = config_get_string_value(configuracion,"PUERTO_APP");
    nombreRestaurante = config_get_string_value(configuracion, "NOMBRE_RESTAURANTE");
    quantumElegido = config_get_int_value(configuracion, "QUANTUM");
    algoritmoElegido = config_get_string_value(configuracion, "ALGORITMO_PLANIFICACION");
    RETARDO_CICLO_CPU = config_get_int_value(configuracion, "RETARDO_CICLO_CPU");

    logger = cargarUnLog(LOG_PATH, "Cliente");
    log_info(logger, "Obtuve de config el nombre: %s" , nombreRestaurante);

    //comunicarme con sindicato -> socket -> mensaje OBTENER_RESTAURANTE
    obtenerMetadataRestaurante();

    crearColasPlanificacion();
    crearHornos();
    crearHilosPlanificacion();

}


void obtenerMetadataRestaurante(){

	//me trato de conectar con sindicato que deberia estar levantado esperando que le vaya a pedir la info
	socket_sindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
    resultado_de_conexion(socket_sindicato, logger, "destino");

    obtener_restaurante* estructura = malloc(sizeof(obtener_restaurante));
    estructura->largoNombreRestaurante = strlen(nombreRestaurante);
    estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
    strcpy(estructura->nombreRestaurante, nombreRestaurante);

    //emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
    mandar_mensaje(estructura, OBTENER_RESTAURANTE, socket_sindicato);

    free(estructura->nombreRestaurante);
    free(estructura);

    printf("pude mandar la solicitud de metadata a sindic.\n");

    //recibo el codigo de operacion, ya se que va a ser RESPUESTA_OBTENER_R
    codigo_operacion codigoRecibido;
    bytesRecibidos(recv(socket_sindicato, &codigoRecibido, sizeof(codigo_operacion), MSG_WAITALL));

    printf("El codigo recibido del emisor es: %d", codigoRecibido);

    uint32_t sizePayload;
    bytesRecibidos(recv(socket_sindicato, &sizePayload, sizeof(uint32_t), MSG_WAITALL));

    printf("El size del buffer/payload para la metadata es: %u", sizePayload);

    respuesta_obtener_restaurante* estructuraRespuestaObtenerRestaurante = malloc(sizePayload);

   //recepcion del choclo divino
    recibir_mensaje(estructuraRespuestaObtenerRestaurante, RESPUESTA_OBTENER_REST, socket_sindicato);

    printf("pude recibir toda la de metadata de sindic.\n");


    // SI > 0 => True, si = 0 => false
    if (cantidadDeElementosEnArray(listaPlatos)){
    	printf("Hay platos\n");
    } else {
    	printf("No hay platos (no hay restaurant)\n");
    	exit(-2);
    }

    //trabajo interno con la metadata recibida
    miPosicionX = estructuraRespuestaObtenerRestaurante->posX;
    miPosicionY = estructuraRespuestaObtenerRestaurante->posY;
    cantHornos = estructuraRespuestaObtenerRestaurante->cantHornos;
    cantCocineros = estructuraRespuestaObtenerRestaurante->cantidadCocineros;

    //rescato las variables char* en punteros globales aparte para reutilizarlas en los mensajes futuros que tenga q contestar
    platos = malloc(estructuraRespuestaObtenerRestaurante->longitudPlatos+1);
    strcpy(platos, estructuraRespuestaObtenerRestaurante->platos);

    afinidades = malloc(estructuraRespuestaObtenerRestaurante->longitudAfinidades+1);
    strcpy(afinidades, estructuraRespuestaObtenerRestaurante->afinidades);

    listaPlatos = string_get_string_as_array(platos);
    listaAfinidades = string_get_string_as_array(afinidades);

    free(estructuraRespuestaObtenerRestaurante->platos);
    free(estructuraRespuestaObtenerRestaurante->afinidades);
    free(estructuraRespuestaObtenerRestaurante->precioPlatos);
    free(estructuraRespuestaObtenerRestaurante);
}



void crearColasPlanificacion(){
  int i=0;
  colaNew = queue_create();
  listaDeColasReady = list_create();
  colaBlock = list_create();

  while(listaAfinidades[i] != NULL){
    cola_ready* nuevaColaConAfinidad = malloc(sizeof(cola_ready));
    nuevaColaConAfinidad->afinidad = malloc(strlen(listaAfinidades[i])+1);
    strcpy(nuevaColaConAfinidad->afinidad, listaAfinidades[i]);
    nuevaColaConAfinidad->cola = queue_create();
    list_add(listaDeColasReady, nuevaColaConAfinidad);
  }
    cola_ready* colaSinAfinidad = malloc(sizeof(cola_ready));
    colaSinAfinidad->afinidad = "SinAfinidad";
    colaSinAfinidad->cola = queue_create();
    list_add(listaDeColasReady, colaSinAfinidad);
}


void crearHilosPlanificacion(){
  int i=0;
  int j=0;
  int cantCocinerosConAfinidad = 0;
  while(listaAfinidades[i] != NULL){
	  pthread_t unCocineroConAfinidad;
	  //ojo ver si no conviene hacer esto y pasarle el puntero dentro del array directamente
	  credencialesCocinero* datosCocineroConAfinidad = malloc(sizeof(credencialesCocinero));
	  datosCocineroConAfinidad->afinidad = malloc(strlen(listaAfinidades[i])+1);
	  strcpy(datosCocineroConAfinidad->afinidad, listaAfinidades[i]);
	  datosCocineroConAfinidad->idHilo = j;
	  pthread_create(&unCocineroConAfinidad, NULL, (void*)hiloExecCocinero, datosCocineroConAfinidad);
	  cantCocinerosConAfinidad++;
	  i++;
	  j++;
  }

  for(i=0; i<cantCocineros-cantCocinerosConAfinidad; i++){
	  pthread_t unCocineroSinAfinidad;
	  credencialesCocinero* datosCocineroSinAfinidad = malloc(sizeof(credencialesCocinero));
	  datosCocineroSinAfinidad->afinidad = NULL;
	  datosCocineroSinAfinidad->idHilo = j;
	  pthread_create(&unCocineroSinAfinidad, NULL, (void*)hiloExecCocinero, datosCocineroSinAfinidad);
	  i++;
	  j++;
  }

}

void crearHornos(){
	int i;
	listaDeHornos = list_create();
	colaParaHornear = queue_create();

	for(i=0; i<cantHornos; i++){
		t_horno* unHorno = malloc(sizeof(t_horno));
		unHorno->idHorno = i+1;
		unHorno->enUso = 0;
		list_add(listaDeHornos, unHorno);
	}
}



void agregarANew(pcb_plato* unPlato)
{
	sem_wait(mutexNew);

	queue_push(colaNew, unPlato);

	log_info(logger, "[NEW] Entra el nuevo plato %i, del pedido %i", unPlato->nombrePlato, unPlato->idPedido);

	sem_post(mutexNew);
	//Habilito la señal para el hilo que administra las colas de afinidad lo absorba en la que corresponde
	sem_post(contadorPlatosEnNew);
}


void agregarAReady(pcb_plato* unPlato){
    int i;
	sem_wait(mutexListaReady);

	for(i=0;i<list_size(listaDeColasReady);i++){

		cola_ready* unaColaReady = list_get(listaDeColasReady, i);
		if(strcmp(unaColaReady->afinidad, unPlato->nombrePlato) == 0){
			queue_push(unaColaReady->cola, unPlato);
			sem_post(mutexListaReady);
			return;
		} else {
		}


	}
	cola_ready* laColaReadySinAfinidad = list_get(listaDeColasReady, i);
	queue_push(laColaReadySinAfinidad->cola, unPlato);
	sem_post(mutexListaReady);

}

void agregarABlock(pcb_plato* elPlato){

	sem_wait(mutexBlock);

	list_add(colaBlock, elPlato);
	//printf("[BLOCK] Ingresa el plato %s del pedido %i.\n", elPlato->nombrePlato, elPlato->idPedido);

	sem_post(mutexBlock);

}

void agregarAExit(pcb_plato* elPlato){

}

// Hilo que maneja pasar los procesos de new a ready (preparar el Plato control block)
void hiloNew_Ready(){

	while(1){

		//printf("Esperando a que entre alguien a new..\n");
		// Espero a que me manden la señal que entro alguien nuevo
		sem_wait(contadorPlatosEnNew);

		sem_wait(mutexNew);

		// Region critica de tocar la lista new
		pcb_plato* unPlato = queue_pop(colaNew);

		sem_post(mutexNew);

        //preparo el PCB para meterlo en ready
        unPlato->enHorno = 0;
        unPlato->esperandoHorno = 0;
        unPlato->motivoBlock = NO_BLOCK;
        unPlato->quantumRestante = 0;
        unPlato->duracionBlock = 0;
        //los pasos de receta los preparo aca si se le llega a complicar a nico, comunicacion sindicato

		agregarAReady(unPlato);
	}
}


void hiloExecCocinero(credencialesCocinero* datosCocinero){

	if(strcmp(algoritmoElegido, "FIFO") == 0){

		int i;

		while(1){
			pcb_plato* platoAEjecutar = obtenerSiguienteDeReady(datosCocinero->afinidad);

		    if(list_size(platoAEjecutar->pasosReceta) < 1){
				  //el plato fue terminado, lo mando a exit
			}

		    int cantidadCiclos = 1;

		    for(i=0;i<list_size(platoAEjecutar->pasosReceta); i++){

                paso_receta* pasoPendiente = list_get(platoAEjecutar->pasosReceta, i);

		        switch(pasoPendiente->accion){

		        case REPOSAR:
          //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO, lo saco de aca
                 platoAEjecutar->motivoBlock = REPOSO;
                 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
                 list_remove(platoAEjecutar->pasosReceta, i);
                 agregarABlock(platoAEjecutar);
                 i=list_size(platoAEjecutar->pasosReceta);

		        break;

		        case HORNEAR:
          //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO y dentro de un horno, lo saco de aca
		         platoAEjecutar->motivoBlock = HORNO;
				 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
				 list_remove(platoAEjecutar->pasosReceta, i);
                 agregarABlock(platoAEjecutar);
                 i=list_size(platoAEjecutar->pasosReceta);

		        break;

		        case OTRO:
				 while(pasoPendiente->duracionAccion != 0){
				 waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
				 pasoPendiente->duracionAccion--;
				 cantidadCiclos++;
				 }
				 list_remove(platoAEjecutar->pasosReceta, i);

		        break;

		        }



		   }
		   //no hay platos para ejecutar

	}



    //si no es FIFO, se eligio RR
	} else {
		int i;

		while(1){
			pcb_plato* platoAEjecutar = obtenerSiguienteDeReady(datosCocinero->afinidad);

			if(list_size(platoAEjecutar->pasosReceta) < 1){
			   //el plato fue terminado, lo mando a exit
			}
			int cantidadCiclos = 1;
			platoAEjecutar->quantumRestante = quantumElegido;

			for(i=0;i<list_size(platoAEjecutar->pasosReceta); i++){

				paso_receta* pasoPendiente = list_get(platoAEjecutar->pasosReceta, i);

				switch(pasoPendiente->accion){

				case REPOSAR:
		  //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO, lo saco de aca
				 platoAEjecutar->motivoBlock = REPOSO;
				 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
				 list_remove(platoAEjecutar->pasosReceta, i);
				 agregarABlock(platoAEjecutar);
				 i=list_size(platoAEjecutar->pasosReceta);

				break;

				case HORNEAR:
		  //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO y dentro de un horno, lo saco de aca
				 platoAEjecutar->motivoBlock = HORNO;
				 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
				 list_remove(platoAEjecutar->pasosReceta, i);
				 agregarABlock(platoAEjecutar);
				 i=list_size(platoAEjecutar->pasosReceta);

				break;

				case OTRO:

				 while(pasoPendiente->duracionAccion != 0){
			  //Chequeo si se quedo sin quantum para desalojar
				   if(platoAEjecutar->quantumRestante == 0){
			  //Me fijo si el quantum que se le dio al cocinero para atender ese plato fue suficiente para
			  //completar el paso, si no lo fue, lo devuelvo a ready sin tocar la lista de pasos
			  //Si logro ser suficiente, saco el paso de la lista entero porque ya se que se van a ocupar de el luego
					   if(pasoPendiente->duracionAccion == 0){
					  	list_remove(platoAEjecutar->pasosReceta, i);
					  	agregarAReady(platoAEjecutar);
					  	break;
					  } else {
                        agregarAReady(platoAEjecutar);
                        break;
					  }
				   }
				   waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
				   pasoPendiente->duracionAccion--;
				   platoAEjecutar->quantumRestante--;
				   cantidadCiclos++;
				 }

				break;

			}



		  }



	}


}

}



void hiloBlockReady(){
	while(1){

	  sem_wait(habilitarCicloBlockReady);


	  int elementosEnBlock = list_size(colaBlock);

	  if(elementosEnBlock == 0)
	  	log_trace(logger, "[BLOCK] No hay platos en block.");

	  int i;

	  // Escaneo todos los elementos en block para sumar 1 ciclo de reposo/horno a cada plato
	  for (i = 0; i < elementosEnBlock; i++){

	    sem_wait(mutexBlock);
	  	pcb_plato* platoActual = list_get(colaBlock, i);
	  	sem_post(mutexBlock);

	  	switch(platoActual->motivoBlock){

	  	case NO_BLOCK:
	  		//error, jamas deberia pasar
	  		break;

	  	case REPOSO:
          if(platoActual->duracionBlock == 0){
        	//el plato termino de reposar, es devuelto a ready
        	platoActual->motivoBlock = NO_BLOCK;
        	if(list_size(platoActual->pasosReceta) < 1){
        	  agregarAExit(platoActual);
        	}
        	agregarAReady(platoActual);
        	break;
          }
          platoActual->duracionBlock--;

          break;


	  	case HORNO:
	  	  if(platoActual->esperandoHorno == 1){
	  		 //el plato ya se encuentra esperando para entrar a un horno, no me ocupo de hacer nada
	  	  } else {
	  		//le tengo que buscar un horno, lo pongo en la cola para hornear
	  		 sem_wait(mutexColaHornos);
	  		 queue_push(colaParaHornear, platoActual);
	  		 platoActual->esperandoHorno = 1;
	  		 sem_post(mutexColaHornos);
	  		 sem_post(contadorPlatosEnColaHornos);
	  	  }

	  	}



	  }


   sem_post(finalizarCicloBlockReady);
}
}


void hiloEntradaSalida(){



}


pcb_plato* obtenerSiguienteDeReady(char* afinidad){
	int i;
	pcb_plato* elPlatoPlanificado;
	for(i=0; i<list_size(listaDeColasReady);i++){
		cola_ready* unaColaReady = list_get(listaDeColasReady, i);
		if(strcmp(afinidad, unaColaReady->afinidad) == 0){
			elPlatoPlanificado = queue_pop(unaColaReady->cola);
			return elPlatoPlanificado;
		}
	}
	cola_ready* laColaReadySinAfinidad = list_get(listaDeColasReady, i);
	elPlatoPlanificado = queue_pop(laColaReadySinAfinidad->cola);
	return elPlatoPlanificado;
}



// Inicializacion de semaforos necesarios
void iniciarSemaforosPlanificacion(){
	contadorPlatosEnNew = malloc(sizeof(sem_t));
	sem_init(contadorPlatosEnNew, 0, 0);

	contadorPlatosEnColaHornos = malloc(sizeof(sem_t));
	sem_init(contadorPlatosEnColaHornos, 0, 0);

	mutexNew = malloc(sizeof(sem_t));
	sem_init(mutexNew, 0, 1);

	mutexListaReady = malloc(sizeof(sem_t));
	sem_init(mutexListaReady, 0, 1);

	mutexBlock = malloc(sizeof(sem_t));
	sem_init(mutexBlock, 0, 1);

}

void iniciarSemaforosCiclos(){

	habilitarCicloBlockReady= malloc(sizeof(sem_t));
	sem_init(habilitarCicloBlockReady, 0, 0);
	finalizarCicloBlockReady= malloc(sizeof(sem_t));
	sem_init(finalizarCicloBlockReady, 0, 0);
	habilitarCicloEntradaSalida= malloc(sizeof(sem_t));
	sem_init(habilitarCicloEntradaSalida, 0, 0);
	finalizarCicloEntradaSalida= malloc(sizeof(sem_t));
	sem_init(finalizarCicloEntradaSalida, 0, 0);


	listaSemHabilitarCicloExec = list_create();
	listaSemFinalizarCicloExec = list_create();

	int i;
	for(i=0; i<cantCocineros; i++){
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

		for(i = 0; i < cantCocineros; i++){
			signalSemaforoHabilitarCicloExec(i);
		}
		sem_post(habilitarCicloBlockReady);
        sem_post(habilitarCicloEntradaSalida);
		sleep(RETARDO_CICLO_CPU);

		log_trace(logger, "[HCM] Finalizando ciclo: %i", numCiclo);

		for(i = 0; i < cantCocineros; i++){
			waitSemaforoFinalizarCicloExec(i);
		}
		sem_wait(finalizarCicloEntradaSalida);
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

