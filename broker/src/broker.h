
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

typedef struct {
    int size_memoria;
	int size_min_memoria;
	char* algoritmo_memoria;
	char* algoritmo_reemplazo;
	char* algoritmo_particion_libre;
	char* ip_broker;
	char* puerto;
	int frecuencia_compactacion;
	char* log_file;
} t_config_broker;

typedef struct {
	t_list* cola_catch;
	t_list* cola_caught;
	t_list* cola_get;
	t_list* cola_localized;
	t_list* cola_new;
	t_list* cola_appeared;
} t_colas_mensajes;

typedef struct {
	t_list* lista_suscriptores_catch;
	t_list* lista_suscriptores_caught;
	t_list* lista_suscriptores_get;
	t_list* lista_suscriptores_localized;
	t_list* lista_suscriptores_new;
	t_list* lista_suscriptores_appeared;
} t_listas_suscriptores;

t_config_broker* config;
t_config_broker* config_broker;
t_log* logger;
t_listas_suscriptores* listas_de_suscriptos;
t_colas_mensajes* colas_de_mensajes;

void iniciar_programa(void);
void iniciar_logger(char*, char*);
void leer_config(void);
void terminar_programa(t_log*, t_config_broker*);
void liberar_config(t_config_broker*);
void crear_colas_de_mensajes(void);
void crear_listas_de_suscriptores(void);
void liberar_listas(void);
