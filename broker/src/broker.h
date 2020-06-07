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


t_list* cola_catch;
t_list* cola_caught;
t_list* cola_get;
t_list* cola_localized;
t_list* cola_new;
t_list* cola_appeared;

typedef struct{
	uint32_t id_mensaje;
	uint32_t id_correlativo;
	void* payload;
	t_list* suscriptor_enviado;
	t_list* suscriptor_recibido;
	op_code codigo_operacion;
}t_mensaje;

typedef struct{
	uint32_t socket;
	char* emisor;
	uint32_t temporal;
}t_suscriptor;

typedef struct{
	uint32_t tamanio_mensaje;
	void* payload;
	uint32_t base;
}t_memoria_dinamica;

typedef struct{
  uint32_t id_mensaje;
  uint32_t socket;
}t_ack;

t_list* lista_suscriptores_catch;
t_list* lista_suscriptores_caught;
t_list* lista_suscriptores_get;
t_list* lista_suscriptores_localized;
t_list* lista_suscriptores_new;
t_list* lista_suscriptores_appeared;


void* memoria_cache;
t_config_broker* config;
t_config_broker* config_broker;
t_log* logger;


sem_t semaforo;
sem_t mutex_id_correlativo;

//Funciones generales
void iniciar_programa(void);
void reservar_memoria(void);
void leer_config(void);
void terminar_programa(t_log*, t_config_broker*);
void liberar_config(t_config_broker*);
void crear_colas_de_mensajes(void);
void crear_listas_de_suscriptores(void);
void liberar_listas(void);
void liberar_memoria_cache(void);

//Administracion de mensajes
void encolar_mensaje(t_mensaje*, op_code);
void agregar_mensaje(uint32_t,uint32_t,void*,uint32_t);
uint32_t generar_id_univoco(void);
void enviar_mensajes_get(void);
void enviar_mensajes_catch(void);
void enviar_mensajes_localized(void);
void enviar_mensajes_caught(void);
void enviar_mensajes_appeared(void);
void enviar_mensaje_get(void*);
op_code recibir_confirmacion_de_recepcion(uint32_t, uint32_t);
void desencolar_mensaje(t_mensaje*);
void actualizar_suscriptor(t_mensaje*, void*);
void agregar_suscriptor(t_mensaje*, void*);

//Suscripciones
void recibir_suscripcion(t_mensaje*);
t_suscripcion* deserealizar_suscripcion(void*);
void suscribir_a_cola(t_list*, t_suscripcion*);
bool es_la_misma_suscripcion(void*);
void informar_mensajes_previos(t_suscripcion*);
void descargar_historial_mensajes(t_list*, uint32_t);

//Memoria
void ubicar_particion_de_memoria(void);
void eliminar_particion_de_memoria(void);
void compactar_memoria(void);
void guardar_en_memoria(t_mensaje*);
