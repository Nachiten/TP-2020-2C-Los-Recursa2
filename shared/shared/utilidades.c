#include "utilidades.h"

uint32_t elevar_a(uint32_t a, uint32_t b)
{
	uint32_t resultado = a;
	uint32_t iterador = b;

	if(b == 0)
	{
		resultado = 1;
		return resultado;
	}
	else
		{
		while(iterador > 1)
		{
			resultado = resultado*a;
			iterador--;
		}
	}
	return resultado;
}

uint32_t cambia_a_int(char* enString)
{
	uint32_t enNumero = 0;
	uint32_t longitud = strlen(enString);
	int32_t iterador = longitud-1;
	uint32_t elevado = 0;

	while(iterador >=0)
	{
		elevado = elevar_a(10, ((longitud - 1) - iterador));
		enNumero = enNumero + ((enString[iterador] - 48) * elevado);
		iterador--;
	}

	return enNumero;
}

void bytesRecibidos (int32_t cantidad)
{
	if(cantidad == 0)
	{
		puts("Conexión finalizada (0).\n");
	}

	if(cantidad == -1)
	{
		puts("La conexión está caida (-1).\n");
	}

	if(cantidad > 0)
	{
		printf("Recibí %d bytes sin crashear (Todavía).\n",cantidad);
	}
}

void bytesEnviados (int32_t cantidad)
{
	if(cantidad == -1)
	{
		puts("No se pudo enviar el mensaje, la conexión no está activa (-1).\n");
	}

	if(cantidad > 0)
	{
		printf("Envié %d bytes sin crashear (Todavía).\n",cantidad);
	}
}

// Hace free de cada uno de los elementos y de la lista misma
void destuirListaElementos(t_list* unaLista){
	list_clean_and_destroy_elements(unaLista, free);
	list_destroy(unaLista);
}
