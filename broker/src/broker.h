#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"
#include <semaphore.h>


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
} t_config_broker;

typedef struct {
	uint32_t id_mensaje;
	uint32_t id_correlativo;
	void* 	 payload;
	t_list*  suscriptor_enviado;
	t_list*  suscriptor_recibido;
	op_code  codigo_operacion;
} t_mensaje;

typedef struct {
	uint32_t tamanio_mensaje;
	void* 	 payload;
	uint32_t base;
	uint32_t ocupado;
} t_memoria_dinamica;

typedef struct{
	uint32_t libre;
	uint32_t tamanio;
	void* payload;
} t_memoria_buddy;

struct t_node{
	t_memoria_buddy* bloque;
	struct t_node *izquierda;
	struct t_node *derecha;
};

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

//void* memoria_cache;
t_list* memoria_cache;
t_list* memoria_con_particiones;
t_config* config;
t_config_broker* config_broker;
t_log* logger;

sem_t semaforo;
sem_t mutex_id;

uint32_t id_mensaje_univoco;

//Funciones generales
void iniciar_programa			 (void);
void iniciar_semaforos			 (void);
void reservar_memoria			 (void);
void leer_config				 (void);
void terminar_programa			 (t_log*);
void liberar_config				 (t_config_broker*);
void crear_colas_de_mensajes 	 (void);
void crear_listas_de_suscriptores(void);
void liberar_listas				 (void);
void liberar_memoria_cache		 (void);
void destruir_semaforos			 (void);

//Administracion de mensajes
void 	 encolar_mensaje			       			   (t_mensaje*, op_code);
void 	 agregar_mensaje				   			   (uint32_t,uint32_t,void*,uint32_t);
uint32_t generar_id_univoco				   			   (void);
void 	 enviar_mensajes_get			   			   (void);
void 	 enviar_mensajes_catch			   			   (void);
void 	 enviar_mensajes_localized		   			   (void);
void 	 enviar_mensajes_caught			   			   (void);
void 	 enviar_mensajes_appeared		   			   (void);
void 	 enviar_mensaje_get				   			   (void*);
void 	 desencolar_mensaje				   			   (t_mensaje*);
void 	 actualizar_mensaje_confirmado 	   			   ();
void 	 actualizar_mensajes_confirmados   			   (t_ack*);
void     agregar_suscriptor_a_enviados_confirmados	   (t_mensaje*, uint32_t);
void 	 agregar_suscriptor_a_enviados_sin_confirmar   (t_mensaje*, uint32_t);
void	 eliminar_suscriptor_de_enviados_sin_confirmar (t_mensaje*, uint32_t);
bool 	 es_el_mismo_suscriptor			   			   (void*);
void 	 eliminar_mensajes_confirmados	   			   (void);
void 	 borrar_mensajes_confirmados	   			   (op_code, t_list*, t_list*);
void 	 eliminar_mensaje							   (void*);
bool 	 no_tiene_el_mensaje						   (t_mensaje*, uint32_t);

//Suscripciones
void 		   recibir_suscripcion         (t_suscripcion*);
void 		   suscribir_a_cola			   (t_list*, t_suscripcion*, op_code);
bool 		   es_la_misma_suscripcion     (void*);
void 		   informar_mensajes_previos   (t_suscripcion*, op_code);
void 		   descargar_historial_mensajes(op_code, uint32_t);
void 		   destruir_suscripcion 	   (void*);
t_list*		   encontrar_suscriptores	   (void);
void 		   actualizar_suscriptores     (t_mensaje*);
void 		   eliminar_suscriptor		   (void*);
//Para team y game_card
//t_ack* 		   armar_confirmacion_de_recepcion(t_paquete*);

//Memoria
void eliminar_particion_de_memoria(void);
void compactar_memoria 			  (void);
void guardar_en_memoria			  (t_mensaje*);
//void firstFit(int blockSize,int processSize);
uint32_t obtenerPotenciaDe2(uint32_t);
struct t_node* crear_nodo(uint32_t);
void arrancar_buddy();
void asignar_nodo(struct t_node*, void*);
uint32_t recorrer(struct t_node*,uint32_t ,void*);
void ubicar_particion(uint32_t, uint32_t, void* );
void liberar_particion_dinamica(t_memoria_dinamica*);
void iniciar_memoria_particiones(t_list*);
uint32_t encontrar_primer_ajuste(uint32_t);
uint32_t encontrar_mejor_ajuste(uint32_t);
void destruir_particion(void*);
uint32_t encontrar_indice(t_memoria_dinamica*);



