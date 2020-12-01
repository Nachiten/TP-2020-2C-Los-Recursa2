#include "utilsRestaurante.h"


void inicializarRestaurante(char* elPathDeLaConfig){


	configuracion = leerConfiguracion(elPathDeLaConfig);
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

    strcat(nombreRestaurante,"\0");

    logger = cargarUnLog(LOG_PATH, "Restaurante");

    log_info(logger, "Obtuve de config el nombre: %s" , nombreRestaurante);

    //comunicarme con sindicato -> socket -> mensaje OBTENER_RESTAURANTE
    obtenerMetadataRestaurante();

}


void obtenerMetadataRestaurante(){
    uint32_t exito;
    int32_t sizeAAllocar, nuevoSocketSindicato;


	//me trato de conectar con sindicato que deberia estar levantado esperando que le vaya a pedir la info
	nuevoSocketSindicato = establecer_conexion(ip_sindicato,puerto_sindicato);
	if(socket_sindicato < 0){
		log_info(logger, "Sindicato esta muerto, me muero yo tambien");
	    exit(-2);
	}

    obtener_restaurante* estructura = malloc(sizeof(obtener_restaurante));
    estructura->largoNombreRestaurante = strlen(nombreRestaurante);
    estructura->nombreRestaurante = malloc(estructura->largoNombreRestaurante+1);
    strcpy(estructura->nombreRestaurante, nombreRestaurante);

    //emision del mensaje para pedir la info, OBTENER_RESTAURANTE [nombreR]
    mandar_mensaje(estructura, OBTENER_RESTAURANTE, nuevoSocketSindicato);

    free(estructura->nombreRestaurante);
    free(estructura);

    los_recv_repetitivos(nuevoSocketSindicato, &exito, &sizeAAllocar);
    if(exito == 1){
    	respuesta_obtener_restaurante* estructuraRespuestaObtenerRestaurante = malloc(sizeAAllocar);
    	//recepcion del choclo divino
    	recibir_mensaje(estructuraRespuestaObtenerRestaurante, RESPUESTA_OBTENER_REST, nuevoSocketSindicato);
    	//printf("pude recibir toda la de metadata de sindic.\n");

		//trabajo interno con la metadata recibida
		miPosicionX = estructuraRespuestaObtenerRestaurante->posX;
		miPosicionY = estructuraRespuestaObtenerRestaurante->posY;
		cantHornos = estructuraRespuestaObtenerRestaurante->cantHornos;
		id_global = estructuraRespuestaObtenerRestaurante->cantPedidos;
		cantCocineros = estructuraRespuestaObtenerRestaurante->cantidadCocineros;

		//rescato las variables char* en punteros globales aparte para reutilizarlas en los mensajes futuros que tenga q contestar
		platos = malloc(estructuraRespuestaObtenerRestaurante->longitudPlatos+1);
		strcpy(platos, estructuraRespuestaObtenerRestaurante->platos);

		afinidades = malloc(estructuraRespuestaObtenerRestaurante->longitudAfinidades+1);
		strcpy(afinidades, estructuraRespuestaObtenerRestaurante->afinidades);

		listaPlatos = string_get_string_as_array(platos);
		listaAfinidades = string_get_string_as_array(afinidades);

		// SI > 0 => True, si = 0 => false
		if (cantidadDeElementosEnArray(listaPlatos)){
			printf("Hay platos.\n");
		} else {
			printf("No hay platos (no hay restaurant).\n");
			exit(-2);
		}

		free(estructuraRespuestaObtenerRestaurante->platos);
		free(estructuraRespuestaObtenerRestaurante->afinidades);
		free(estructuraRespuestaObtenerRestaurante->precioPlatos);
		free(estructuraRespuestaObtenerRestaurante);

    }else{
    	puts("Hubo un problemita al recibir la metadata de sindicatox.\n");
    }
    close(socket_sindicato);
}



