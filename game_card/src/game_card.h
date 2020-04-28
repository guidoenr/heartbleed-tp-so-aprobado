#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

typedef struct {
    int tiempo_reintento_conexion;
	int tiempo_reintento_operacion;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	char* puerto_broker;
	char* ip_gameBoy;
	char* puerto_gameBoy;
	char* ip_gameCard;
	char* puerto_gameCard;

} t_config_game_card;


typedef struct {
    int blocksize;
	int blocks;
	char* magic;
}t_metadata;

typedef struct{
	char directory;
	int size;
	int blocks; //TODO - array dinamico?
	char open;
}t_file_metadata;

t_log* logger;
t_config_game_card* leer_config(void);
void terminar_programa(int, t_config_game_card*);
void liberar_conexion(int);
void liberar_logger(t_log* logger);
void liberar_config(t_config_game_card*);
void leerMetadata();
void escribirMetadata();
void crearMetadata();
int tamanio_de_metadata(t_metadata metadata);
