#include "team.h"

int main(void){
	printf("HOLA");

	iniciar_programa();
	//enviar_mensaje(GET_POKEMON, "Pikachu", socket); // se va

	//t_buffer* recibido = recibir_mensaje(socket, strlen("Hola")+ 1);
	log_info(logger, "La ip es : %s", config -> ip_broker);
	log_info(logger, "El port es : %s ", config -> puerto_broker);

	agregar_a_estado(estado_new, config -> entrenadores -> head -> data);
	log_info(logger, "agregado a new");

	if(esta_en_estado(estado_new, config -> entrenadores -> head -> data)){
		log_info(logger, "esta en new");
	}
	cambiar_a_estado(estado_ready, config -> entrenadores -> head -> data);
	log_info(logger, "cambiado a ready");
	if(esta_en_estado(estado_ready, config -> entrenadores -> head -> data)){
		log_info(logger, "esta en ready");
	}

	terminar_programa(/*socket*/);
	return 0;
}

//------------------------------- INICIAR -------------------------------//

void iniciar_programa(){
	iniciar_logger("team.log", "team");
	leer_config();
	inicializar_estados();
	inicializar_semaforos();
	determinar_objetivo_global();

	//iniciar_entrenadores(); // iniciar a los entrenadores
	mapa_pokemons = list_create();
	pedidos_captura = list_create();
	suscribirme_a_colas();
	//iniciar_conexion_game_boy(); abrir socket con el game_boy (pthread_create)
}

void inicializar_estados(){
	estado_new = list_create();
	estado_ready = list_create();
	estado_exec = malloc(sizeof(t_estado_exec));
	estado_block = list_create();
	estado_exit = list_create();
}

void inicializar_semaforos(){
	sem_init(&(estado_exec -> mutex)),0,1);
}

void leer_config(void) {

	config = malloc(sizeof(t_config_team));

	t_config* config_team = config_create("Debug/team.config");

	config -> entrenadores =
		load_entrenadores(
			parsear(config_get_array_value(config_team, "POSICIONES_ENTRENADORES")),
			parsear(config_get_array_value(config_team, "POKEMON_ENTRENADORES")),
			parsear(config_get_array_value(config_team, "OBJETIVOS_ENTRENADORES"))
		);

	config -> tiempo_reconexion = config_get_int_value(config_team, "TIEMPO_RECONEXION");
	config -> retardo_cpu = config_get_int_value(config_team, "RETARDO_CICLO_CPU");
	config -> algoritmo_planificacion = strdup(config_get_string_value(config_team, "ALGORITMO_PLANIFICACION"));
	config -> ip_broker = strdup(config_get_string_value(config_team, "IP_BROKER"));
	config -> quantum = config_get_int_value(config_team, "QUANTUM");
	config -> alpha = config_get_int_value(config_team, "ALPHA");
	config -> puerto_broker = strdup(config_get_string_value(config_team, "PUERTO_BROKER"));
	config -> estimacion_inicial = config_get_int_value(config_team, "ESTIMACION_INICIAL");
	config -> log_file = strdup(config_get_string_value(config_team, "LOG_FILE"));

	config_destroy(config_team);
}

t_list* load_entrenadores(t_list* lista_posiciones, t_list* lista_pokemons, t_list* lista_objetivos) {
	t_list* entrenadores = list_create();
	t_link_element* head_posiciones = lista_posiciones -> head;
	t_link_element* head_pokemons = lista_pokemons -> head;
	t_link_element* head_objetivos = lista_objetivos -> head;

	while(head_posiciones != NULL && head_pokemons != NULL && head_objetivos != NULL){
		t_entrenador* entrenador = malloc(sizeof(t_entrenador));

		// posicion
		uint32_t pos[2];
		t_list* aux_posiciones = head_posiciones -> data;

		pos[0] = atoi(aux_posiciones -> head -> data);
		pos[1] = atoi(aux_posiciones -> head -> next -> data);

		entrenador -> posicion[0] = pos[0];
		entrenador -> posicion[1] = pos[1];

		// pokemons
		entrenador -> pokemons = head_pokemons -> data;

		// objetivos
		entrenador -> objetivos = head_objetivos -> data;

		// entrenador a entrenadores
		list_add(entrenadores, entrenador);

		// actualizo punteros
		head_posiciones = head_posiciones -> next;
		head_pokemons = head_pokemons -> next;
		head_objetivos = head_objetivos -> next;
	}

	liberar_lista_de_lista_de_strings(lista_posiciones);
	free(lista_pokemons);
	free(lista_objetivos);

	return entrenadores;
}

