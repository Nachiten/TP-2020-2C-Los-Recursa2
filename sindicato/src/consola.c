/*
 * consola.c
 *
 *  Created on: 31 ago. 2020
 *      Author: Nachiten
 */

#include "consola.h"

void obtenerInputConsola(){

	while(1){

		char* lineaEntera = NULL;
		size_t longitud = 0;

		printf("Inserte un comando:\n");

		// Leer una linea completa de la consola
		getline(&lineaEntera, &longitud, stdin);

		// Recorto los espacios que puedan entrar (basura) y el \n del final
		string_trim(&lineaEntera);

		// Separo el comando ingresado en cada uno de los parametros
		char** palabrasSeparadas = string_split( lineaEntera , " ");

		// El nombre del comando es la primer palabra
		char* comandoIngresado = palabrasSeparadas[0];

		if ( strcmp(comandoIngresado, "CrearRestaurante") == 0 ){
			printf("El comando ingresado es: CrearRestaurante\n");

			int cantidadPalabras = cantidadDeElementosEnArray(palabrasSeparadas);

			//printf("Cantidad Palabras: %i\n", cantidadPalabras);

			if (cantidadPalabras == 8){
				printf("La cantidad de argumentos es correcta.. obteniendo datos.\n");

				// Convierto la tercer palabra a entero
				int cantidadCocineros = atoi(palabrasSeparadas[2]);

				if (cantidadCocineros <= 0){
					printf("ERROR | La cantidad de cocineros debe ser un numero entero mayor a 0\n");
					return;
				}

				// Convierto la ultima palabra a entero
				int cantidadHornos = atoi(palabrasSeparadas[7]);

				if (cantidadHornos <= 0){
					printf("ERROR | La cantidad de hornos debe ser un numero entero mayor a 0\n");
					return;
				}

				// Checkear que la cantidad de platos sea igual a la cantidad de precios platos
				if (!checkearLongitudArraysStrings(palabrasSeparadas[5], palabrasSeparadas[6])){
					printf("ERROR | La cantidad de platos y de preciosPlatos debe ser la misma\n");
					return;
				}

				// Struct con datos del restaurant
				datosRestaurant restaurantNuevo;

				// Inserto los datos obtenidos en struct
				restaurantNuevo.cantCocineros = cantidadCocineros;
				restaurantNuevo.posicion = palabrasSeparadas[3];
				restaurantNuevo.afinidad = palabrasSeparadas[4];
				restaurantNuevo.platos = palabrasSeparadas[5];
				restaurantNuevo.preciosPlatos = palabrasSeparadas[6];
				restaurantNuevo.cantHornos = cantidadHornos;

				//printearDatosRestaurant(restaurantNuevo);

				// Si el restaurant no existe
				if (!existeRestaurant(palabrasSeparadas[1])){
					// Crear un nuevo restaurant vacio con el nombre
					crearRestaurant(palabrasSeparadas[1], restaurantNuevo);
				} else {
					printf("El restaurant que se quiere crear ya existe. Aborting.\n");
				}

			} else {
				printf("ERROR | Sintaxis incorecta: El comando es de la forma:\nCrearRestaurante [NOMBRE] "
						"[CANTIDAD_COCINEROS] [POSICION] [AFINIDAD_COCINEROS] [PLATOS] [PRECIO_PLATOS] [CANTIDAD_HORNOS]\n");
			}

		} else if ( strcmp(comandoIngresado, "CrearReceta") == 0 ){
			printf("El comando ingresado es: CrearReceta\n");

			int cantidadPalabras = cantidadDeElementosEnArray(palabrasSeparadas);

			if (cantidadPalabras == 4){
				printf("La cantidad de argumentos es correcta.. obteniendo datos.\n");


				// Checkear que la cantidad de pasos sea igual a la cantidad de tiemposPasos
				if (!checkearLongitudArraysStrings(palabrasSeparadas[2], palabrasSeparadas[3])){
					printf("ERROR | La cantidad de pasos y tiemposPasos debe ser la misma\n");
					return;
				}

				// Struct con datos de los pasos
				datosReceta unaReceta;

				unaReceta.pasos = palabrasSeparadas[2];
				unaReceta.tiempoPasos = palabrasSeparadas[3];

				// Si no existe receta con ese nombre
				if (!existeReceta(palabrasSeparadas[1])){
					// Crear nueva receta con ese nombre
					crearReceta(palabrasSeparadas[1], unaReceta);
				} else {
					printf("La receta que se quiere crear ya existe. Aborting.\n");
				}

			} else {
				printf("ERROR | Sintaxis incorecta: El comando es de la forma:\nCrearReceta [NOMBRE] [PASOS] [TIEMPO_PASOS]\n");
			}

		} else {
			printf("ERROR | El comando ingresado no es valido. Los comandos posibles son: CrearRestaurante, CrearReceta.\n");
			return;
		}

		// Libero las palabras separadas
		freeDeArray(palabrasSeparadas);
	}
}

void printearDatosRestaurant(datosRestaurant unRestaurant){
	printf("CantidadCocineros: %i\n", unRestaurant.cantCocineros);
	printf("Posicion: %s\n",          unRestaurant.posicion);
	printf("AfinidadCocineros: %s\n", unRestaurant.afinidad);
	printf("Platos: %s\n",            unRestaurant.platos);
	printf("Precios: %s\n",           unRestaurant.preciosPlatos);
	printf("CantidadHornos: %i\n",    unRestaurant.cantHornos);
}

void printearDatosPasos(datosReceta unosPasos){
	printf("Pasos: %s\n", unosPasos.pasos);
	printf("Tiempo Pasos: %s\n", unosPasos.tiempoPasos);
}

// Checkear que ambos strings de la forma [a,b,c] tengan la misma cantidad de elementos
int checkearLongitudArraysStrings(char* array1, char* array2){

	// Separo los strings en un array de sus elementos
	char** elementos1 = string_get_string_as_array(array1);
	char** elementos2 = string_get_string_as_array(array2);

	int retorno = 0;

	if (cantidadDeElementosEnArray(elementos1) == cantidadDeElementosEnArray(elementos2)){
		retorno = 1;
	}

	freeDeArray(elementos1);
	freeDeArray(elementos2);

	return retorno;
}
