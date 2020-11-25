#include "funcionesCliente.h"

//devuelve el valor en entero en base al string adoptado en consola para el switch (modulo CLIENTE)

uint32_t valor_para_switch_case(char* input) {

	uint32_t switcher;

	//CONSULTAR_RESTAURANTES
	if (strcmp(input,"CONSULTAR_RESTAURANTES") == 0)
	{
		switcher = CONSULTAR_RESTAURANTES;
	}

	//SELECCIONAR_RESTAURANTE [nombreRESTAURANTE]
	if (strcmp(input,"SELECCIONAR_RESTAURANTE") == 0)
	{
		switcher = SELECCIONAR_RESTAURANTE;
	}

	//OBTENER_RESTAURANTE [nombreRESTAURANTE]
	if (strcmp(input,"OBTENER_RESTAURANTE") == 0)
	{
		switcher = OBTENER_RESTAURANTE;
	}

	//CONSULTAR_PLATOS [nombreRESTAURANTE]
	if (strcmp(input,"CONSULTAR_PLATOS") == 0)
	{
		switcher = CONSULTAR_PLATOS;
	}

	//GUARDAR_PLATO [idPEDIDO]
	if (strcmp(input,"GUARDAR_PLATO") == 0)
	{
		switcher = GUARDAR_PLATO;
	}

	//AGREGAR_PLATO [idPEDIDO] [nombreRESTAURANTE] [nombrePLATO] [cantidadPLATO]
	if (strcmp(input,"AGREGAR_PLATO") == 0)
	{
		switcher = A_PLATO;
	}

	//PLATO_LISTO [idPEDIDO] [nombreRESTAURANTE] [nombrePLATO]
	if (strcmp(input,"PLATO_LISTO") == 0)
	{
		switcher = PLATO_LISTO;
	}

	//CREAR_PEDIDO (se hace luego del SELECCIONAR_RESTAURANTE si hablas con app o bien al restaurante de una)
	if (strcmp(input,"CREAR_PEDIDO") == 0)
	{
		switcher = CREAR_PEDIDO;
	}

	//GUARDAR_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"GUARDAR_PEDIDO") == 0)
	{
		switcher = GUARDAR_PEDIDO;
	}

	//CONFIRMAR_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"CONFIRMAR_PEDIDO") == 0)
	{
		switcher = CONFIRMAR_PEDIDO;
	}

	//CONSULTAR_PEDIDO [idPEDIDO]
	if (strcmp(input,"CONSULTAR_PEDIDO") == 0)
	{
		switcher = CONSULTAR_PEDIDO;
	}

	//OBTENER_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"OBTENER_PEDIDO") == 0)
	{
		switcher = OBTENER_PEDIDO;
	}

	//FINALIZAR_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"FINALIZAR_PEDIDO") == 0)
	{
		switcher = FINALIZAR_PEDIDO;
	}

	//TERMINAR_PEDIDO [idPEDIDO] [nombreRESTAURANTE]
	if (strcmp(input,"TERMINAR_PEDIDO") == 0)
	{
		switcher = TERMINAR_PEDIDO;
	}

	//OBTENER_RECETA [nombreRECETA]
	if (strcmp(input,"OBTENER_RECETA") == 0)
	{
		switcher = OBTENER_RECETA;
	}

	return switcher;
}

// Cuenta la cantidad de elementos en un array
int cantidadDeElementosEnArray(char** array){
	int i = 0;
	while(array[i] != NULL && strcmp(array[i], "\n") != 0){
		i++;
	}
	return i;
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

