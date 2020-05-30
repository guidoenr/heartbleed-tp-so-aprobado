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

typedef enum{
	EN_ESPERA =0, //NO LO ENVIAMOS
	ENVIADO = 1,/// SE LO ENVIA A UN PROCESO
	CONFIRMADO = 2, /// ME CONFIRMO EL PROCESO
} status_mensaje;

typedef struct {
	t_paquete* mensaje;
	uint32_t id;
	uint32_t id_correlativo;///AConfirmar
	status_mensaje estado_mensaje;
	char[20] suscriptor; //Esto esta horrible, lo tenemos que pensar.
}t_mensaje;

typedef struct {
	t_list* cola_catch;
	t_list* cola_caught;
	t_list* cola_get;
	t_list* cola_localized;
	t_list* cola_new;
	t_list* cola_appeared;
} t_colas_mensajes;

typedef struct{
  uint32_t id_mensaje;
  uint32_t socket;
}t_ack;

typedef struct {
	t_list* lista_suscriptores_catch;
	t_list* lista_suscriptores_caught;
	t_list* lista_suscriptores_get;
	t_list* lista_suscriptores_localized;
	t_list* lista_suscriptores_new;
	t_list* lista_suscriptores_appeared;
} t_listas_suscriptores;

//Preguntar por estructuras auxiliares que se mencionan en el enunciado
typedef struct {
	void* mensaje;
} t_memoria_cache;

t_memoria_cache* memoria_cache;
t_config_broker* config;
t_config_broker* config_broker;
t_log* logger;
t_listas_suscriptores* listas_de_suscriptos;
t_colas_mensajes* colas_de_mensajes;

sem_t semaforo;

//FUnciones generales
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
void encolar_mensaje(t_paquete*, op_code);
void recibir_suscripcion(t_paquete*);
t_suscripcion* deserealizar_suscripcion(void*);
void agregar_mensaje(uint32_t,uint32_t,void*,uint32_t);
uint32_t generar_id_univoco(void);
void suscribir_a_cola(t_list*, t_suscripcion*);
bool es_la_misma_suscripcion(void*);
void informar_mensajes_previos(t_suscripcion*);
void descargar_historial_mensajes(t_list*, uint32_t);
void gestionar_mensajeria(void);
void enviar_mensajes_get(void);
void enviar_mensajes_catch(void);
void enviar_mensajes_localized(void);
void enviar_mensajes_caught(void);
void enviar_mensajes_appeared(void);
void enviar_mensaje_get(void*);
void recibir_confirmacion_de_recepcion(uint32_t);


