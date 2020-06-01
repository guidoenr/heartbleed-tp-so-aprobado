#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <semaphore.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

// agregar un id 0 al mensaje a mandar al broker

typedef struct {
	sem_t sem_binario;
	uint32_t posicion[2];
	t_list* pokemons;
	t_list* objetivos;
	uint32_t pasos_a_moverse;
	uint32_t tire_catch;
	uint32_t resultado_caught;
	sem_t esperar_caught;
} t_entrenador;

typedef struct {
	char* nombre;
	uint32_t posicion[2];
	uint32_t cantidad;
} t_pokemon_mapa;

typedef struct {
	t_entrenador* entrenador;
	t_pokemon_mapa* pokemon;
	uint32_t distancia;
} t_pedido_captura;


typedef struct {
	t_list* entrenadores;
	uint32_t tiempo_reconexion;
	uint32_t retardo_cpu;
	char* algoritmo_planificacion;
	uint32_t quantum;
	uint32_t alpha;
	char* ip_broker;
	char* puerto_broker;
	uint32_t estimacion_inicial;
	char* log_file;
} t_config_team;

t_config_team* config;
t_log* logger;

// utils
char* append(const char*, char);
void* parsear(char**);
uint32_t distancia(uint32_t[2], uint32_t[2]);

// iniciar
void iniciar_programa();
void inicializar_estados();
void leer_config(void);
void suscribirme_a_colas();
void suscribirse_a(op_code);


// entrenadores
void iniciar_entrenadores();
void* operar_entrenador(void*);
void obtener_entrenadores(void*);
t_list* load_entrenadores(t_list*, t_list*, t_list*);
void cargar_pokemons_a_entrenador(t_list*, t_link_element*, t_list*);
void remover_entrenadores_en_deadlock(t_list*);
bool tengo_la_mochila_llena(t_entrenador*);

// mensajes

// segun algoritmo:
int distancia_segun_algoritmo(t_pedido_captura*);
void planificar_segun_algoritmo(t_pedido_captura* pedido);


void procesar_caught(t_pedido_captura*);


// planificacion
t_pedido_captura* buscar_pedido(t_entrenador*);
void* planificar_entrenadores();
void planificar_fifo();
void planificar_rr();
void planificar_sjf_sd();
void planificar_sjf_cd();
void* ejecutar_fifo_o_rr_o_sjf_sd();
void crear_hilo_planificar_entrenadores();

// ejecucion
void agarrar_pokemon(t_pedido_captura*);
void iniciar_hilos_ejecucion();
void crear_hilo_segun_algoritmo();
pthread_t hiloAlgoritmo;
pthread_t hiloEntrenadores;

// estados

t_list* estado_new;
t_list* estado_ready;
t_list* estado_exec;
t_list* estado_block;
t_list* estado_exit;
t_list* estados;

void agregar_a_estado(t_list*, t_entrenador*);
void eliminar_de_estado(t_list*, t_entrenador*);
void cambiar_a_estado(t_list*, t_entrenador*);
t_list* buscar_en_estados(t_list*, t_entrenador*);
bool esta_en_estado(t_list*, t_entrenador*);
int estado_block_replanificable_no_interbloqueado();

//semaforos
sem_t mx_estado_new;
sem_t mx_estado_ready;
sem_t mx_estado_exec;
sem_t mx_estado_block;
sem_t mx_estado_exit;
sem_t entrenadores_ready;
void inicializar_semaforos();


// mapa
t_list* mapa_pokemons;


// pedido
t_list* pedidos_captura;

void eliminar_pedido(t_pedido_captura*);
void destruir_pedido(void*);
void armar_pedido(t_pedido_captura*);
void destruir_pokemon(t_pokemon_mapa*);

void limpiar_mapa(void*);
void destruir_pokemon_mapa(void*);
void matchear_pokemon_con_entrenador(t_pedido_captura*);
void eliminar_pokemon_de_mapa(t_pokemon_mapa*);

// objetivo
t_list* objetivo_global;

void determinar_objetivo_global();
bool no_esta_en_objetivo(void*);
void eliminar_los_que_ya_tengo();
bool cumplio_objetivo_personal(t_entrenador*);

// terminar
void liberar_config();
void liberar_lista_de_lista_de_strings(t_list*);
void liberar_entrenadores();
void terminar_programa(/*uint32_t*/);
void liberar_conexion(uint32_t);
void liberar_estados();
