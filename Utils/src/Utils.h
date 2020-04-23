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
	//Team a Broker
	TE_GET_POKEMON_BR = 1,
	TE_CATCH_POKEMON_BR = 2,
	//Gamecard a Broker
	GC_LOCALIZED_POKEMON_BR = 3,
	GC_CAUGHT_POKEMON_BR = 4,
	//Gameboy a Team
	GB_APPEARED_POKEMON_TE = 5,
	//Gameboy a Gamecard
	GB_NEW_POKEMON_GC = 6,
	GB_GET_POKEMON_GC = 7,
	GB_CATCH_POKEMON_GC = 8,
	//Gameboy a Broker
	GB_NEW_POKEMON_BR = 9,
	GB_CAUGHT_POKEMON_BR = 10,
	//Broker a Cola - Gamecard
	BR_GET_POKEMON_GC = 11,
	BR_CATCH_POKEMON_GC = 12,
	BR_NEW_POKEMON_GC = 13,
	//Broker a Cola - Team
	BR_LOCALIZED_POKEMON_TE = 14,
	BR_CAUGHT_POKEMON_TE = 15, // A ESTE LO PUEDEN ACCEDER PROVINIENDO DE GC O GB.
}op_code;

typedef struct {
	int id_mensaje;
	int pokemon;
	int posicion[2];
	int cantidad;
} t_new_pokemon;

typedef struct {
	int id_mensaje;
	int pokemon;
	int posicion[2];
} t_catch_pokemon;

typedef struct {
	int id_mensaje;
	int pokemon;
} t_get_pokemon;

typedef struct {
	int id_mensaje;
	int pokemon;
	int posicion[2];
} t_appeared_pokemon;

typedef struct {
	int id_mensaje;
	int resultado;
} t_caught_pokemon;

typedef struct {
	int pokemon;
	int posicion[2];
	int cantidad;
	int* next;
	// capaz tengamos q agregar un int* previous;
} t_lista_pokemones; //CUIDADO ACA, TEAM TIENE UN t_lista_pokemons, esto va a ir en gamecard

typedef struct {
	int id_mensaje;
	int pokemon;
	t_lista_pokemones* lista_pokemons; //CUIDADO ACA, TEAM TIENE UN t_lista_pokemons
} t_localized_pokemon;

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

pthread_t thread;
t_log* logger;

void* recibir_buffer(int*, int); /// este no esta definido en utils.c

//client
void* serializar_paquete(t_paquete* paquete, int* bytes);
int crear_conexion(char* ip, char* puerto);


//server
void eliminar_paquete(t_paquete* paquete);
void iniciar_servidor(char *IP, char *PUERTO);
void esperar_cliente(int);
void serve_client(int *socket);
void process_request(int cod_op, int cliente_fd);
int recibir_operacion(int);

//mensaje
void enviar_mensaje(int cod_op, char* mensaje, int socket_cliente);
void* recibir_mensaje(int socket_cliente, int* size);
void devolver_mensaje(int cod_op, int size, void* payload, int socket_cliente);
void reenviar_mensaje(int cod_op, int size, void* payload, int socket_cliente);


void liberar_conexion(int socket_cliente);
void liberar_logger(t_log* logger);


#endif /* UTILS_H_ */
