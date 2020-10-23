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
void destruirListaYElementos(t_list* unaLista){
	list_clean_and_destroy_elements(unaLista, free);
	list_destroy(unaLista);
}

char* resultadoDeRespuesta(uint32_t resultado)
{
	if(resultado == 1)
	{
		return "exitoso";
	}

	else
	{
		return "fallido";
	}
}

// Cuenta la cantidad de elementos en un array
// Si no hay elementos retorna 0
int cantidadDeElementosEnArray(char** array){
	int i = 0;
	while(array[i] != NULL){
		i++;
	}
	return i;
}

void los_recv_repetitivos(int32_t socket_conexion_establecida, uint32_t *exito, int32_t *sizeAAllocar)
{
	int32_t bytesRecibidosCodOP = 0;
	int32_t recibidosSize = 0;
	codigo_operacion cod_op;

	//recibo codigo de op
	bytesRecibidosCodOP = recv(socket_conexion_establecida, &cod_op, sizeof(cod_op), MSG_WAITALL);
	bytesRecibidos(bytesRecibidosCodOP);

	//si se cayo la conexion, basicamente no hacemos hada
	if(bytesRecibidosCodOP < 1)
	{
		cod_op = 0;
		sizeAAllocar = 0;
	}

	//si la conexion NO se cayo, intento recibir lo que sigue
	else
	{
		//recibo tamaño de lo que sigue
		recibidosSize = recv(socket_conexion_establecida, sizeAAllocar, sizeof(int32_t), MSG_WAITALL);
		bytesRecibidos(recibidosSize);

		//si se cayo la conexion, no se hace nada con esto
		if(recibidosSize < 1)
		{
			*exito = 0;
			cod_op = 0;
			sizeAAllocar = 0;
		}

	}

	if(cod_op != 0)
	{
		*exito = 1;
		printf("Tamaño del Payload: %i.\n", *sizeAAllocar);
	}
}
