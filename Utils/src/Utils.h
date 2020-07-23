#ifndef UTILS_H_
#define UTILS_H_


#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<signal.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<fcntl.h>

sem_t semaforo;
typedef enum {
	GET_POKEMON = 1,
	CATCH_POKEMON= 2,
	LOCALIZED_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	APPEARED_POKEMON = 5,
	NEW_POKEMON = 6,
	SUBSCRIPTION = 7,//Suscripcion a cola
	ACK = 8			// gamecard
}op_code;

typedef struct {
	uint32_t socket;
	uint32_t tiempo_suscripcion;
	op_code cola_a_suscribir;
	uint32_t id_proceso;
} t_suscripcion;

typedef struct {
  uint32_t id_mensaje;
  op_code  tipo_mensaje;
  uint32_t    id_proceso;
} t_ack;

typedef struct {
	uint32_t id_mensaje;
	char* pokemon;
	uint32_t posicion[2];
	uint32_t cantidad;
} t_new_pokemon;

typedef struct {
	uint32_t id_mensaje;
	char* pokemon;
	uint32_t posicion[2];
} t_catch_pokemon;

typedef struct {
	uint32_t id_mensaje;
	char* pokemon;
} t_get_pokemon;

typedef struct {
	uint32_t id_mensaje;
	uint32_t id_mensaje_correlativo;
	char* pokemon;
	uint32_t posicion[2];
} t_appeared_pokemon;

typedef struct {
	uint32_t id_mensaje;
	uint32_t id_mensaje_correlativo;
	uint32_t resultado;
} t_caught_pokemon;


typedef struct {
	uint32_t id_mensaje;
	uint32_t id_mensaje_correlativo;
	char* pokemon;
	uint32_t tamanio_lista;
	t_list* posiciones;
} t_localized_pokemon;

struct sockaddr_in direccionServidor;
pthread_t thread;
t_log* logger;
uint32_t id_mensaje_univoco;

void* recibir_buffer(uint32_t*, uint32_t); /// este no esta definido en utils.c

// iniciar
void iniciar_logger(char* file, char* program_name);

//client
//void* serializar_paquete(t_paquete* paquete, uint32_t* bytes);
uint32_t crear_conexion(char* ip, char* puerto);


//server
//void eliminar_paquete(t_paquete* paquete);
void iniciar_servidor(char *IP, char *PUERTO);
void esperar_cliente(uint32_t);
void serve_client(uint32_t socket);
void process_request(uint32_t cod_op, uint32_t cliente_fd);
uint32_t recibir_operacion(uint32_t);
void* recibir_paquete(uint32_t,uint32_t*,op_code*);

//mensaje
void enviar_mensaje(uint32_t cod_op, void* mensaje, uint32_t socket_cliente, uint32_t size_mensaje);
void* recibir_mensaje(uint32_t socket_cliente, uint32_t* size);
void devolver_mensaje(uint32_t cod_op, uint32_t size, void* payload, uint32_t socket_cliente);
void reenviar_mensaje(uint32_t cod_op, uint32_t size, void* payload, uint32_t socket_cliente);
void liberar_mensaje_get(t_get_pokemon* mensaje);
void liberar_mensaje_catch(t_catch_pokemon* mensaje);
void liberar_mensaje_localized(t_localized_pokemon* mensaje);
void liberar_mensaje_caught(t_caught_pokemon* mensaje);
void liberar_mensaje_appeared(t_appeared_pokemon* mensaje);
void liberar_mensaje_new(t_new_pokemon* mensaje);
void liberar_suscripcion(t_suscripcion* mensaje);
void liberar_ack(t_ack* mensaje);
uint32_t size_caught_pokemon(t_caught_pokemon* pokemon);
uint32_t size_get_pokemon(t_get_pokemon* pokemon);
uint32_t size_catch_pokemon(t_catch_pokemon* pokemon);
uint32_t size_localized_pokemon(t_localized_pokemon* pokemon);
uint32_t size_appeared_pokemon(t_appeared_pokemon* pokemon);
uint32_t size_new_pokemon(t_new_pokemon* pokemon);
uint32_t size_ack(t_ack* confirmacion);
uint32_t size_suscripcion(t_suscripcion* suscripcion);
uint32_t size_mensaje(void* mensaje, op_code codigo);
void* serializar_paquete(void* mensaje, uint32_t size_mensaje, op_code codigo, uint32_t* size_serializado);
void* serializar_new_pokemon(void* mensaje, uint32_t size_mensaje, uint32_t* size_serializado);
void* serializar_get_pokemon(void* mensaje, uint32_t size_mensaje, uint32_t* size_serializado);
void* serializar_appeared_pokemon(void* mensaje, uint32_t size_mensaje, uint32_t* size_serializado);
void* serializar_localized_pokemon(void* mensaje_new, uint32_t size_mensaje, uint32_t* size_serializado);
void* serializar_catch_pokemon(void* mensaje, uint32_t size_mensaje, uint32_t* size_serializado);
void* serializar_caught_pokemon(void* mensaje, uint32_t size_mensaje, uint32_t* size_serializado);
void* serializar_suscripcion(void* mensaje, uint32_t size_mensaje, uint32_t* size_serializado);
void* serializar_ack(void* mensaje, uint32_t size_mensaje, uint32_t* size_serializado);
void* deserealizar_paquete(void* stream, op_code codigo_operacion, uint32_t tamanio_mensaje);
t_suscripcion* deserealizar_suscripcion(void* stream, uint32_t size_mensaje);
t_ack* deserealizar_ack(void* stream, uint32_t size_mensaje);
t_get_pokemon* deserealizar_get_pokemon(void* stream, uint32_t size_mensaje);
t_catch_pokemon* deserealizar_catch_pokemon(void* stream, uint32_t size_mensaje);
t_localized_pokemon* deserealizar_localized_pokemon(void* stream, uint32_t size_mensaje);
t_caught_pokemon* deserealizar_caught_pokemon(void* stream, uint32_t size_mensaje);
t_appeared_pokemon* deserealizar_appeared_pokemon(void* stream, uint32_t size_mensaje);
t_new_pokemon* deserealizar_new_pokemon(void* stream, uint32_t size_mensaje);
t_localized_pokemon* deserealizar_localized_pokemon(void*,uint32_t);



// terminar
void liberar_conexion(uint32_t socket_cliente);
void liberar_logger();


#endif /* UTILS_H_ */
