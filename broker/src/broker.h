#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"
#include <semaphore.h>
#include <time.h>

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

t_config_broker* config;
t_config_broker* config_broker;
t_log* logger;
t_listas_suscriptores* listas_de_suscriptos;
t_colas_mensajes* colas_de_mensajes;

sem_t semaforo;
clock_t tiempo_suscripto;

void terminar_programa(t_log*, t_config_broker*);
void liberar_config(t_config_broker*);
void crear_colas_de_mensajes(void);
void crear_listas_de_suscriptores(void);
void liberar_listas(void);
void encolar_mensaje(t_paquete*, op_code);
void recibir_suscripcion(t_paquete*);
t_suscripcion* despaquetar_suscripcion(void*);
void agregar_mensaje(uint32_t,uint32_t,void*,uint32_t);
uint32_t generar_id_univoco(void);
void suscribir_temporalmente(t_list* lista_de_suscriptores, uint32_t socket_cliente);
//void gestionar_suscripciones(void);
void gestionar_mensajeria(void);
void enviar_mensajes_get(void);
void enviar_mensajes_catch(void);
void enviar_mensajes_localized(void);
void enviar_mensajes_caught(void);
void enviar_mensajes_appeared(void);


