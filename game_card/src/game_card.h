#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/bitarray.h>
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

t_config_game_card* config_gc;
t_config_game_card* leer_config(void);

typedef struct {
	int blocksize;
	int blocks;
	char* magic;
}t_metadata;

typedef struct{
	char* directory;
	char* size;
	t_list* blocks;
	char* open;
}t_file_metadata;

//package sending
void enviar_new_pokemon(t_new_pokemon* pokemon, uint32_t socket_cliente);
t_new_pokemon* recibir_new_pokemon(uint32_t socket_cliente, uint32_t* size);
void informarAlBroker(int socket,op_code codigo);
t_appeared_pokemon* armar_appeared(t_new_pokemon* new_pokemon);
bool existePokemonEnPosicion(t_new_pokemon* pokemon);
void enviar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon,int socket);

//metadata + fileSystem
void iniciar_tall_grass();
t_metadata leer_fs_metadata();
void escribirMetadata();
void crear_metadata_fs();
int tamanio_de_metadata(t_metadata metadata);
int existe_directorio(char* path);
void verificar_existencia_pokemon(t_new_pokemon* pokemon,int socket);
void verificar_apertura_pokemon(t_new_pokemon* msg,int socket);
int tamanio_file_metadata(t_file_metadata fileMeta);
t_file_metadata leer_file_metadata(char* path);

//commons
void terminar_programa(int, t_config_game_card*);
void liberar_conexion(uint32_t);
void liberar_logger();
void liberar_config_gc(t_config_game_card*);
void suscribirme_a_colas();
void suscribirse_a(op_code);

//procces
void process_request(uint32_t cod_op, uint32_t cliente_fd);
void funcion_hilo_new_pokemon(t_new_pokemon* pokemon,int socket);

//parsers + tools
char* concatenar(char* str1,char* str2);
bool isFile(char* path);
uint32_t sizeNewPokemon(t_new_pokemon* pokemon);
uint32_t sizeAppearedPokemon(t_appeared_pokemon* pokemon);
char* obtener_path_metafile(t_new_pokemon* pokemon);
char* obtener_path_dir_pokemon(t_new_pokemon* pokemon);
void conectarse(int socket);
bool is_open(char* path);
bool isDir(const char* name);
bool existeElFileSystem(char* puntoMontaje);
t_list* asignar_block_inicial();
void inicializarPokemon(t_new_pokemon* newPoke);
char* buscar_block_libre();
char* posicion_into_string(t_new_pokemon* newpoke);
t_file_metadata generar_file_metadata(t_new_pokemon* newPoke);
t_bitarray* obtener_bitmap();
char* get_value_from_position(t_new_pokemon* newpoke);
char* get_key_from_position(t_new_pokemon* newpoke);
char* list_to_string_array(t_list* blocks);
int la_posicion_ya_existe(t_new_pokemon* newpoke,char* key);
