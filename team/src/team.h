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
	sem_t sem_contador;
	uint32_t posicion[2];
	t_list* pokemons;
	t_list* objetivos;
} t_entrenador;

typedef struct {
	char* nombre;
	uint32_t posicion[2];
	uint32_t cantidad;
} t_pokemon_mapa;

typedef struct {
	t_entrenador* entrenador;
	t_pokemon_mapa* pokemon;
} t_pedido_captura;


typedef struct {
	t_list* entrenadores;
	uint32_t tiempo_reconexion;
	uint32_t retardo_cpu;
	char* algoritmo_planificacion;
	char* ip_broker;
	char* puerto_broker;
	uint32_t estimacion_inicial;
	char* log_file;
} t_config_team;

t_config_team* config;
t_log* logger;
t_list* objetivo_global;

t_list* estado_new;
t_list* estado_ready;
t_list* estado_exec;
t_list* estado_block;
t_list* estado_exit;

t_list* estados;
t_list* mapa_pokemons;
t_list* pedidos_captura;

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
void inciar_entrenadores();
void* operar_entrenador(void*);
void obtener_entrenadores(void*);
t_list* load_entrenadores(t_list*, t_list*, t_list*);
void cargar_pokemons_a_entrenador(t_list*, t_link_element*, t_list*);


// estados
void agregar_a_estado(t_list*, t_entrenador*);
void eliminar_de_estado(t_list*, t_entrenador*);
void cambiar_a_estado(t_list*, t_entrenador*);
t_list* buscar_en_estados(t_list*, t_entrenador*);
bool esta_en_estado(t_list*, t_entrenador*);

//semaforos
void inicializar_semaforos();

// mapa
void limpiar_mapa(void*);
void destruir_pokemon_mapa(void*);
void matchear_pokemon_con_entrenador(t_pedido_captura*);

// objetivo
void determinar_objetivo_global();
bool no_esta_en_objetivo(void*);


// terminar
void liberar_config();
void liberar_lista_de_lista_de_strings(t_list*);
void liberar_entrenadores();
void terminar_programa(/*uint32_t*/);
void liberar_conexion(uint32_t);
void liberar_estados();
