#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"
#include <semaphore.h>

typedef struct {
    uint32_t size_memoria;
	uint32_t size_min_memoria;
	char* algoritmo_memoria;
	char* algoritmo_reemplazo;
	char* algoritmo_particion_libre;
	char* ip_broker;
	char* puerto;
	uint32_t frecuencia_compactacion;
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

/*typedef struct {
	uint32_t socket;
	op_code cola_a_suscribir;
} t_suscripcion;*/

t_config_broker* config;
t_config_broker* config_broker;
t_log* logger;
t_listas_suscriptores* listas_de_suscriptos;
t_colas_mensajes* colas_de_mensajes;
sem_t* semaforo;

void iniciar_programa(void);
void iniciar_logger(char*, char*);
void leer_config(void);
void terminar_programa(t_log*, t_config_broker*);
void liberar_config(t_config_broker*);
void crear_colas_de_mensajes(void);
void crear_listas_de_suscriptores(void);
void liberar_listas(void);
void encolar_mensaje(t_paquete*, op_code);
void recibir_suscripcion(t_paquete*, t_suscripcion*);
void enviar_mensajes_get(void);
void agregar_mensaje(uint32_t,uint32_t,void*,uint32_t);
