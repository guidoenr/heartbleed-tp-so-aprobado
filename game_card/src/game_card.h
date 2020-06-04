#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <semaphore.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"


//global
t_log* logger;
char* punto_montaje;
t_new_pokemon* luken;



typedef struct {
    int tiempo_reintento_conexion;
	int tiempo_reintento_operacion;
	int tiempo_retardo_operacion;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	char* puerto_broker;
	char* ip_gameBoy;
	char* puerto_gameBoy;
	char* ip_gameCard;
	char* puerto_gameCard;

} t_config_game_card;


t_config_game_card* config;
t_config_game_card* leer_config(void);


typedef struct {
    int blocksize;
	int blocks;
	char* magic;
}t_metadata;

typedef struct{
	char directory;
	int size;
	int blocks[2]; //TODO - array dinamico?
	char open;
}t_file_metadata;

//package sending
void enviar_new_pokemon(t_new_pokemon* pokemon, uint32_t socket_cliente);
t_new_pokemon* recibir_new_pokemon(uint32_t socket_cliente, uint32_t* size);
void informarAlBroker(int socket,op_code codigo);
t_appeared_pokemon* armar_appeared(t_new_pokemon* new_pokemon);
bool existePokemonEnPosicion(t_new_pokemon* pokemon);
void enviar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon,int socket);



//metadata + fileSystem
void iniciarTallGrass();
t_metadata leerMetadata();
void escribirMetadata();
void crearMetadata();
int tamanio_de_metadata(t_metadata metadata);
int existeDirectorio(char* path);
void verificarExistenciaPokemon(t_new_pokemon* pokemon);
void verificarAperturaPokemon(t_new_pokemon* msg,int socket);
int tamanio_file_metadata(t_file_metadata fileMeta);


//commons
void terminar_programa(int, t_config_game_card*);
void liberar_conexion(uint32_t);
void liberar_logger();
void liberar_config(t_config_game_card*);
void suscribirme_a_colas();
void suscribirse_a(op_code);

//procces
void process_request(uint32_t cod_op, uint32_t cliente_fd);
void funcionHiloNewPokemon(t_new_pokemon* pokemon,int socket);

//parsers + tools
char* concatenar(char* str1,char* str2);
bool isFile(char* path);
uint32_t sizeNewPokemon(t_new_pokemon* pokemon);
uint32_t sizeAppearedPokemon(t_appeared_pokemon* pokemon);
char* obtenerPathMetaFile(t_new_pokemon* pokemon);
void conectarse(int socket);
