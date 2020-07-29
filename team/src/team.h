#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <semaphore.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

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
	uint32_t id_espera_catch;
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
	char* ip_team;
	char* puerto_team;
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
void iniciar_hilo_caught();
void iniciar_hilo_localized();
void iniciar_hilo_appeared();


// utils
char* append(const char*, char);
void* parsear(char**);
uint32_t distancia(uint32_t[2], uint32_t[2]);

// iniciar
void iniciar_programa();
void inicializar_estados();
void leer_config(void);
void*suscribirme_a_colas();
void suscribirse_a(op_code);
void inicializar_semaforos();
void iniciar_hilos_ejecucion();
void crear_listas_globales();
void conexion_inicial_broker();


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
bool entrenadores_con_mochila_llena();

// mensajes
uint32_t recibir_id_de_mensaje_enviado(uint32_t);
void enviar_mensaje_catch(t_pedido_captura*);
void procesar_caught(t_pedido_captura*);
void enviar_get_pokemon();
void procesar_localized();
void agregar_localized_al_mapa();
void procesar_mensaje_caught(t_caught_pokemon*);
void procesar_mensaje_appeared(t_appeared_pokemon*);
void levantar_server_broker();
void enviar_ack_broker(uint32_t, op_code);

// segun algoritmo:
int distancia_segun_algoritmo(t_pedido_captura*);
void planificar_segun_algoritmo(t_pedido_captura* pedido);
void conectarse_a_br();


// planificacion
void* planificar_entrenadores();
void planificar_fifo_o_rr(t_pedido_captura*);
void planificar_sjf_sd(t_pedido_captura*);
void planificar_sjf_cd(t_pedido_captura*);
void desalojar_ejecucion();
void ordenar_ready_segun_estimacion();
void* ejecutar_algoritmo();
void planificar_deadlocks();
void loggear_entrenadores_deadlock();

// hilos
pthread_t hilo_algoritmo;
pthread_t hilo_planificar;
pthread_t hilo_game_boy;
pthread_t hilo_broker;
pthread_t hilo_localized;
pthread_t hilo_appeared;
pthread_t hilo_get;
pthread_t hilo_caught;
void crear_hilo_segun_algoritmo();
void crear_hilo_planificar_entrenadores();
void terminar_hilos();
void esperar_hilos_entrenadores();
void iniciar_conexion();
void* iniciar_server_gamecard();
void levantar_server_broker();
void* conexion_con_broker();
void* hilo_mensaje_get(void*);


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
sem_t mx_mapas_objetivos_pedidos;
sem_t sem_cont_mapa;
sem_t sem_cont_entrenadores_a_replanif;
sem_t mx_contexto;
sem_t mx_paquete;

// pedido
t_list* pedidos_captura;
t_list* pedidos_intercambio;

void eliminar_pedido_captura(t_pedido_captura*);
void destruir_pedido_captura(void*);
void eliminar_pedido_intercambio(t_pedido_intercambio*);
void armar_pedido_captura(t_pedido_captura*);

t_pedido_captura* buscar_pedido_captura(t_entrenador*);
t_pedido_intercambio* buscar_pedido_intercambio(t_entrenador*);
void matchear_pokemon_con_entrenador(t_pedido_captura*);
void eliminar_pokemon_de_mapa(t_pokemon_mapa*);
void armar_pedido_intercambio_segun_algoritmo(t_pedido_intercambio*);
void destruir_pedido_intercambio(void*);

// objetivo
t_list* objetivo_global;
t_list* especies_objetivo_global;
t_list* especies_ya_localizadas;
t_list* objetivo_global_pendiente;
t_list* mapa_pokemons;
t_list* mapa_pokemons_pendiente;

void determinar_objetivo_global();
void eliminar_los_que_ya_tengo();
bool cumplio_objetivo_personal(t_entrenador*);
bool comparar_pokemon(void*, void*);
void eliminar_del_objetivo_global(t_pokemon_mapa*);

// mapa
void mover_especie_de_mapa(t_list*, t_list*, char*);
void vaciar_especie_de_mapa(t_list*, char*);


// terminar
void loggear_resultados();
void liberar_config();
void liberar_lista_de_lista_de_strings(t_list*);
void liberar_entrenadores();
void terminar_programa();
void liberar_conexion(uint32_t);
void liberar_estados();
void liberar_listas();
void liberar_semaforos();