void* parsear(char** datos_de_config) {
	t_list* lista = list_create();
	t_list* lista_lista = list_create();

	while(*datos_de_config != NULL) {
		char** cadena = string_split(*datos_de_config, "|"); // de esto se queja el valgrind pero funca. // VER SI ANDA ASI

		for(char* d = *cadena; d; d=*++cadena) {
			list_add(lista, d);
		}

		//t_list* lista_aux = ; // de esto se queja el valgrind pq se pierde lista_aux pero funca. // VER SI ANDA ASI
		list_add(lista_lista, list_duplicate(lista));
		list_clean(lista);
		datos_de_config++;
	}
	list_destroy(lista); // not sure
	return lista_lista;
}

void cargar_pokemons_a_entrenador(t_list* aux, t_link_element* cabeza, t_list* destino){

	while(cabeza != NULL){

		list_add(destino, cabeza -> data);
		cabeza = cabeza -> next;
	}
}

void determinar_objetivo_global() {
	objetivo_global = list_create();

	void obtener_entrenadores(void* entrenador) {

		t_entrenador* un_entrenador = entrenador;
		t_list* objetivo = un_entrenador -> objetivos;

		void agregar_pokemon_a_objetivos(void* objetivo) {
			list_add(objetivo_global, objetivo);
		}

		list_iterate(objetivo, agregar_pokemon_a_objetivos);
	}

	list_iterate(config -> entrenadores, obtener_entrenadores);

	eliminar_los_que_ya_tengo();
}

void eliminar_los_que_ya_tengo(){


	void eliminar_si_lo_tengo(void* entrenador) {

		t_entrenador* un_entrenador = entrenador;
		t_list* pokemons = un_entrenador -> pokemons;


		void remover_pokemon_objetivo_global(void* pokemon) {

			bool es_el_pokemon(void* un_pokemon){
				char* otro_pokemon = un_pokemon;
				char* otro_pokemon2 = pokemon;

				return string_equals_ignore_case(otro_pokemon, otro_pokemon2);
			}

			list_remove_by_condition(objetivo_global, es_el_pokemon);
		}

		list_iterate(pokemons, remover_pokemon_objetivo_global);
	}

	list_iterate(config -> entrenadores, eliminar_si_lo_tengo);

}

void suscribirme_a_colas() {
	suscribirse_a(APPEARED_POKEMON);
	suscribirse_a(CAUGHT_POKEMON);
	suscribirse_a(LOCALIZED_POKEMON);
}


//TODO
void suscribirse_a(op_code una_cola){
	//uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	//mandar por stream el socket
}

// ------------------------------- ENTRENADORES -------------------------------//

void inciar_entrenadores() {

	void crear_hilo_entrenador(void* un_entrenador) {
		pthread_t hilo;
		t_entrenador* entrenador = un_entrenador;
		agregar_a_estado(estado_new, un_entrenador);
		sem_init(&(entrenador -> sem_contador), 0, 0);
		uint32_t err = pthread_create(&hilo, NULL, operar_entrenador, entrenador);
		if(err != 0){
			log_error(logger, "el hilo no pudo ser creado"); // preguntar si estos logs se pueden hacer
		}
	}

	list_iterate(config -> entrenadores, crear_hilo_entrenador);
}

//TODO
void* operar_entrenador(void* un_entrenador) {
	t_entrenador* entrenador = un_entrenador;
	t_list* estado_actual = estado_new;

	while(estado_actual != estado_exit){

		// ver --  importante
		sem_wait(&(entrenador -> sem_contador));

		sem_wait(&(estado_exec -> mutex));

		agarrar_pokemon(entrenador);

	}

	// falta una banda
	return entrenador;
}

void agarrar_pokemon(t_entrenador* entrenador){

	t_pedido_captura* pedido = buscar_pedido(entrenador);


	while(1){ // ver como hacer para los otros algortimos

		if(pedido -> entrenador -> posicion[0] < pedido -> pokemon -> posicion[0]){
			pedido -> entrenador -> posicion[0] ++;
		} else if(pedido -> entrenador -> posicion[0] > pedido -> pokemon -> posicion[0]){
			pedido -> entrenador -> posicion[0] --;
		} else if(pedido -> entrenador -> posicion[1] < pedido -> pokemon -> posicion[1]){
			pedido -> entrenador -> posicion[1] ++;
		} else if(pedido -> entrenador -> posicion[1] > pedido -> pokemon -> posicion[1]){
			pedido -> entrenador -> posicion[1] --;
		} else{ //misma posicion
			// tirar el catch (agarrar)
			break;
		}

		signal(&(entrenador -> sem_contador));
	}

	// sleep(config -> tiempo_espera)

}

