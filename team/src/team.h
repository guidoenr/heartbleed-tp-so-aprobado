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
    int id;
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
t_list* objetivo_global; // conviene global?
pthread_t hilo; // conviene global?
// no se si deberian ser doblemente enlazadas
t_list* estado_new;
t_list* estado_ready;
t_list* estado_exec;
t_list* estado_block;
t_list* estado_exit;

char* append(const char *s, char c);
void* parsear(char** datos_de_config);
void* parsear_posiciones(char** datos_de_config);

void iniciar_programa();
void leer_config(void);
t_list* load_entrenadores(t_list*, t_list*, t_list*);
void cargar_pokemons_a_entrenador(t_list*, t_link_element*, t_list*);
t_list* obtener_objetivo_global();
void terminar_programa(int);
void liberar_conexion(int);
void liberar_config();
