/*
 * sindicato.c
 *
 *  Created on: 29 ago. 2020
 *      Author: utnso
 */

#include "sindicato.h"

void obtenerInputConsola(){

	char* lineaEntera = NULL;
	size_t longitud = 0;

	printf("Inserte un comando:\n");

	getline(&lineaEntera, &longitud, stdin);

	//printf("%s\n", lineaEntera);

	char** palabrasSeparadas = string_split( lineaEntera , " ");

	char* comandoIngresado = palabrasSeparadas[0];

	if ( strcmp(comandoIngresado, "CrearRestaurante") == 0 ){
		printf("El comando ingresado es: CrearRestaurante\n");

		int cantidadPalabras = cantidadDeElementosEnArray(palabrasSeparadas);

		printf("Cantidad Palabras: %i\n", cantidadPalabras);

		if (cantidadPalabras == 8){
			printf("La cantidad de argumentos es correcta.. obteniendo datos.\n");

			datosRestaurant restaurantNuevo;

			int cantidadCocineros = atoi(palabrasSeparadas[2]);

			char* hornosChar = string_substring_until(palabrasSeparadas[7], strlen(palabrasSeparadas[7]) - 1);

			int cantidadHornos = atoi(hornosChar);

			printf("Nombre: %s\n", palabrasSeparadas[1]);
			printf("CantidadCocineros: %i\n", cantidadCocineros);
			printf("Posicion: %s\n", palabrasSeparadas[3]);
			printf("AfinidadCocineros: %s\n", palabrasSeparadas[4]);
			printf("Platos: %s\n", palabrasSeparadas[5]);
			printf("Precios: %s\n", palabrasSeparadas[6]);
			printf("CantidadHornos: %i\n", cantidadHornos);

			restaurantNuevo.nombre = palabrasSeparadas[1];
			restaurantNuevo.cantCocineros = cantidadCocineros;
			restaurantNuevo.posicion = palabrasSeparadas[3];
			restaurantNuevo.afinidad = palabrasSeparadas[4];
			restaurantNuevo.platos = palabrasSeparadas[5];
			restaurantNuevo.preciosPlatos = palabrasSeparadas[6];
			restaurantNuevo.cantHornos = cantidadHornos;

		} else {
			printf("Sintaxis incorecta: El comando es de la forma:\nCrearRestaurante [NOMBRE] "
					"[CANTIDAD_COCINEROS] [POSICION] [AFINIDAD_COCINEROS] [PLATOS] [PRECIO_PLATOS] [CANTIDAD_HORNOS]\n");
		}

	} else if ( strcmp(comandoIngresado, "CrearReceta") == 0 ){
		printf("El comando ingresado es: CrearReceta\n");

		int cantidadPalabras = cantidadDeElementosEnArray(palabrasSeparadas);

		// TODO Terminar de procesar el comando crearReceta

		if (cantidadPalabras == 4){
			printf("La cantidad de argumentos es correcta.. obteniendo datos.");
		} else {
			printf("Sintaxis incorecta: El comando es de la forma:\nCrearReceta [NOMBRE] [PASOS] [TIEMPO_PASOS]\n");
		}

	} else {
		printf("El comando ingresado no es valido. Los comandos posibles son: CrearRestaurante, CrearReceta.\n");
		return;
	}
}

int cantidadDeElementosEnArray(char** array){
	int i = 0;
	while(array[i] != NULL && strcmp(array[i], "\n") != 0){
		i++;
	}
	return i;
}

int main(){
	printf("Comienzo sindicato\n");

	obtenerInputConsola();

	return 0;
}
