#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>

//--------- estructuras ---------//
typedef struct {
    uint32_t indice;
    uint32_t base;
    uint32_t padre_id;
} t_indice;

typedef struct {
    uint32_t size_memoria;
	uint32_t size_min_memoria;
	char* 	 algoritmo_memoria;
	char* 	 algoritmo_reemplazo;
	char* 	 algoritmo_particion_libre;
	char* 	 ip_broker;
	char* 	 puerto;
	uint32_t frecuencia_compactacion;
	char* 	 log_file;
	char* 	 memory_log;
} t_config_broker;

typedef struct {
	uint32_t id_mensaje;
	uint32_t id_correlativo;
	void* 	 payload;
	t_list*  suscriptor_enviado;
	t_list*  suscriptor_recibido;
	op_code  codigo_operacion;
	uint32_t tamanio_mensaje;
	uint32_t tamanio_lista_localized;
} t_mensaje;

typedef struct {
	uint32_t tamanio;
	uint32_t tamanio_part;
	uint32_t base;
	uint32_t ocupado;
	uint64_t ultima_referencia;
	uint64_t tiempo_de_carga;
	op_code codigo_operacion;
	void* contenido;
} t_memoria_dinamica;

typedef struct{
	uint32_t tamanio_exponente;
	uint32_t tamanio_mensaje;
	uint32_t base;
	uint32_t ocupado;
	uint64_t ultima_referencia;
	uint64_t tiempo_de_carga;
	op_code codigo_operacion;
	void* contenido;
	uint32_t posicion; // 0 = izquierda y 1 = derecha
} t_memoria_buddy;

typedef struct {
	t_suscripcion* suscriptor;
	t_mensaje* mensaje;
} t_envio_mensaje;


//--------- listas ---------//
t_list* cola_catch;
t_list* cola_caught;
t_list* cola_get;
t_list* cola_localized;
t_list* cola_new;
t_list* cola_appeared;

t_list* lista_suscriptores_catch;
t_list* lista_suscriptores_caught;
t_list* lista_suscriptores_get;
t_list* lista_suscriptores_localized;
t_list* lista_suscriptores_new;
t_list* lista_suscriptores_appeared;

t_list* memoria_cache;
t_list* memoria_con_particiones;
sem_t muteadito;
sem_t muteadin;
sem_t envio_mensaje;
t_config* config;
t_config_broker* config_broker;
t_log* logger;
t_log* logger_memoria;


//--------- variables ---------//
uint32_t particiones_liberadas;
uint32_t asignado = 0;
uint32_t numero_particion = 0;
uint32_t id_mensaje_univoco;
void* memoria;
uint32_t nodo_id;
uint32_t id_bloque;


//--------- hilos ---------//
pthread_t hilo_mensaje;
pthread_t hilo_envio_mensajes;
pthread_t hilo_signal;


//--------- semaforos --------- //
sem_t mx_suscripciones;
sem_t muteadito;

// --------- funciones generales --------- //
void iniciar_programa();
void iniciar_semaforos_broker();
void reservar_memoria();
void leer_config();
void terminar_programa(t_log*);
void liberar_config(t_config_broker*);
void crear_colas_de_mensajes();
void crear_listas_de_suscriptores();
void liberar_listas();
void liberar_memoria_cache();
void crear_hilo_por_mensaje();
void terminar_hilos_broker();
void liberar_semaforos_broker();
void iniciar_logger_broker(char*, char*);


//--------- administracion de mensajes --------- //
void encolar_mensaje(t_mensaje*, op_code);
void agregar_mensaje(uint32_t,uint32_t,void*,uint32_t);
uint32_t generar_id_univoco(void);
void enviar_mensajes(t_mensaje*, t_list*);
void actualizar_mensajes_confirmados(t_ack*);
void agregar_suscriptor_a_enviados_confirmados(t_mensaje*, uint32_t);
void agregar_suscriptor_a_enviados_sin_confirmar(t_mensaje*, uint32_t);
uint32_t eliminar_suscriptor_de_enviados_sin_confirmar(t_mensaje*, uint32_t);
void eliminar_mensajes_confirmados(void);
void borrar_mensajes_confirmados(op_code, t_list*, t_list*);
void eliminar_mensaje(void*);
bool tiene_el_mensaje(t_list*, uint32_t);
void* preparar_mensaje(t_mensaje*);
void* armar_contenido_appeared(t_appeared_pokemon*);
void* armar_contenido_localized(t_localized_pokemon*);
void* armar_contenido_caught(t_caught_pokemon*);
void* armar_contenido_catch(t_catch_pokemon*);
void* armar_contenido_get(t_get_pokemon*);
void* armar_contenido_new(t_new_pokemon*);
void* armar_contenido_de_mensaje(void*, uint32_t);
t_mensaje* encontrar_mensaje(uint32_t, op_code);
bool puede_guardarse_mensaje(t_mensaje*);
t_mensaje* eliminar_de_message_queue(t_mensaje*, op_code);
uint32_t obtener_tamanio_contenido_mensaje(void*, uint32_t);
t_get_pokemon* preparar_mensaje_get(t_mensaje*);
t_catch_pokemon* preparar_mensaje_catch(t_mensaje*);
t_localized_pokemon* preparar_mensaje_localized(t_mensaje*);
t_caught_pokemon* preparar_mensaje_caught(t_mensaje*);
t_new_pokemon* preparar_mensaje_new(t_mensaje*);
t_appeared_pokemon* preparar_mensaje_appeared(t_mensaje*);