void crearColasPlanificacion(){
  int i=0;
  colaNew = queue_create();
  colaBlock = list_create();
  colaParaHornear = queue_create();
  platosHorneandose = list_create();

  listaDeColasReady = list_create();
  colaReadySinAfinidad = queue_create();

  while(listaAfinidades[i] != NULL){
    cola_ready* nuevaColaConAfinidad = malloc(sizeof(cola_ready));
    nuevaColaConAfinidad->afinidad = malloc(strlen(listaAfinidades[i])+1);
    strcpy(nuevaColaConAfinidad->afinidad, listaAfinidades[i]);
    nuevaColaConAfinidad->cola = queue_create();
    list_add(listaDeColasReady, nuevaColaConAfinidad);
    i++;
  }
}


void crearHilosPlanificacion(){
  int i=0;
  int j=0;
  int cantCocinerosConAfinidad = 0;

  pthread_t threadNewReady;
  pthread_create(&threadNewReady, NULL, (void*)hiloNewReady, NULL);

  pthread_t threadBlockReady;
  pthread_create(&threadBlockReady, NULL, (void*)hiloBlockReady, NULL);

  pthread_t threadEntradaSalida;
  pthread_create(&threadEntradaSalida, NULL, (void*)hiloEntradaSalida, NULL);

  pthread_t threadControlCiclos;
  pthread_create(&threadControlCiclos, NULL, (void*)hiloCiclosMaestro, NULL);


  while(listaAfinidades[i] != NULL){
	  pthread_t unCocineroConAfinidad;
	  //ojo ver si no conviene hacer esto y pasarle el puntero dentro del array directamente
	  credencialesCocinero* datosCocineroConAfinidad = malloc(sizeof(credencialesCocinero));
	  datosCocineroConAfinidad->afinidad = malloc(strlen(listaAfinidades[i])+1);
	  strcpy(datosCocineroConAfinidad->afinidad, listaAfinidades[i]);
	  datosCocineroConAfinidad->idHilo = i;
	  pthread_create(&unCocineroConAfinidad, NULL, (void*)hiloExecCocinero, datosCocineroConAfinidad);
	  cantCocinerosConAfinidad++;
	  i++;
	  j++;
  }

  for(i=0; i<cantCocineros-cantCocinerosConAfinidad; i++){
	  pthread_t unCocineroSinAfinidad;
	  credencialesCocinero* datosCocineroSinAfinidad = malloc(sizeof(credencialesCocinero));
	  datosCocineroSinAfinidad->afinidad = "SinAfinidad";
	  datosCocineroSinAfinidad->idHilo = j;
	  pthread_create(&unCocineroSinAfinidad, NULL, (void*)hiloExecCocinero, datosCocineroSinAfinidad);
	  i++;
	  j++;
  }

}

/*
void crearHornos(){
//
}*/


void agregarANew(pcb_plato* unPlato)
{
	sem_wait(mutexNew);
	queue_push(colaNew, unPlato);
	sem_post(mutexNew);

	//LOG DE ENUNCIADO!!!!1!1!
	sem_wait(semLog);
	log_info(logger, "[NEW] Entra el nuevo plato < %s >, del pedido < %d >"
			,unPlato->nombrePlato, unPlato->idPedido);
	sem_post(semLog);

	//Habilito la señal para el hilo que administra las colas de afinidad lo absorba en la que corresponde
	sem_post(contadorPlatosEnNew);
}


void agregarAReady(pcb_plato* unPlato){
    int i;
	sem_wait(mutexListaReady);

	if(list_size(listaDeColasReady) == 0){
		sem_post(mutexListaReady);
		sem_wait(semLog);
		log_trace(logger, "[READY] No hay colas de planificacion con afinidad creadas.");
		sem_post(semLog);
	}

	for(i=0;i<list_size(listaDeColasReady);i++){

		cola_ready* unaColaReady = list_get(listaDeColasReady, i);
		if(strcmp(unaColaReady->afinidad, unPlato->nombrePlato) == 0){
			queue_push(unaColaReady->cola, unPlato);
			sem_wait(semLog);
			log_trace(logger, "[READY] Entra el plato < %s >, del pedido < %d > a su cola de afinidad."
						,unPlato->nombrePlato, unPlato->idPedido);
			sem_post(semLog);
			sem_post(mutexListaReady);
			return;
		} else {
		}
	}

	queue_push(colaReadySinAfinidad, unPlato);
	sem_wait(semLog);
	log_trace(logger, "[READY] Entra el plato < %s >, del pedido < %d > a la cola sin afinidad."
				,unPlato->nombrePlato, unPlato->idPedido);
	sem_post(semLog);
	sem_post(mutexListaReady);

}

