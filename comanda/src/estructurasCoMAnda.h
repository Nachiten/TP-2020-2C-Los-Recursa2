#ifndef SRC_ESTRUCTURASCOMANDA_H_
#define SRC_ESTRUCTURASCOMANDA_H_

#include<stdint.h> //WTF?!?!?!?

typedef struct tabla_paginas
{
	uint32_t numero_de_pagina;
	uint32_t numero_de_victima;

	uint32_t cantidadPedidaComida;
	uint32_t cantidadComidaPreparada;
	char* nombreDeMorfi; //allocar siempre con 24 bytes

	uint32_t cargadoEnSWAP;
	int32_t posicionInicialEnSWAP;
	uint32_t cargadoEnMEMORIA;
	int32_t numeroDePagina;

	struct tabla_paginas* anter_pagina;
	struct tabla_paginas* sig_pagina;
}tabla_paginas;

/*---->NO TOCAR!!!!<---- el "tabla_segmentos" al lado del "typedef struct", si se saca, aparece un error extraño donde la estructura
no se reconoce a si misma cuando queremos sus campos anterior y siguiente apunten a otras instancias de la misma estructura */
typedef struct tabla_segmentos
{
	uint32_t numero_de_segmento;
	int32_t id_Pedido;

	tabla_paginas* mi_tabla; //cada segmento tiene su tabla de paginas

	struct tabla_segmentos* anter_segmento;
	struct tabla_segmentos* sig_segmento;
}segmentos;

typedef struct tablas_segmentos_restaurantes
{
	char* nombreRestaurante;

	uint32_t cantidadDeSegmentos;
	segmentos* miTablaDePedidos;

	struct tablas_segmentos_restaurantes* anter_lista;
	struct tablas_segmentos_restaurantes* sig_lista;
}tablas_segmentos_restaurantes;

typedef struct espacioEnSWAP
{
	uint32_t numeroDeEspacio;
	uint32_t espacioOcupado;

	struct espacioEnSWAP* anter_espacio;
	struct espacioEnSWAP* sig_espacio;
}espacioEnSWAP;

#endif /* SRC_ESTRUCTURASCOMANDA_H_ */
