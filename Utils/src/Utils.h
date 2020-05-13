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
	op_code cola_a_suscribir;
	uint32_t tiempo_suscripcion;
} t_suscripcion;

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
	char* pokemon;
	uint32_t posicion[2];
} t_appeared_pokemon;

typedef struct {
	uint32_t id_mensaje;
	uint32_t resultado;
} t_caught_pokemon;

typedef struct {
	char* pokemon;
	uint32_t posicion[2];
	uint32_t cantidad;
	uint32_t* next;
	// capaz tengamos q agregar un uint32_t* previous;
} t_lista_pokemones; //CUIDADO ACA, TEAM TIENE UN t_lista_pokemons, esto va a ir en gamecard

typedef struct {
	uint32_t id_mensaje;
	char* pokemon;
	t_lista_pokemones* lista_pokemons; //CUIDADO ACA, TEAM TIENE UN t_lista_pokemons
} t_localized_pokemon;

typedef struct {
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct {
	uint32_t id_mensaje;
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


pthread_t thread;
t_log* logger;
uint32_t id_mensaje_univoco;
void* recibir_buffer(uint32_t*, uint32_t); /// este no esta definido en utils.c

//client
void* serializar_paquete(t_paquete* paquete, uint32_t* bytes);
uint32_t crear_conexion(char* ip, char* puerto);


//server
void eliminar_paquete(t_paquete* paquete);
void iniciar_servidor(char *IP, char *PUERTO);
void esperar_cliente(uint32_t);
void serve_client(uint32_t *socket);
void process_request(uint32_t cod_op, uint32_t cliente_fd);
uint32_t recibir_operacion(uint32_t);

//mensaje
void enviar_mensaje(uint32_t cod_op, void* mensaje, uint32_t socket_cliente);
void* recibir_mensaje(uint32_t socket_cliente, uint32_t* size);
void devolver_mensaje(uint32_t cod_op, uint32_t size, void* payload, uint32_t socket_cliente);
void reenviar_mensaje(uint32_t cod_op, uint32_t size, void* payload, uint32_t socket_cliente);

void iniciar_logger(char* file, char* program_name);
void liberar_conexion(uint32_t socket_cliente);
void liberar_logger();


#endif /* UTILS_H_ */
