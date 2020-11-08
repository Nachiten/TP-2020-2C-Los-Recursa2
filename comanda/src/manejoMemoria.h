#ifndef SRC_MANEJOMEMORIA_H_
#define SRC_MANEJOMEMORIA_H_

#include "estructurasCoMAnda.h"
#include "comanda.h"
#include "semaphore.h"
#include "shared/estructuras.h"

//cosas que necesita coMAnda para trabajar
void* MEMORIA_PRINCIPAL;
void* AREA_DE_SWAP;
uint32_t numero_de_victima;

sem_t* semaforoNumeroVictima;
sem_t* semaforoTocarListaPedidosTodosLosRestaurantes;
sem_t* semaforoTocarListaEspaciosEnSWAP;
sem_t* semaforoTocarListaEspaciosEnMP;
sem_t* semaforoTocarMP;
sem_t* semaforoTocarSWAP;

tablas_segmentos_restaurantes* lista_de_pedidos_de_todos_los_restaurantes; //aca empiezan las lista de segmentos que tiene CoMAnda para cada restaurante
espacio* lista_de_espacios_en_SWAP; //aca comienza la lista de los espacios disponibles/ocupados en area de SWAP
espacio* lista_de_espacios_en_MP; //aca comienza la lista de los espacios disponibles/ocupados en area de SWAP

//funciones de memoria****************************************************************************************************************************
//esta porqueria lo que hace es la inicializacion original para tener tablas de pedidos para cda restaurante
void inicializar_lista_de_tablas_de_segmentos_de_restaurantes(tablas_segmentos_restaurantes* listas_de_pedidos);
void inicializar_tabla_de_segmentos(segmentos* laTablaDeSegmentos);
void inicializar_tabla_de_paginas(tabla_paginas* laTablaDePaginas);
void inicializar_lista_de_espacios(espacio* listaDeEspacios, uint32_t TAMANIO_AREA); //utilizada para inicializar listas de los espacios disponibles de MP y SWAP

//crea una nueva tabla y devuelve puntero a la tabla creada
tablas_segmentos_restaurantes* crear_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//esto crea un nuevo pedido (segumento = pedido)
uint32_t crearSegmento(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido);
//crea una nueva pagina y devuelve el puntero a la pagina creada
tabla_paginas* crearPagina(tabla_paginas* tablaDePlatosDelPedido, char* nombrePlato, uint32_t cantidadPlatos);
void crearNuevoEspacio(espacio* espacio); //para preparar los espacios que puede haber en SWAP/MP
int32_t buscarPrimerEspacioLibre(espacio* listaDeEspacios); //devuelve el numero del espacio encontrado, o -1 si no se encontro
void marcarEspacioComoOcupado(espacio* listaDeEspacios, uint32_t numeroDeEspacioElegido);
void marcarEspacioComoLibre(espacio* listaDeEspacios, uint32_t numeroDeEspacioElegido);

//crea y asigna un nuevo numero de victima a la pagina
void asignarNumeroDeVictima(uint32_t* miNumeroDeVictima);

//devuelve la lista de pedidos del restaurante solicitado, si no existe, la crea
tablas_segmentos_restaurantes* selector_de_tabla_de_pedidos(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante, uint32_t negarCreacion);
//dada la tabla de segmentos de un restaurante y el Nº de segmento, devuelve puntero al pedido
segmentos* selectordePedidoDeRestaurante(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento);
//busca si ya existe la lista de pedidos de X restaurante
uint32_t buscar_tabla_de_segmentos_de_restaurante(tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, char* nombreDeRestaurante);
//para ver si ya existe el pedido, y devolver FAIL, o si no (Existe = 1, no existe = 0)
uint32_t verificarExistenciaDePedido (tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t idDelPedido);
//devuelve el numero de segmento si lo encontro, o 0 si no existe
uint32_t buscar_segmento_de_pedido(tablas_segmentos_restaurantes* laTablaDeSegmentos, uint32_t idDelPedido);

//toma todos los platos de un pedido y se asegura que esten cargados en MP (NO los carga en SWAP, ya deberían estar ahi)
void cargarPaginasEnMP(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento);

//se trae los datos de MP a las paginas del pedido ToDO revisar si en realidad hay que traerlas desde SWAP
void actualizarTodosLosPlatosConDatosDeMP(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento);

//busco si existe el plato con el nombre provisto en la lista de platos del pedido seleccionado (Existe = 1, no existe = 0)
uint32_t verificarExistenciaDePlato(segmentos* segmentoSeleccionado, char* nombrePlato);
//esto agrega un plato (o aumenta su cantidad) a un pedido (segmento) ya existente. Devuelve puntero al plato editado/creado
tabla_paginas* agregarPlatoAPedido(tablas_segmentos_restaurantes* tablaDePedidosDelRestaurante, uint32_t numeroDeSegmento, char* nombrePlato, uint32_t cantidadPlatos);

//ponemos en area de SWAP los datos de una pagina
void agregar_pagina_a_swap(tabla_paginas* tablaDePlatosDelPedido, uint32_t posicionInicial);

//copiamos la info que tenemos en el area de SWAP a MEMORIA PRINCIPAL
void mover_pagina_a_memoriaPrincipal(tabla_paginas* tablaDePlatosDelPedido, uint32_t posicionInicialDeSWAP, uint32_t posicionInicialDeMEMORIA);

//actualiza los datos de una Pagina de SWAP con la data que hay en MP
void actualizar_pagina_en_SWAP (tabla_paginas* laPagina);

//toma los datos de MP y los devuelve a su PAGINA
void tomar_datos_de_MP(tabla_paginas* platoDelPedido);

//toma los datos de SWAP y los devuelve a su PAGINA
void tomar_datos_de_SWAP(tabla_paginas* platoDelPedido);

// Grim Reaper
void algoritmo_de_reemplazo(char* ALGOR_REEMPLAZO, tablas_segmentos_restaurantes* lasListasDePedidosDeRestaurantes, espacio* lista_de_espacios_en_MP);

//dada una pagina, borra los datos del plato, para cumplir con enunciado
void borrar_datos_del_plato(tabla_paginas* platoDelPedido);

//dado un PEDIDO (no la ID, el segmento), borra todos los datos de sus paginas
void borrar_datos_de_todos_los_platos_del_pedido(segmentos* tablaDePedidos);

//prepara la respuesta de OBTENER_PEDIDO con el formato acordado
void preparar_datos_de_platos_con_formato_de_obtener_pedido(segmentos* tablaDePedidos, respuesta_obtener_pedido* resultadoObtenerPedido);

#endif /* SRC_MANEJOMEMORIA_H_ */
