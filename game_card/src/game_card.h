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
sem_t terminarprograma;
sem_t muteadito;
pthread_t hilo_game_card;
pthread_t hilo_broker;
pthread_t hilo_servidor;
pthread_t hilo_new;
pthread_t hilo_catch;
pthread_t hilo_get;

typedef struct {
    int tiempo_reintento_conexion;
	int tiempo_reintento_operacion;
	int tiempo_retardo_operacion;
	int id_proceso;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	char* puerto_broker;
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
void grabar_metadata_file(t_file_metadata metadata,char* path);
//package sending
int bitarray_default_size_in_bytes();
void enviar_new_pokemon(t_new_pokemon* pokemon, uint32_t socket_cliente);
t_new_pokemon* recibir_new_pokemon(uint32_t socket_cliente, uint32_t* size);
void informar_al_broker(uint32_t id_mensaje, op_code codigo);
t_appeared_pokemon* armar_appeared(t_new_pokemon* new_pokemon);
bool existePokemonEnPosicion(t_new_pokemon* pokemon);
void enviar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon,int socket);
int block_que_tiene_esa_posicion(char** blocks,char* key);
int calcular_size_inicial(t_new_pokemon* newPoke);
void appendy(char* s, char c);
void iniciar_hilo_catch();
void iniciar_hilo_new();
void iniciar_hilo_get();
void iniciar_hilos_suscripcion();
void iniciar_semaforos();
void limpiar_blocks(char** blocks);
int verificar_espacio_ocupado_por_pokemon(t_catch_pokemon* catch_pokemon,char* meta_path,int resultado);

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
int file_current_size(block);
char* list_to_string_array(t_list* blocks);
char* block_path(char* block);
int hay_algun_block_vacio(char** blocks);
int posicion_block_vacio(char** blocks, char* block_vacio);
void liberar_block_de_la_indextable(char* metapath,char* block_vacio,t_config* metaconfig);
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
void iniciar_conexion();
bool es_directorio(char* path);
void* iniciar_server_gamecard();
int size_char_doble(char** array);
int la_posicion_ya_existe(t_new_pokemon* newpoke,char* meta_path, char* key_posicion);
t_list* chardoble_to_tlist(char** chardoble);
void remover_posicion(char* a,char* key,char* meta_path);
void informar_al_broker(uint32_t, op_code);
void iniciar_hilo_broker();
void llenar_lista(t_list* lista_de_todo, char* temporary_file);
void funcion_hilo_get_pokemon(t_get_pokemon* get_pokemon,uint32_t socket_br);
void verificar_espacio_en_blocks(char* metapath);
void funcion_hilo_catch_pokemon(t_catch_pokemon* catch_pokemon,uint32_t socket_br);
void liberar_block_de_la_indextable(char* metapath,char* block_vacio,t_config* metaconfig);
void liberar_block_del_bitmap(char* numero_block);
int block_default_size();
void crear_directorios(char* path);
bool el_file_system_esta_lleno();
void crear_bitmap(char* path);
void crear_blocks(char* path);
void create_file_with_size(char* path,int size);
void destrozar_fs_metadata(t_metadata metadata);
void crear_pokemon(t_new_pokemon* newpoke,char* path);
/* CATCH POKEMON */
t_caught_pokemon* armar_caught_pokemon(t_catch_pokemon* catch_pokemon,uint32_t resultado);
/* FS */
void agregar_nueva_posicion(t_new_pokemon* newpoke,char* pathmeta_poke,char* key,char* value);
void escribir_data_en_block(char* path_last_block,char* key,char* value);
void iniciar_tall_grass(int cantidad_clusters,int tam_clusters);
t_metadata leer_fs_metadata();
void escribirMetadata();
void crear_metadata_fs(char* path,int cantidad_clusters,int tam_cluster);
void escribir_block_inicial(t_file_metadata metadata,t_new_pokemon* newPoke);
int tamanio_de_metadata(t_metadata metadata);
int tamanio_file_metadata(t_file_metadata fileMeta);
t_file_metadata leer_file_metadata(char* path);
char* leer_linea(FILE* archivo);
int file_size(char* path);
void actualizar_size_new_pokemon(char* pathmetafile,int nuevosize);
/* COMMONS */
void unlock_file(char* path);
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