void agregarABlock(pcb_plato* elPlato){

	sem_wait(mutexBlock);
	list_add(colaBlock, elPlato);
	sem_wait(semLog);
	log_trace(logger, "[BLOCK] Ingresa el plato %s del pedido %d.\n", elPlato->nombrePlato, elPlato->idPedido);
	sem_post(semLog);
	sem_post(mutexBlock);

}

void agregarAExit(pcb_plato* elPlato){
	 plato_listo* notificacionPlatoListoAMandar;
     Pedido* elPedidoAsociado;
     uint32_t exito;
     int32_t sizeAAllocar, nuevoSocketSindicato, indiceDelPedidoAsociado;

	sem_wait(semLog);
	//LOG DE ENUNCIADO!!!!1!1!
	log_info(logger, "[EXIT] Entra el plato < %s >, del pedido < %i > por haber culminado su receta."
				, elPlato->nombrePlato, elPlato->idPedido);
	sem_post(semLog);



	//se envia un mensaje a sindicato para q actualice el estado del pedido,
	//con el id del mismo y mi restaurante, aparentemente tambien al modulo que lo solicito el pedido
	//Se enviaria PLATO_LISTO, ver con nico despues los detalles
	notificacionPlatoListoAMandar = malloc(sizeof(plato_listo));
	notificacionPlatoListoAMandar->idPedido = elPlato->idPedido;
	notificacionPlatoListoAMandar->largoNombrePlato = strlen(elPlato->nombrePlato);
	notificacionPlatoListoAMandar->largoNombreRestaurante = strlen(nombreRestaurante);
	notificacionPlatoListoAMandar->nombrePlato = malloc(notificacionPlatoListoAMandar->largoNombrePlato+1);
	notificacionPlatoListoAMandar->nombreRestaurante = malloc(notificacionPlatoListoAMandar->largoNombreRestaurante+1);
	strcpy(notificacionPlatoListoAMandar->nombrePlato, elPlato->nombrePlato);
	strcpy(notificacionPlatoListoAMandar->nombreRestaurante, nombreRestaurante);

	indiceDelPedidoAsociado = buscar_pedido_por_id(elPlato->idPedido);

	if(indiceDelPedidoAsociado == -2){
		puts("Estas al horno");
		exit(-2);
	}

	sem_wait(semListaPedidos);
	elPedidoAsociado = list_get(listaPedidos,indiceDelPedidoAsociado);
	sem_post(semListaPedidos);

    mandar_mensaje(notificacionPlatoListoAMandar, PLATO_LISTO, elPedidoAsociado->socket_cliente);

    los_recv_repetitivos(elPedidoAsociado->socket_cliente, &exito, &sizeAAllocar);
    if(exito == 1){

    	respuesta_ok_error* respuestaNotificacion = malloc(sizeof(respuesta_ok_error));
    	recibir_mensaje(respuestaNotificacion, RESPUESTA_PLATO_LISTO, elPedidoAsociado->socket_cliente);

    	free(respuestaNotificacion);
    }

    nuevoSocketSindicato = establecer_conexion(ip_sindicato, puerto_sindicato);
	if(nuevoSocketSindicato < 0){
		sem_wait(semLog);
		log_error(logger, "Sindicato esta muerto, me muero yo tambien");
		sem_post(semLog);
		exit(-2);
	}

    mandar_mensaje(notificacionPlatoListoAMandar, PLATO_LISTO, nuevoSocketSindicato);

    los_recv_repetitivos(nuevoSocketSindicato, &exito, &sizeAAllocar);
    if(exito == 1){

		respuesta_ok_error* respuestaNotificacion = malloc(sizeof(respuesta_ok_error));
		recibir_mensaje(respuestaNotificacion, RESPUESTA_PLATO_LISTO, nuevoSocketSindicato);

		free(respuestaNotificacion);
	}

	//No se si hace falta esto, confirmenme
	//list_destroy(elPlato->pasosReceta)
	//se libera memoria ocupada por donPlato
	free(elPlato->nombrePlato);
	free(elPlato);
	//se libera la memoria ocupada por donNotificacionDePlatoListo
	free(notificacionPlatoListoAMandar->nombrePlato);
	free(notificacionPlatoListoAMandar->nombreRestaurante);
	free(notificacionPlatoListoAMandar);

}

