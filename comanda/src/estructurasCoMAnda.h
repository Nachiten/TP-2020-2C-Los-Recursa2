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

	struct tabla_segmentos* anter_pagina;
	struct tabla_segmentos* sig_pagina;
}tabla_paginas;


/*---->NO TOCAR!!!!<---- el "tabla_segmentos" al lado del "typedef struct", si se saca, aparece un error extraño donde la estructura
no se reconoce a si misma cuando queremos sus campos anterior y siguiente apunten a otras instancias de la misma estructura */
typedef struct tabla_segmentos
{
	uint32_t numero_de_segmento;
	char* nombreRestaurante;

	uint32_t numero_de_victima; //me parece q aca no va
	int32_t ID_MENSAJE_GUARDADO; //esto me suena que esta de mas


	tabla_paginas* mi_tabla; //cada segmento tiene su tabla de paginas

	struct tabla_segmentos* anter_segmento;
	struct tabla_segmentos* sig_segmento;
}segmentos;



#endif /* SRC_ESTRUCTURASCOMANDA_H_ */
