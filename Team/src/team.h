
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<utils.h>

typedef struct {
	char* pokemon;
	int* prev;
	int* next;
} t_lista_pokemons;

typedef struct { // capaz tengamos que agregar un id
    int posicion[2];
    int* prev;
	int* next;
} t_lista_posiciones;

typedef struct {
	t_lista_pokemons* pokemon;
	int* prev;
	int* next;
} t_lista_pokemon_entrenadores;

typedef struct {
	t_lista_pokemons* pokemon;
	int* prev;
	int* next;
} t_lista_objetivos_entrenadores;

typedef struct {
	t_lista_posiciones* posiciones_entrenadores;
	t_lista_pokemon_entrenadores* pokemon_entrenadores;
	t_lista_objetivos_entrenadores* objetivos_entrenadores;
    int tiempo_reconexion;
	int retardo_cpu;
	char* algoritmo_planificacion;
	char* ip_broker;
	char* puerto_broker;
	int estimacion_inicial;
	char* log_path;
} t_config_team;

t_config_team* config;
t_log* logger;
void iniciar_logger(void);
void leer_config(void);
void terminar_programa(int, t_log*, t_config_team*);
void liberar_conexion(int);
void liberar_logger(t_log* logger);
void liberar_config(t_config_team*);


