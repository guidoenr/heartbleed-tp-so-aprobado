#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/bitarray.h>
#include<readline/readline.h>
#include<semaphore.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"


//global
t_log* logger;
char* punto_montaje;
sem_t mx_bitmap;
pthread_t hilo_game_boy;

typedef struct {
    int tiempo_reintento_conexion;
	int tiempo_reintento_operacion;
	int tiempo_retardo_operacion;
	int id_proceso;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	char* puerto_broker;
	char* ip_gameBoy;
	char* puerto_gameBoy;
	char* ip_gameCard;
	char* puerto_gameCard;
} t_config_game_card;

typedef struct{
	char* x;
	char* y;
	char* value;
}t_localized_lista;



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
void informar_al_broker(uint32_t id_mensaje, op_code codigo);
t_appeared_pokemon* armar_appeared(t_new_pokemon* new_pokemon);
bool existePokemonEnPosicion(t_new_pokemon* pokemon);
void enviar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon,int socket);

/* NEW POKEMON */
void verificar_existencia_pokemon(t_new_pokemon* pokemon,int socket);
void funcion_hilo_new_pokemon(t_new_pokemon* pokemon,uint32_t socket);
void re_grabar_temporary_en_blocks(char* temporary_file,char* path_metafile);
void destrozar_metadata_file(t_file_metadata metadata);
void destrozar_new_pokemon(t_new_pokemon* new_pokemon);
void escribir_data_sin_fragmentacion_interna(char* block_nuevo,char* block_viejo, char*key, char*value);
void conectarse_a_br();
void conectarse_a_gb(int socket);
void verificar_apertura_pokemon(char* path_metafile,char* nombre_pokemon);
void actualizar_pokemon(char* temporary_path,char* path_metafile,char* key,char* value);
char* get_key_from_position(uint32_t posicion[2]);
char* get_value_from_cantidad(uint32_t cantidad);
char* list_to_string_array(t_list* blocks);
char* block_path(char* block);
char* rand_string(char* nombre_pokemon);
char* generar_string_desde_blocks(char** blocks);
char* generar_archivo_temporal(char* metapath_file,char* nombre_pokemon);
bool los_blocks_estan_llenos(char** blocks,int cantidad_blocks);
bool el_block_tiene_espacio_justo(char* key,char* value,char* ultimo_block_path);
bool el_block_tiene_espacio_pero_no_alcanza(char* blockpath);
bool existe_la_key_mal_grabada(char* key,char* temporary_file);
bool esta_la_posicion_mal_grabada(char* key,char* temporary_file);
bool existe_la_posicion(char* key,char* temporary_path);
bool el_pokemon_esta_creado(char* path);
bool es_directorio(char* path);
int size_char_doble(char** array);
int la_posicion_ya_existe(t_new_pokemon* newpoke,char* meta_path, char* key_posicion);
t_list* chardoble_to_tlist(char** chardoble);
void remover_posicion(char* a,char* key,char* meta_path);
/* CATCH POKEMON */
t_caught_pokemon* armar_caught_pokemon(t_catch_pokemon* catch_pokemon,uint32_t resultado);
/* FS */
void iniciar_tall_grass();
t_metadata leer_fs_metadata();
void escribirMetadata();
void crear_metadata_fs();
int tamanio_de_metadata(t_metadata metadata);
int tamanio_file_metadata(t_file_metadata fileMeta);
t_file_metadata leer_file_metadata(char* path);
char* leer_linea(FILE* archivo);

/* COMMONS */
void terminar_programa(int, t_config_game_card*);
void liberar_conexion(uint32_t);
void liberar_logger();
void liberar_config_gc(t_config_game_card*);
void suscribirme_a_colas();
void suscribirse_a(op_code);
void process_request(uint32_t cod_op, uint32_t cliente_fd);
uint32_t recibir_id_de_mensaje_enviado(uint32_t socket_cliente);
//parsers + tools
char* concatenar(char* str1,char* str2);
bool isFile(char* path);
uint32_t sizeNewPokemon(t_new_pokemon* pokemon);
uint32_t sizeAppearedPokemon(t_appeared_pokemon* pokemon);
char* obtener_path_metafile(char* nombre_pokemon);
char* obtener_path_dir_pokemon(char* nombre_pokemon);
void conectarse(int socket);
bool esta_lockeado(char* path);
bool isDir(const char* name);
bool existeElFileSystem(char* puntoMontaje);
t_list* asignar_block_inicial();
void inicializarPokemon(t_new_pokemon* newPoke);
char* buscar_block_libre();
char* posicion_into_string(char*key,char*value);
t_file_metadata generar_file_metadata(t_new_pokemon* newPoke);
t_bitarray* obtener_bitmap();
char* get_y_from_linea(char* linea);
char* get_x_from_linea(char* linea);
char* get_value_from_linea(char* linea);

void pruebas_catch_pokemon(int socket);
t_list* obtener_posiciones_y_cantidades(char* meta_path,char* temporary_file);