// Hilo que maneja pasar los procesos de new a ready (preparar el Plato control block)
void hiloNewReady(){

	while(1){

		//printf("Esperando a que entre alguien a new..\n");
		// Espero a que me manden la señal que entro alguien nuevo
		sem_wait(contadorPlatosEnNew);

		sem_wait(mutexNew);

		// Region critica de tocar la lista new
		pcb_plato* unPlato = queue_pop(colaNew);

		sem_post(mutexNew);

        //preparo el PCB para meterlo en ready
        unPlato->motivoBlock = NO_BLOCK;
        unPlato->quantumRestante = 0;
        unPlato->duracionBlock = 0;

		agregarAReady(unPlato);
	}
}


void hiloExecCocinero(credencialesCocinero* datosCocinero){

	if(strcmp(algoritmoElegido, "FIFO") == 0){

		int i;
		int deboDesalojar;
		int cantidadCiclos;

		while(1){
            pcb_plato* platoAEjecutar;

			if(strcmp(datosCocinero->afinidad, "SinAfinidad") == 0){
				platoAEjecutar = obtenerSiguienteDeReadySinAfinidad();
			} else {
				platoAEjecutar = obtenerSiguienteDeReadyConAfinidad(datosCocinero->afinidad);
			}

			if(platoAEjecutar == NULL){
				waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
				sem_wait(semLog);
				log_trace(logger, "[EXEC-%d] Cocinero desperdicia ciclo porque no hay nadie en ready.",
						datosCocinero->idHilo+1);
				sem_post(semLog);
				signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);
				continue;
			}

			if(list_size(platoAEjecutar->pasosReceta) < 1){
				//obtuve un plato de ready sin receta para procesar, jamas deberia pasar, asi que me muero
				printf("ERROR | El plato de ready no tiene receta.\n");
				exit(2);
			}

			cantidadCiclos = 1;
			deboDesalojar = 0;

		    for(i=0;i<list_size(platoAEjecutar->pasosReceta); i++){

                paso_receta* pasoPendiente = list_get(platoAEjecutar->pasosReceta, i);

		      switch(pasoPendiente->accion){

		        case REPOSAR:
          //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO, lo saco de aca
		         waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
                 platoAEjecutar->motivoBlock = REPOSO;
                 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
                 //list_remove(platoAEjecutar->pasosReceta, i);
                 list_remove_and_destroy_element(platoAEjecutar->pasosReceta, i, (void*)free);
                 agregarABlock(platoAEjecutar);
          //esto es para forzar al for a terminar y poder pasar al siguiente plato inmediatamente
                 deboDesalojar = 1;
                 cantidadCiclos++;
                 signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);
		        break;

		        case HORNEAR:
          //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO y dentro de un horno, lo saco de aca
		         waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
		         platoAEjecutar->motivoBlock = HORNO;
				 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
				 //list_remove(platoAEjecutar->pasosReceta, i);
				 list_remove_and_destroy_element(platoAEjecutar->pasosReceta, i, (void*)free);
                 agregarABlock(platoAEjecutar);
          //esto es para forzar al for a terminar y poder pasar al siguiente plato inmediatamente
                 deboDesalojar = 1;
                 cantidadCiclos++;
                 signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);
		        break;

		        case OTRO:
				 while(pasoPendiente->duracionAccion > 0){
				 waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
				 pasoPendiente->duracionAccion--;
				 cantidadCiclos++;
				 signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);
				 }
				 //list_remove(platoAEjecutar->pasosReceta, i);
				 list_remove_and_destroy_element(platoAEjecutar->pasosReceta, i, (void*)free);
                 i--;
				 if(list_size(platoAEjecutar->pasosReceta) < 1){
					agregarAExit(platoAEjecutar);
					deboDesalojar = 1;
				 }
		        break;

		      }

		        if(deboDesalojar == 1){
		        	break;
		        }

		   }
		   //no hay platos para ejecutar, no pasa naranja

	}



    //si no es FIFO, se eligio RR
	} else {
		int i;
		int deboDesalojar;
		int cantidadCiclos;

		while(1){
			pcb_plato* platoAEjecutar;

			if(strcmp(datosCocinero->afinidad, "SinAfinidad") == 0){
				platoAEjecutar = obtenerSiguienteDeReadySinAfinidad(datosCocinero->afinidad);
			} else {
				platoAEjecutar = obtenerSiguienteDeReadyConAfinidad(datosCocinero->afinidad);
			}


			if(platoAEjecutar == NULL){
				waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
				sem_wait(semLog);
				log_trace(logger, "[EXEC-%d] Cocinero desperdicia ciclo porque no hay nadie en ready.",
						datosCocinero->idHilo+1);
				sem_post(semLog);
				signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);
				continue;
			}

			if(list_size(platoAEjecutar->pasosReceta) < 1){
			//obtuve un plato de ready sin receta para procesar, jamas deberia pasar, asi que me muero
			   exit(-1);
			}

			cantidadCiclos = 1;
			deboDesalojar = 0;

			platoAEjecutar->quantumRestante = quantumElegido;

			for(i=0;i<list_size(platoAEjecutar->pasosReceta); i++){

				paso_receta* pasoPendiente = list_get(platoAEjecutar->pasosReceta, i);

				switch(pasoPendiente->accion){

			case REPOSAR:
		  //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO, lo saco de aca
		  //gasto 1 ciclo en solicitar su cambio de estado

				 waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
				 platoAEjecutar->motivoBlock = REPOSO;
				 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
				 list_remove_and_destroy_element(platoAEjecutar->pasosReceta, i, (void*)free);
				 agregarABlock(platoAEjecutar);
				 deboDesalojar = 1;
				 signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);

				break;

			case HORNEAR:
		  //el plato tiene que cumplir sus ciclos en el estado BLOQUEADO y dentro de un horno, lo saco de aca
		  //gasto 1 ciclo en solicitar su cambio de estado

				 waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
				 platoAEjecutar->motivoBlock = HORNO;
				 platoAEjecutar->duracionBlock = pasoPendiente->duracionAccion;
				 list_remove_and_destroy_element(platoAEjecutar->pasosReceta, i, (void*)free);
				 agregarABlock(platoAEjecutar);
				 deboDesalojar = 1;
				 signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);

				break;

			case OTRO:

			  while(pasoPendiente->duracionAccion > 0){
			  //Chequeo si aun tiene quantum para laburar

				   if(platoAEjecutar->quantumRestante > 0)
				   {

					  waitSemaforoHabilitarCicloExec(datosCocinero->idHilo);
					  pasoPendiente->duracionAccion--;
					  platoAEjecutar->quantumRestante--;
					  cantidadCiclos++;
					  signalSemaforoFinalizarCicloExec(datosCocinero->idHilo);

					   if(pasoPendiente->duracionAccion == 0)
					   {

					  	 //list_remove(platoAEjecutar->pasosReceta, i);
					  	 list_remove_and_destroy_element(platoAEjecutar->pasosReceta, i, (void*)free);
                         i--;

					  	   if(list_size(platoAEjecutar->pasosReceta)<1)
					  	   {
					  	     agregarAExit(platoAEjecutar);
					  	     deboDesalojar = 1;
					  	     break;
					  	   } else {
					  		 break;
					  	   }

					  }


				   } else {
					   agregarAReady(platoAEjecutar);
					   deboDesalojar = 1;
					   break;
				   }


				 }

				break;

			}

				if(deboDesalojar == 1){
					break;
				}


		  }


	}


}

}



