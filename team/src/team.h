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
	uint32_t id;
	pthread_t hilo;
	sem_t sem_binario;
	uint32_t posicion[2];
	t_list* pokemons;
	t_list* objetivos;
	uint32_t pasos_a_moverse;
	uint32_t tire_accion;
	uint32_t resultado_caught;
	sem_t esperar_caught;
	uint32_t estimacion;
	uint32_t ciclos_cpu;
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
	t_entrenador* entrenador_buscando;
	t_entrenador* entrenador_esperando;
	uint32_t distancia;
	char* pokemon_a_dar;
	char* pokemon_a_recibir;
} t_pedido_intercambio;

typedef struct {
	t_list* entrenadores;
	uint32_t tiempo_reconexion;
	uint32_t retardo_cpu;
	char* algoritmo_planificacion;
	uint32_t quantum;
	float alpha;
	char* ip_broker;
	char* puerto_broker;
	uint32_t estimacion_inicial;
	char* log_file;
	uint32_t id_proceso;
} t_config_team;

t_config_team* config;
t_log* logger;

uint32_t deadlocks_totales;
uint32_t deadlocks_resueltos;
uint32_t ciclos_cpu_totales;
uint32_t cambios_de_contexto;

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
void inicializar_semaforos();
void iniciar_hilos_ejecucion();

// entrenadores
void iniciar_entrenadores();
void* operar_entrenador(void*);
void obtener_entrenadores(void*);
t_list* load_entrenadores(t_list*, t_list*, t_list*);
void cargar_pokemons_a_entrenador(t_list*, t_link_element*, t_list*);
void remover_entrenadores_en_deadlock(t_list*);
bool tengo_la_mochila_llena(t_entrenador*);
bool estoy_en_deadlock(t_entrenador*);
char* encontrar_pokemon_faltante(t_entrenador*);
char* encontrar_pokemon_sobrante(t_entrenador*);
bool le_sobra_pokemon(t_entrenador*, char*);
void manejar_desalojo_captura(t_pedido_captura*);
void capturar_pokemon(t_pedido_captura*);
void tradear_pokemon(t_pedido_intercambio*);
void ejecutar_trade(t_pedido_intercambio*);
void asignar_estado_luego_de_trade(t_entrenador*);
void calcular_estimaciones_ready();
bool estoy_esperando_trade(t_entrenador*);

// mensajes

// segun algoritmo:
int distancia_segun_algoritmo(t_pedido_captura*);
void planificar_segun_algoritmo(t_pedido_captura* pedido);


void procesar_caught(t_pedido_captura*);


// planificacion
void* planificar_entrenadores();
void planificar_fifo_o_rr(t_pedido_captura*);
void planificar_sjf_sd(t_pedido_captura*);
void planificar_sjf_cd(t_pedido_captura*);
void desalojar_ejecucion();
void ordenar_ready_segun_estimacion();
void* ejecutar_algoritmo();
void planificar_deadlocks_fifo_o_sjf();
void planificar_deadlocks_rr();

// hilos
pthread_t hilo_algoritmo;
pthread_t hilo_planificar;
pthread_t hilo_game_boy;
void crear_hilo_segun_algoritmo();
void crear_hilo_planificar_entrenadores();
void terminar_hilos();
void terminar_hilos_entrenadores();
void iniciar_conexion_game_boy();


// game boy
void* conexion_con_game_boy();


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
sem_t mx_estados;
sem_t mx_estado_exec;
sem_t mx_desalojo_exec;
sem_t entrenadores_ready;
sem_t sem_cont_mapa;
sem_t sem_cont_entrenadores_a_replanif;
sem_t mx_contexto;
sem_t mx_paquete;


// mapa
t_list* mapa_pokemons;


// pedido
t_list* pedidos_captura;
t_list* pedidos_intercambio;

void eliminar_pedido_captura(t_pedido_captura*);
void destruir_pedido_captura(void*);
void eliminar_pedido_intercambio(t_pedido_intercambio*);
void armar_pedido_captura(t_pedido_captura*);
void destruir_pokemon(t_pokemon_mapa*);

t_pedido_captura* buscar_pedido_captura(t_entrenador*);
t_pedido_intercambio* buscar_pedido_intercambio(t_entrenador*);
void limpiar_mapa(void*);
void destruir_pokemon_mapa(void*);
void matchear_pokemon_con_entrenador(t_pedido_captura*);
void eliminar_pokemon_de_mapa(t_pokemon_mapa*);
t_pedido_intercambio* armar_pedido_intercambio_segun_algoritmo();

// objetivo
t_list* objetivo_global;

void determinar_objetivo_global();
bool no_esta_en_objetivo(void*);
void eliminar_los_que_ya_tengo();
bool cumplio_objetivo_personal(t_entrenador*);
bool comparar_pokemon(void*, void*);


// terminar
void loggear_resultados();
void liberar_config();
void liberar_lista_de_lista_de_strings(t_list*);
void liberar_entrenadores();
void terminar_programa(/*uint32_t*/);
void liberar_conexion(uint32_t);
void liberar_estados();
void liberar_listas();
void liberar_semaforos();
