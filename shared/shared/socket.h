#ifndef SHARED_SOCKET_H_
#define SHARED_SOCKET_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<readline/readline.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include <errno.h>
#include"estructuras.h"
#include"utilidades.h"


int32_t reservarSocket(char* miPuerto); //me reserva un socket (pasarle la key que sacamos de nuestro config)
int32_t crearSocketServidor(char* ip, char* puerto);
int32_t establecer_conexion(char* ip, char* puerto);//se le da una IP y un PUERTO y establece la conexion
//dice el resultado de intentar conectarse a un modulo
void resultado_de_conexion(int32_t socket, t_log* logger, char* modulo); //enviarle el socket al que se intenta conectar, el logger y nombre del modulo al que intentamos conectar.
void cerrar_conexion(int32_t socket); //se le da el socket y lo libera
void mandar_mensaje(void* mensaje, codigo_operacion tipoMensaje,  int32_t socket_cliente); //se le da el mensaje, el tipo de mensaje que vamos a mandar y el socket que le dice donde mandarlo
void recibir_mensaje(void* estructura, codigo_operacion tipoMensaje, int32_t socket_cliente); //recibe un mensaje
void eliminar_paquete(t_paquete* paquete); //libera la memoria utilizada para manejar los paquetes

//FUNCIONES SERIALIZAR

//se le da el paquete, el "algo" que vamos a mandar, tipo de mensaje y un size para que "anote" cuanto pesa el paquete despues de serializarlo.
void* serializar_paquete(t_paquete* paquete, void* mensaje, codigo_operacion tipoMensaje, uint32_t *size_serializado);//prepara paquete y deja listo para mandar

//estas funciones las necesita serializar_paquete para saber que empaqueta
uint32_t serializar_paquete_seleccionar_restaurante(t_paquete* paquete, seleccionar_restaurante* estructura);
uint32_t serializar_paquete_obtener_restaurante(t_paquete* paquete, obtener_restaurante* estructura);
uint32_t serializar_paquete_consultar_platos(t_paquete* paquete, consultar_platos* estructura);
uint32_t serializar_paquete_guardar_plato(t_paquete* paquete, guardar_plato* estructura);
uint32_t serializar_paquete_aniadir_plato(t_paquete* paquete, a_plato* estructura);
uint32_t serializar_paquete_plato_listo(t_paquete* paquete, plato_listo* estructura);
uint32_t serializar_paquete_guardar_pedido(t_paquete* paquete, guardar_pedido* estructura);
uint32_t serializar_paquete_consultar_pedido(t_paquete* paquete, consultar_pedido* estructura);
uint32_t serializar_paquete_obtener_receta(t_paquete* paquete, obtener_receta* estructura);
uint32_t serializar_paquete_handshake(t_paquete* paquete, handshake* estructura);
uint32_t serializar_paquete_respuesta_consultar_restaurantes(t_paquete* paquete, respuesta_consultar_restaurantes* estructura);
uint32_t serializar_paquete_respuesta_obtener_restaurante(t_paquete* paquete, respuesta_obtener_restaurante* estructura);
uint32_t serializar_paquete_respuesta_consultar_platos(t_paquete* paquete, respuesta_consultar_platos* estructura);
uint32_t serializar_paquete_respuesta_crear_pedido(t_paquete* paquete, respuesta_crear_pedido* estructura);
uint32_t serializar_paquete_respuesta_consultar_pedido(t_paquete* paquete, respuesta_consultar_pedido* estructura);
uint32_t serializar_paquete_respuesta_obtener_pedido(t_paquete* paquete, respuesta_obtener_pedido* estructura);
uint32_t serializar_paquete_respuesta_obtener_receta(t_paquete* paquete, respuesta_obtener_receta* estructura);
uint32_t serializar_paquete_ok_fail(t_paquete* paquete, respuesta_ok_error* estructura);
uint32_t serializar_paquete_crear_pedido(t_paquete* paquete, crear_pedido* estructura);
uint32_t serializar_paquete_agregar_restaurante(t_paquete* paquete, agregar_restaurante* estructura);

//FUNCIONES DESSERIALIZAR
//void desserializar_mensaje (void* estructura, codigo_operacion tipoMensaje, int32_t socket_cliente); //esto vuela a la mierda a menos que se decida lo contrario

//estas funciones las necesita desserializar para saber como manejar la info que le llega
void desserializar_seleccionar_restaurante(seleccionar_restaurante* estructura, int32_t socket_cliente);
void desserializar_obtener_restaurante(obtener_restaurante* estructura, int32_t socket_cliente);
void desserializar_crear_pedido(crear_pedido* estructura, int32_t socket_cliente);
void desserializar_guardar_plato(guardar_plato* estructura, int32_t socket_cliente);
void desserializar_aniadir_plato(a_plato* estructura, int32_t socket_cliente);
void desserializar_plato_listo(plato_listo* estructura, int32_t socket_cliente);
void desserializar_guardar_pedido(guardar_pedido* estructura, int32_t socket_cliente);
void desserializar_consultar_pedido(consultar_pedido* estructura, int32_t socket_cliente);
void desserializar_obtener_receta(obtener_receta* estructura, int32_t socket_cliente);
void desserializar_handshake(handshake* estructura, int32_t socket_cliente);
void desserializar_respuesta_consultar_restaurantes(respuesta_consultar_restaurantes* estructura, int32_t socket_cliente);
void desserializar_respuesta_obtener_restaurante(respuesta_obtener_restaurante* estructura, int32_t socket_cliente);
void desserializar_respuesta_consultar_platos(respuesta_consultar_platos* estructura, int32_t socket_cliente);
void desserializar_respuesta_crear_pedido(respuesta_crear_pedido* estructura, int32_t socket_cliente);
void desserializar_respuesta_consultar_pedido(respuesta_consultar_pedido* estructura, int32_t socket_cliente);
void desserializar_respuesta_obtener_pedido(respuesta_obtener_pedido* estructura, int32_t socket_cliente);
void desserializar_respuesta_obtener_receta(respuesta_obtener_receta* estructura, int32_t socket_cliente);
void desserializar_ok_fail(respuesta_ok_error* estructura, int32_t socket_cliente);
void desserializar_consultar_platos(consultar_platos* estructura, int32_t socket_cliente);
void desserializar_agregar_restaurante(agregar_restaurante* estructura, int32_t socket_cliente);
#endif /* SHARED_SOCKET_H_ */