t_pedido_captura* buscar_pedido(t_entrenador* entrenador){

	bool es_pedido_del_entrenador(void* un_pedido){
		t_pedido_captura* pedido = un_pedido;

		return pedido -> entrenador == entrenador;
	}

	return list_find(pedidos_captura, es_pedido_del_entrenador);
}


void planificar_entrenadores(){

	if(mapa_pokemons -> elements_count){

		list_iterate(mapa_pokemons, limpiar_mapa); // borra pokemons cargados en mapa que ya no estan en objetivo global

		t_pedido_captura* pedido = malloc(sizeof(t_pedido_captura));
		pedido -> entrenador = NULL;
		matchear_pokemon_con_entrenador(pedido);

		// aca se planifica al primer (unico en fifo) entrenador de pedidos_captura

		cambiar_a_estado(estado_ready, pedido -> entrenador);

		planificar_segun_algoritmo();

		list_add(pedidos_captura, pedido);
		free(pedido);

	} else{
		// contempla el caso de deadlock
	}

}

void planificar_segun_algoritmo(){

	if(string_equals_ignore_case(config -> algoritmo_planificacion, "FIFO")){

		planificar_fifo();

	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "RR")){


	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-CD")){


	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-SD")){ // va a funcionar igual que fifo?

	}


}

void planificar_fifo(){

	t_pedido_captura* pedido = malloc(sizeof(t_pedido_captura));
	pedido -> entrenador = NULL;
	matchear_pokemon_con_entrenador(pedido);

	cambiar_a_estado(estado_ready, pedido -> entrenador);

	// ver
	cambiar_a_exec(pedido -> entrenador);

	for(int i = 0; i < pedido -> distancia; i++){
		sem_post(&(pedido -> entrenador -> sem_contador));
	}


	list_add(pedidos_captura, pedido);
	free(pedido);

}

void limpiar_mapa(void* pokemon) {

	list_remove_and_destroy_by_condition(mapa_pokemons, no_esta_en_objetivo, destruir_pokemon_mapa);
}

bool no_esta_en_objetivo(void* pokemon){

	bool es_el_pokemon(void* otro_pokemon){
		t_pokemon_mapa* un_pokemon = otro_pokemon;

		 return pokemon == un_pokemon -> nombre;
	}

	return list_find(objetivo_global, es_el_pokemon);
}

void destruir_pokemon_mapa(void* un_pokemon){
	t_pokemon_mapa* pokemon = un_pokemon;
	free(pokemon -> nombre);
	free(pokemon); // acordarse de hacer los malloc cuando se cargan al mapa
}

uint32_t distancia(uint32_t pos1[2], uint32_t pos2[2]){

	return abs(pos1[0] - pos2[0]) + abs(pos1[1] - pos2[1]);
}

void matchear_pokemon_con_entrenador(t_pedido_captura* pedido){

	void hallar_match(void* un_entreador){
		t_entrenador* entrenador = un_entreador;
		t_pedido_captura* pedido_aux = malloc(sizeof(t_pedido_captura));
		pedido_aux -> entrenador = NULL;

		void matchear_con_entrenador(void* un_pokemon){
			t_pokemon_mapa* pokemon = un_pokemon;

			if(pedido_aux -> entrenador == NULL){
				pedido_aux -> entrenador = entrenador;
				pedido_aux -> pokemon = pokemon;
				pedido_aux -> distancia = distancia(pedido_aux -> entrenador, pedido_aux -> pokemon);

			} else if(distancia(entrenador -> posicion, pokemon -> posicion) <
						distancia(pedido_aux -> entrenador -> posicion, pedido_aux -> pokemon -> posicion)){

				pedido_aux -> entrenador = entrenador;
				pedido_aux -> pokemon = pokemon;
				pedido_aux -> distancia = distancia(pedido_aux -> entrenador, pedido_aux -> pokemon);
			}
		}

		list_iterate(mapa_pokemons, matchear_con_entrenador);

		if(pedido -> entrenador == NULL){
			pedido -> entrenador = pedido_aux -> entrenador;
			pedido -> pokemon = pedido_aux -> pokemon;
			pedido -> distancia = distancia(pedido -> entrenador, pedido -> pokemon);

		} else if(distancia(pedido -> entrenador -> posicion, pedido -> pokemon -> posicion) >
					distancia(pedido_aux -> entrenador -> posicion, pedido_aux -> pokemon -> posicion)){

			pedido -> entrenador = pedido_aux -> entrenador;
			pedido -> pokemon = pedido_aux -> pokemon;
			pedido -> distancia = distancia(pedido -> entrenador, pedido -> pokemon);
		}

		free(pedido_aux);
	}

	list_iterate(config -> entrenadores, hallar_match);
}

