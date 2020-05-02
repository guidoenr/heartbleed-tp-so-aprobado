#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

// agregar un id 0 al mensaje a mandar al broker

typedef struct {
	int posicion[2];
	t_list* pokemons;
	t_list* objetivos;
} t_entrenador;

typedef struct {
	t_list* entrenadores;
    int tiempo_reconexion;
	int retardo_cpu;
	char* algoritmo_planificacion;
	char* ip_broker;
	char* puerto_broker;
	int estimacion_inicial;
	char* log_file;
} t_config_team;

t_config_team* config;
t_log* logger;
t_list* objetivo_global;

// estado
t_list* estado_new;
t_list* estado_ready;
t_list* estado_exec;
t_list* estado_block;
t_list* estado_exit;


// utils
char* append(const char *s, char c);
void* parsear(char** datos_de_config);
void* parsear_posiciones(char** datos_de_config);

// iniciar
void iniciar_programa();
void inicializar_estados();
void leer_config(void);
t_list* load_entrenadores(t_list*, t_list*, t_list*);
void cargar_pokemons_a_entrenador(t_list*, t_link_element*, t_list*);
void obtener_objetivo_global();
void obtener_entrenadores(void*);
void agregar_pokemon_a_objetivos(void*);

// estados
void agregar_a_estado(t_list*, int*);
void cambiar_a_estado(t_list*, int*);
void eliminar_de_estado(t_list*, int*);
t_list* buscar_en_estados(t_list*, int*);
bool esta_en_estado(t_list*, int*);

// operar
void* operar_entrenador(void*);

// terminar
void liberar_lista_de_lista_de_strings(t_list*);
void liberar_conexion(int);
void liberar_config();
void liberar_entrenadores();
void terminar_programa(int);
void liberar_conexion(int);