//--------- suscripciones ---------//
void recibir_suscripcion(t_suscripcion* mensaje_suscripcion,uint32_t socket);
void suscribir_a_cola(t_list*, t_suscripcion*, op_code);
bool es_la_misma_suscripcion(void*);
void informar_mensajes_previos(t_suscripcion*, op_code);
void descargar_historial_mensajes(op_code, uint32_t, uint32_t);
void destruir_suscripcion(void*);
void enviar_mensajes_cacheados_en_particiones(op_code tipo_mensaje, uint32_t socket);
void enviar_mensajes_cacheados_en_buddy_system(op_code tipo_mensaje, uint32_t socket);


//--------- memoria ---------//
void guardar_en_memoria(t_mensaje*, void*);
void ubicar_particion(uint32_t, t_memoria_dinamica*);
void reemplazo_buddy(uint32_t, void*, t_mensaje*);
void liberar_particion_dinamica(t_memoria_dinamica*);
uint32_t obtener_nueva_base(t_memoria_dinamica*, uint32_t);
t_memoria_dinamica* seleccionar_victima_de_reemplazo_fifo();
t_memoria_dinamica* seleccionar_victima_de_reemplazo_lru();
void iniciar_memoria_particiones(t_list*);
uint32_t encontrar_primer_ajuste(uint32_t);
uint32_t encontrar_mejor_ajuste(uint32_t);
void destruir_particion(void*);
uint32_t encontrar_indice(void*);
void consolidar_particiones_dinamicas(t_list*);
void consolidar_particiones(uint32_t, uint32_t);
void compactar_memoria_cache(t_list*);
void compactar_particiones_dinamicas(t_list*);
void guardar_particion(t_mensaje*, void*);
void dump_info_particion(void*);
void guardar_contenido_de_mensaje(uint32_t,  void*, uint32_t);
bool ambas_estan_vacias(uint32_t, uint32_t);
t_memoria_dinamica* armar_particion(uint32_t, uint32_t, t_mensaje*, uint32_t, void*);
char* obtener_cola_del_mensaje(t_memoria_dinamica*);
t_memoria_dinamica* seleccionar_particion_victima_de_reemplazo();
bool tiene_siguiente(uint32_t);
uint64_t timestamp(void);
void sig_handler();
bool mensaje_recibido_por_todos(void*, t_list*);
void establecer_tiempo_de_carga(t_mensaje*);
void actualizar_ultima_referencia(t_mensaje*);
void dump_de_memoria();
void liberar_mensaje_de_memoria(t_mensaje*);
uint32_t crear_id_nodo();
void main_hilo_mensaje(t_suscripcion*, t_mensaje*);
void* main_hilo_signal();
void crear_hilo_signal();


// --------- buddy nuevo --------- //
void arrancar_buddy();
t_memoria_buddy* crear_raiz();
t_memoria_buddy* recorrer_first_fit(uint32_t);
t_memoria_buddy* recorrer_best_fit(uint32_t);
void dividir_buddy(t_memoria_buddy*);
void guardar_buddy(t_memoria_buddy*, t_mensaje*, void*);
uint32_t remover_buddy(t_memoria_buddy*);
void reemplazar_buddy(t_mensaje*, void*);
t_memoria_buddy* armar_buddy(uint32_t, uint32_t, t_mensaje*, uint32_t, void*, uint32_t);
uint32_t generar_id_bloque();
uint32_t determinar_exponente(t_mensaje*);
uint32_t obtenerPotenciaDe2(uint32_t);
void limpiar_buddy(t_memoria_buddy*);
void recorrer_segun_algoritmo(uint32_t, t_mensaje*, void*);
t_memoria_buddy* seleccionar_victima_fifo();
t_memoria_buddy* seleccionar_victima_lru();
void consolidar_buddy(uint32_t, t_memoria_buddy*);
bool son_buddies_hermanos(t_memoria_buddy*, t_memoria_buddy*);
t_mensaje* encontrar_mensaje_buddy(uint32_t, op_code);
void dump_info_buddy(void*);
uint32_t obtener_id_buddy(t_memoria_buddy*);
char* obtener_cola_del_mensaje_buddy(t_memoria_buddy*);