void hiloBlockReady(){
	while(1){

	  sem_wait(habilitarCicloBlockReady);

      sem_wait(mutexBlock);
	  int elementosEnBlock = list_size(colaBlock);
	  sem_post(mutexBlock);

	  if(elementosEnBlock == 0){
		sem_wait(semLog);
	  	log_trace(logger, "[BLOCK] No hay platos en block. Desperdiciando ciclo.");
	  	sem_post(semLog);
	  	sem_post(finalizarCicloBlockReady);
        continue;
	  }

	  int i;

	  // Escaneo todos los elementos en block para sumar 1 ciclo de reposo/horno a cada plato
	  for (i = 0; i < elementosEnBlock; i++){

	    sem_wait(mutexBlock);
	  	pcb_plato* platoActual = list_get(colaBlock, i);
	  	sem_post(mutexBlock);

	  	switch(platoActual->motivoBlock)
	  	{
	  	case NO_BLOCK:
	  		//jamas deberia entrar un plato aca sin tener su motivoBlock alterado
	  		break;

	  	case REPOSO:
          if(platoActual->duracionBlock == 0){
        	//el plato termino de reposar, es devuelto a ready siempre A MENOS QUE este fue su ultimo paso
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
	  		 sem_wait(mutexBlock);
	  	     list_remove(colaBlock, i);
	  		 sem_post(mutexBlock);
	  		//le tengo que buscar un horno, lo pongo en la cola para hornear
	  		 sem_wait(mutexColaHornos);
	  		 queue_push(colaParaHornear, platoActual);

	  		 sem_wait(semLog);
	  		 log_trace(logger,
	  		  "[ENTRADA/SALIDA] Ingresa a la cola para hornear un/a < %s > del pedido < %d >. Cantidad de platos en espera: %d",
				platoActual->nombrePlato, platoActual->idPedido, queue_size(colaParaHornear));
             sem_post(semLog);

	  		 sem_post(mutexColaHornos);
	  	  break;
	  	 }

	  }

	  sem_post(finalizarCicloBlockReady);

	  }
}





void hiloEntradaSalida(){
	int i;
	while(1){

		sem_wait(habilitarCicloEntradaSalida);

		if(list_size(platosHorneandose)<1){
			sem_wait(semLog);
			log_trace(logger, "[ENTRADA/SALIDA] Transcurre 1 ciclo sin platos en los hornos.");
			sem_post(semLog);
		} else {



		for(i=0; i<list_size(platosHorneandose);i++){
			pcb_plato* unPlatoHorneandose = list_get(platosHorneandose, i);
			if(unPlatoHorneandose->duracionBlock == 0){
				//el plato termino de hornearse, me fijo si le quedan mas pasos por hacer en su receta
				//sino lo mando a exit (raro)
				if(list_size(unPlatoHorneandose->pasosReceta)<1){
					unPlatoHorneandose->motivoBlock = NO_BLOCK;
					list_remove(platosHorneandose, i);

					//LOG DE ENUNCIADO!!!!1!!
					sem_wait(semLog);
					log_info(logger, "[ENTRADA/SALIDA] El horneado del plato < %s >, "
						"del pedido < %i > ha finalizado."
						,unPlatoHorneandose->nombrePlato, unPlatoHorneandose->idPedido);
					sem_post(semLog);

					agregarAExit(unPlatoHorneandose);
				} else {
					unPlatoHorneandose->motivoBlock = NO_BLOCK;
					list_remove(platosHorneandose, i);

					//LOG DE ENUNCIADO!!!!1!
					sem_wait(semLog);
					log_info(logger, "[ENTRADA/SALIDA] El horneado del plato < %s >, "
						"del pedido < %i > ha finalizado."
						,unPlatoHorneandose->nombrePlato, unPlatoHorneandose->idPedido);
					sem_post(semLog);
					agregarAReady(unPlatoHorneandose);
				}
			}
			unPlatoHorneandose->duracionBlock--;
		}

		}

		sem_wait(mutexColaHornos);
        //me fijo si puedo agregar un plato a hornear en este ciclo, de ser posible lo hago
		if(queue_size(colaParaHornear)<1){
			sem_wait(semLog);
			log_trace(logger, "[ENTRADA/SALIDA] Transcurre dicho ciclo sin platos esperando a ser horneados.");
			sem_post(semLog);
			sem_post(mutexColaHornos);
		} else {
			//hay platos en la cola para hornear, me fijo si tengo hornos disponibles
			if(list_size(platosHorneandose) < cantHornos){
				pcb_plato* platoAHornear = queue_pop(colaParaHornear);
				sem_post(mutexColaHornos);
				if(platoAHornear == NULL){
        //aca no deberia entrar jamas, si un plato fue agregado a la cola de hornos se presupone que esta creado bien
				} else {
				list_add(platosHorneandose, platoAHornear);

				sem_wait(semLog);
				//LOG DE ENUNCIADO!!!1!!!1
				log_info(logger, "[ENTRADA/SALIDA] Ingresa a hornearse un/a < %s >, del pedido < %i >."
									, platoAHornear->nombrePlato, platoAHornear->idPedido);
				sem_post(semLog);

				}
			} else {
				sem_post(mutexColaHornos);
			}


		}


         sem_post(finalizarCicloEntradaSalida);


	}
}


pcb_plato* obtenerSiguienteDeReadySinAfinidad(){
	int i;
	pcb_plato* elPlatoPlanificado;

	sem_wait(mutexListaReady);
	for(i=0; i<list_size(listaDeColasReady);i++){
		cola_ready* unaColaReadyConAfinidad = list_get(listaDeColasReady, i);
		if(queue_size(unaColaReadyConAfinidad->cola) == 0){
          //no hago nada, me fijo la proxima
		} else {
		elPlatoPlanificado = queue_pop(unaColaReadyConAfinidad->cola);
		sem_post(mutexListaReady);
		return elPlatoPlanificado;
		}

    }

	if(queue_size(colaReadySinAfinidad) == 0){
		sem_post(mutexListaReady);
		elPlatoPlanificado = NULL;
		return elPlatoPlanificado;
	} else {
		elPlatoPlanificado = queue_pop(colaReadySinAfinidad);
		sem_post(mutexListaReady);
		return elPlatoPlanificado;
	}

}


pcb_plato* obtenerSiguienteDeReadyConAfinidad(char* afinidad){
	int i;
	pcb_plato* elPlatoPlanificado;

	sem_wait(mutexListaReady);
	for(i=0; i<list_size(listaDeColasReady);i++){
		cola_ready* unaColaReadyConAfinidad = list_get(listaDeColasReady, i);
	  if(strcmp(unaColaReadyConAfinidad->afinidad, afinidad) == 0){
		  if(queue_size(unaColaReadyConAfinidad->cola) == 0){
		    //si la cola de mi afinidad no tiene nada, returneo NULL, no puedo ponerme cocinar.
			elPlatoPlanificado = NULL;
			sem_post(mutexListaReady);
			return elPlatoPlanificado;

		  	} else {

		  	elPlatoPlanificado = queue_pop(unaColaReadyConAfinidad->cola);
		  	sem_post(mutexListaReady);
		  	return elPlatoPlanificado;
		  	}

          }

	  }
	elPlatoPlanificado = NULL;
	return elPlatoPlanificado;

}









void iniciarSemaforos(){
	semId = malloc(sizeof(sem_t));
	semLog = malloc(sizeof(sem_t));
	semListaPedidos = malloc(sizeof(sem_t));

	sem_init(semId, 0, 1);
	sem_init(semLog, 0, 1);
	sem_init(semListaPedidos, 0, 1);
}

void inicializar_planificacion(){
	iniciarSemaforos();
	iniciarSemaforosPlanificacion();
	iniciarSemaforosCiclos();
	listaPedidos = list_create();
	crearColasPlanificacion();
	crearHilosPlanificacion();
}


// Inicializacion de semaforos necesarios
void iniciarSemaforosPlanificacion(){
	contadorPlatosEnNew = malloc(sizeof(sem_t));
	sem_init(contadorPlatosEnNew, 0, 0);

	mutexNew = malloc(sizeof(sem_t));
	sem_init(mutexNew, 0, 1);

	mutexListaReady = malloc(sizeof(sem_t));
	sem_init(mutexListaReady, 0, 1);

	mutexBlock = malloc(sizeof(sem_t));
	sem_init(mutexBlock, 0, 1);

	mutexColaHornos = malloc(sizeof(sem_t));
	sem_init(mutexColaHornos, 0, 1);

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

    	sem_wait(semLog);
		log_trace(logger, "[HCM] Habilitando ciclo: %i\n", numCiclo);
        sem_post(semLog);
		for(i = 0; i < cantCocineros; i++){
			signalSemaforoHabilitarCicloExec(i);
		}
		sem_post(habilitarCicloBlockReady);
        sem_post(habilitarCicloEntradaSalida);
		sleep(RETARDO_CICLO_CPU);
		sem_wait(semLog);
		log_trace(logger, "[HCM] Finalizando ciclo: %i", numCiclo);
		sem_post(semLog);
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

