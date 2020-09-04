#include "funcionesCliente.h"
#include "cliente.h"

//devuelve el valor en entero en base al string adoptado en consola para el switch (modulo CLIENTE)

uint32_t valor_para_switch_case(char* input) {

	uint32_t switcher;
	if (strcmp(input,"CONSULTAR_RESTAURANTES") == 0)
	{
		switcher = CONSULTAR_RESTAURANTES;
	}
	if (strcmp(input,"SELECCIONAR_RESTAURANTE") == 0)
	{
		switcher = SELECCIONAR_RESTAURANTE;
	}
	if (strcmp(input,"OBTENER_RESTAURANTE") == 0)
	{
		switcher = OBTENER_RESTAURANTE;
	}
	if (strcmp(input,"CONSULTAR_PLATOS") == 0)
	{
		switcher = CONSULTAR_PLATOS;
	}
	if (strcmp(input,"GUARDAR_PLATO") == 0)
	{
		switcher = GUARDAR_PLATO;
	}
	if (strcmp(input,"AGREGAR_PLATO") == 0)
	{
		switcher = A_PLATO;
	}
	if (strcmp(input,"PLATO_LISTO") == 0)
	{
		switcher = PLATO_LISTO;
	}
	if (strcmp(input,"CREAR_PEDIDO") == 0)
	{
		switcher = CREAR_PEDIDO;
	}
	if (strcmp(input,"GUARDAR_PEDIDO") == 0)
	{
		switcher = GUARDAR_PEDIDO;
	}
	if (strcmp(input,"CONFIRMAR_PEDIDO") == 0)
	{
		switcher = CONFIRMAR_PEDIDO;
	}
	if (strcmp(input,"CONSULTAR_PEDIDO") == 0)
	{
		switcher = CONSULTAR_PEDIDO;
	}
	if (strcmp(input,"OBTENER_PEDIDO") == 0)
	{
		switcher = OBTENER_PEDIDO;
	}
	if (strcmp(input,"FINALIZAR_PEDIDO") == 0)
	{
		switcher = FINALIZAR_PEDIDO;
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

void obtenerInputConsolaCliente(){

	char* lineaEntera = NULL;
	size_t longitud = 0;
	uint32_t switcher;

	printf("Inserte un comando:\n");


	getline(&lineaEntera, &longitud, stdin);

	string_trim(&lineaEntera);

	char** palabrasSeparadas = string_split(lineaEntera , " ");

	// El nombre del comando es la primer palabra (POR EL MOMENTO CON GUIONES_BAJOS) -> EJ: CONSULTAR_RESTAURANTES
    char* comandoIngresado = palabrasSeparadas[0];

    printf("el comando que pediste fue: %s \n", comandoIngresado);

    switcher = valor_para_switch_case(comandoIngresado);


    freeDeArray(palabrasSeparadas);



}