// ------------------------------- ESTADOS ------------------------------- //

void agregar_a_estado(t_list* estado, t_entrenador* entrenador) {
	list_add(estado, entrenador);
}

void eliminar_de_estado(t_list* estado, t_entrenador* entrenador) {

	bool es_el_entrenador(void* un_entrenador) {

		return un_entrenador == entrenador;
	}

	list_remove_by_condition(estado, es_el_entrenador);
}

void cambiar_a_estado(t_list* estado, t_entrenador* entrenador) {
	t_list* estado_actual;
	t_list* estados_a_buscar = list_create();

	if(estado == estado_ready) {

		list_add(estados_a_buscar, estado_new);
		list_add(estados_a_buscar, estado_exec);
		list_add(estados_a_buscar, estado_block);
		estado_actual = buscar_en_estados(estados_a_buscar, entrenador);

	} else if(estado == estado_exec){

		if(esta_en_estado(estado_ready, entrenador)){
			estado_actual = estado_ready;
		}
	} else if(estado == estado_block){

		if(esta_en_estado(estado_exec, entrenador)){
			estado_actual = estado_exec;
		}
	} else if(estado == estado_exit){

		list_add(estados_a_buscar, estado_exec);
		list_add(estados_a_buscar, estado_block);
		estado_actual = buscar_en_estados(estados_a_buscar, entrenador);

	} else {

		log_error(logger, "ESTOY TRATANDO DE CAMBIAR A UN ESTADO QUE NO DEBERIA.");
	}

	list_destroy(estados_a_buscar);
	eliminar_de_estado(estado_actual, entrenador);
	agregar_a_estado(estado, entrenador);
}

t_list* buscar_en_estados(t_list* estados_a_buscar, t_entrenador* entrenador) {

	t_link_element* cabeza = estados_a_buscar -> head;

	while(cabeza != NULL) {

		if(esta_en_estado(cabeza -> data, entrenador)) {

			return cabeza -> data;
		}
		cabeza = cabeza -> next;
	}

	return NULL;

}

bool esta_en_estado(t_list* estado, t_entrenador* entrenador) {

	bool es_el_entrenador(void* un_entrenador) {

		return un_entrenador == entrenador;
	}

	if(list_find(estado, es_el_entrenador)) {

		return 1;

	} else{

		return 0;
	}
}

//TODO
void process_request(uint32_t cod_op, uint32_t cliente_fd) {
	uint32_t size;
	void* msg;

	log_info(logger,"Codigo de operacion %d", cod_op);
	/*
	switch (cod_op) {
		case GET_POKEMON:
			msg = malloc(sizeof(t_get_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(GET_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case LOCALIZED_POKEMON:
			msg = malloc(sizeof(t_localized_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(LOCALIZED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CAUGHT_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(CAUGHT_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case APPEARED_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(APPEARED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;

		/*case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}*/
}

// ------------------------------- TERMINAR -------------------------------//

void liberar_config() {
	liberar_entrenadores();
	free(config -> algoritmo_planificacion);
	free(config -> log_file);
	free(config -> ip_broker);
	free(config -> puerto_broker);
	free(config);
}

void liberar_lista_de_lista_de_strings(t_list* lista_de_lista) {

	void destruir_lista_de_strings(void* lista_de_strings) {

		list_destroy(lista_de_strings);
	}

	list_destroy_and_destroy_elements(lista_de_lista, destruir_lista_de_strings);
}

void liberar_entrenadores() {

	void destruir_entrenador(void* un_entrenador) {

		t_entrenador* entrenador = un_entrenador;

		list_destroy(entrenador -> pokemons);
		list_destroy(entrenador -> objetivos);

		free(entrenador);
	}

	list_destroy_and_destroy_elements(config -> entrenadores, destruir_entrenador);
}

void liberar_estados() {
	list_destroy(estado_new);
	list_destroy(estado_ready);
	sem_destroy(estado_exec -> mutex)
	free(estado_exec);
	list_destroy(estado_block);
	list_destroy(estado_exit);
}

void terminar_programa(/*uint32_t conexion*/) {
	list_destroy(objetivo_global); // puede ser solo free
	free(mapa_pokemons); // ver, se va destruyendo como objetivo global?
	liberar_logger();
	//liberar_conexion(conexion);
	liberar_config();
	liberar_estados();
}
