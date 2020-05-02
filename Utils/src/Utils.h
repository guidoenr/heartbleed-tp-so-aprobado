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
	ACK=8  	//temas gamecard, no lo entenderías..
}op_code;

typedef struct {
	int id_mensaje;
	char* pokemon;
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
	char* pokemon;
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
	int id_mensaje;
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct{
	char* ip;
	char* puerto;
} t_subscripcion;

pthread_t thread;
t_log* logger;
int id_mensaje_univoco;
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
void enviar_mensaje(int cod_op, void* mensaje, int socket_cliente);
void* recibir_mensaje(int socket_cliente, int* size);
void devolver_mensaje(int cod_op, int size, void* payload, int socket_cliente);
void reenviar_mensaje(int cod_op, int size, void* payload, int socket_cliente);

void iniciar_logger(char* file, char* program_name);
void liberar_conexion(int socket_cliente);
void liberar_logger();


#endif /* UTILS_H_ */
