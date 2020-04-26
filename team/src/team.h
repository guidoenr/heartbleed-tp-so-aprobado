#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

typedef struct { // capaz tengamos que agregar un id
    int id;
	int posicion[2];
} t_entrenador;

typedef struct {
	t_list* posiciones_entrenadores;
	t_list* pokemon_entrenadores;
	t_list* objetivos_entrenadores;
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

/*void concatenar(char* palabra, char c);*/
char* append(const char *s, char c);
void* parsear(char** datos_de_config);
void* parsear_posiciones(char** datos_de_config);

void iniciar_programa();
void leer_config(void);
t_list* obtener_objetivo_global();
void terminar_programa(int, t_log*, t_config_team*);
void liberar_conexion(int);
void liberar_logger(t_log* logger);
void liberar_config(t_config_team*);
