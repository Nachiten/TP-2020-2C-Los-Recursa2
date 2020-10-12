#ifndef SRC_MANEJOMEMORIA_H_
#define SRC_MANEJOMEMORIA_H_

#include "estructurasCoMAnda.h"
#include "comanda.h"
#include "semaphore.h"

//cosas que necesita coMAnda para trabajar
void* MEMORIA_PRINCIPAL;
void* AREA_DE_SWAP;
uint32_t numero_de_victima;

sem_t* semaforoNumeroVictima;
sem_t* semaforoTocarListaPedidosTodosLosRestaurantes;
sem_t* semaforoTocarListaEspaciosEnSWAP;

tablas_segmentos_restaurantes* lista_de_pedidos_de_todos_los_restaurantes; //aca empiezan las lista de segmentos que tiene CoMAnda para cada restaurante
espacioEnSWAP* lista_de_espacios_en_SWAP; //aca comienza la lista de los espacios disponibles/ocupados en area de SWAP

//funciones de memoria****************************************************************************************************************************
//esta porqueria lo que hace es la inicializacion original para tener tablas de pedidos para cda restaurante
void inicializar_lista_de_tablas_de_segmentos_de_restaurantes(tablas_segmentos_restaurantes* listas_de_pedidos);
void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos);
void inicializar_tabla_de_paginas(tabla_paginas* laTablaDePaginas);
void inicializar_lista_de_espacios_en_SWAP(espacioEnSWAP* espacio, uint32_t TAMANIO_AREA_DE_SWAP);

//crea una nueva tabla y devuelve puntero a la tabla creada
tablas_segmentos_restaurantes* crear_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//esto crea un nuevo pedido (segumento = pedido)
uint32_t crearSegmento(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido);
//crea una nueva pagina y devuelve el puntero a la pagina creada
tabla_paginas* crearPagina(tabla_paginas* tablaDePlatosDelPedido, char* nombrePlato, uint32_t cantidadPlatos);
void crearNuevoEspacioEnSWAP(espacioEnSWAP* espacio); //para preparar los espacios que puede haber en SWAP
int32_t buscarPrimerEspacioLibreEnSWAP(espacioEnSWAP* listaDeEspacios); //devuelve el numero del espacio encontrado, o -1 si no se encontro

//crea y asigna un nuevo numero de victima a la pagina
void asignarNumeroDeVictima(uint32_t* miNumeroDeVictima);

//devuelve la lista de pedidos del restaurante solicitado, si no existe, la crea
tablas_segmentos_restaurantes* selector_de_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante, uint32_t negarCreacion);
//busca si ya existe la lista de pedidos de X restaurante
uint32_t buscar_tabla_de_segmentos_de_restaurante(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//para ver si ya existe el pedido, y devolver FAIL, o si no (Existe = 1, no existe = 0)
uint32_t verificarExistenciaDePedido (tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido);

uint32_t buscar_segmento_de_pedido(tablas_segmentos_restaurantes* laTablaDeSegmentos, uint32_t idDelPedido);

//busco si existe el plato con el nombre provisto en la lista de platos del pedido seleccionado (Existe = 1, no existe = 0)
uint32_t verificarExistenciaDePlato(segmentos* segmentoSeleccionado, char* nombrePlato);
//esto agrega un plato (o aumenta su cantidad) a un pedido (segmento) ya existente. Devuelve puntero al plato editado/creado
tabla_paginas* agregarPlatoAPedido(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento, char* nombrePlato, uint32_t cantidadPlatos);

//ponemos en area de SWAP los datos de una pagina
void agregar_pagina_a_swap(tabla_paginas* tablaDePlatosDelPedido, uint32_t posicionInicial);

//copiamos la info que tenemos en el area de SWAP a MEMORIA PRINCIPAL
void mover_pagina_a_memoriaPrincipal(tabla_paginas* tablaDePlatosDelPedido, uint32_t posicionInicialDeSWAP, uint32_t posicionInicialDeMEMORIA);

#endif /* SRC_MANEJOMEMORIA_H_ */
