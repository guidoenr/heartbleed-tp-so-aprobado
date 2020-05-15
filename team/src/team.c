#include "team.h"

int main(void) {

	iniciar_programa();

	log_info(logger, "La ip es: %s", config -> ip_broker);
	log_info(logger, "El port es: %s", config -> puerto_broker);

	t_pokemon_mapa* pikachu1 = malloc(sizeof(t_pokemon_mapa));
	pikachu1 -> nombre = "Pikachu";
	pikachu1 -> posicion[0] = 0;
	pikachu1 -> posicion[1] = 0;
	pikachu1 -> cantidad = 1;
	list_add(mapa_pokemons, pikachu1);
	t_pokemon_mapa* pikachu2 = malloc(sizeof(t_pokemon_mapa));
	pikachu2 -> nombre = "Pikachu";
	pikachu2 -> posicion[0] = 8;
	pikachu2 -> posicion[1] = 8;
	pikachu2 -> cantidad = 1;
	list_add(mapa_pokemons, pikachu2);
	t_pokemon_mapa* charmander = malloc(sizeof(t_pokemon_mapa));
	charmander -> nombre = "Charmander";
	charmander -> posicion[0] = 12;
	charmander -> posicion[1] = 12;
	charmander -> cantidad = 1;
	list_add(mapa_pokemons, charmander);
	while(1) {
		planificar_entrenadores();
		ejecutar_fifo_o_rr();
	}
	/*
	esperando_caught = 0;
	t_pedido_captura* pedido = malloc(sizeof(t_pedido_captura));
	pedido = buscar_pedido(config -> entrenadores -> head -> data);

	while(pedido -> entrenador -> pasos_a_moverse > 0 && buscar_en_estados(estados, pedido -> entrenador) == estado_exec) {

		agarrar_pokemon(pedido);
	}

	if(!esperando_caught && pedido -> entrenador -> pasos_a_moverse <= 0) { // RR
		cambiar_a_estado(estado_ready, pedido -> entrenador);
		log_info(logger, "el entrenador termino su quantum y vuelve a ready");
	} else if(pedido -> entrenador -> pasos_a_moverse <= 0) { // FIFO o SJF Sin Desalojo o ultimo paso del RR

		while(esperando_caught); // settear a 0 en el process request
		procesar_caught(pedido);

		if(cumplio_objetivo_personal(pedido -> entrenador)) {

			cambiar_a_estado(estado_exit, pedido -> entrenador);
			log_info(logger, "el entrenador cumplio su objetivo personal y se mueve a exit");

		} else {

			cambiar_a_estado(estado_block, pedido -> entrenador);
			log_info(logger, "el entrenador termino de procesar el caught y se mueve a block");
		}
	}
	*/


	terminar_programa(/*socket*/);
	return 0;
}

//------------------------------- INICIAR -------------------------------//

void iniciar_programa() {
	iniciar_logger("team.log", "team");
	leer_config();
	inicializar_estados();
	//inicializar_semaforos();
	determinar_objetivo_global();


	esperando_caught = 0;


	iniciar_entrenadores();
	mapa_pokemons = list_create();
	pedidos_captura = list_create();
	//suscribirme_a_colas();
	//iniciar_conexion_game_boy(); abrir socket con el game_boy (pthread_create)
}

void inicializar_estados() {
	estado_new = list_create();
	estado_ready = list_create();
	estado_exec = list_create();
	estado_block = list_create();
	estado_exit = list_create();
	estados = list_create();
	list_add(estados, estado_new);
	list_add(estados, estado_ready);
	list_add(estados, estado_exec);
	list_add(estados, estado_block);
	list_add(estados, estado_exit);
}

void inicializar_semaforos() {
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

	while(head_posiciones != NULL && head_pokemons != NULL && head_objetivos != NULL) {
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

void cargar_pokemons_a_entrenador(t_list* aux, t_link_element* cabeza, t_list* destino) {

	while(cabeza != NULL) {

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

void eliminar_los_que_ya_tengo() {


	void eliminar_si_lo_tengo(void* entrenador) {

		t_entrenador* un_entrenador = entrenador;
		t_list* pokemons = un_entrenador -> pokemons;


		void remover_pokemon_objetivo_global(void* pokemon) {

			bool es_el_pokemon(void* un_pokemon) {
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
void suscribirse_a(op_code una_cola) {
	//uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	//mandar por stream el socket
}

// ------------------------------- ENTRENADORES -------------------------------//

void iniciar_entrenadores() {

	void crear_hilo_entrenador(void* un_entrenador) {
		pthread_t hilo;
		t_entrenador* entrenador = un_entrenador;
		agregar_a_estado(estado_new, entrenador);
		entrenador -> pasos_a_moverse = 0;
		sem_init(&(entrenador -> sem_binario), 0, 0);
		uint32_t err = pthread_create(&hilo, NULL, operar_entrenador, entrenador);
		if(err != 0) {
			log_error(logger, "el hilo no pudo ser creado"); // preguntar si estos logs se pueden hacer
		}
	}

	list_iterate(config -> entrenadores, crear_hilo_entrenador);
}

//TODO
void* operar_entrenador(void* un_entrenador) {
	t_entrenador* entrenador = un_entrenador;
	t_list* estado_actual = buscar_en_estados(estados, entrenador);

	while(estado_actual != estado_exit) {

		sem_wait(&(entrenador -> sem_binario));
		t_pedido_captura* pedido = buscar_pedido(entrenador);

		while(entrenador -> pasos_a_moverse > 0 && buscar_en_estados(estados, entrenador) == estado_exec) {

			agarrar_pokemon(pedido);
		}

		if(!esperando_caught && entrenador -> pasos_a_moverse <= 0) { // RR
			cambiar_a_estado(estado_ready, entrenador);
			log_info(logger, "el entrenador termino su quantum y vuelve a ready");
		} else if(entrenador -> pasos_a_moverse <= 0) { // FIFO o SJF Sin Desalojo o ultimo paso del RR

			while(esperando_caught); // settear a 0 en el process request
			procesar_caught(pedido);

			if(cumplio_objetivo_personal(entrenador)) {

				cambiar_a_estado(estado_exit, entrenador);
				log_info(logger, "el entrenador cumplio su objetivo personal y se mueve a exit");

			} else {

				cambiar_a_estado(estado_block, entrenador);
				log_info(logger, "el entrenador termino de procesar el caught y se mueve a block");
			}
		}

		estado_actual = buscar_en_estados(estados, entrenador);
	}

	return entrenador;
}

bool cumplio_objetivo_personal(t_entrenador* entrenador) {

	uint32_t cumplio_objetivo = 1;

	void chequear_cantidad_de_pokemons(void* un_pokemon) {
		char* pokemon = un_pokemon;

		bool es_el_pokemon(void* otro_pokemon) {
			char* another_pokemon = otro_pokemon;
			return string_equals_ignore_case(another_pokemon, pokemon);
		}

		if(list_count_satisfying(entrenador -> pokemons, es_el_pokemon) !=
			list_count_satisfying(entrenador -> objetivos, es_el_pokemon)) {
			cumplio_objetivo = 0;
		}
	}

	list_iterate(entrenador -> pokemons, chequear_cantidad_de_pokemons);

	return cumplio_objetivo;
}

void procesar_caught(t_pedido_captura* pedido){

	log_info(logger, "ENTRE A PROCESAR CAUGHT");
	resultado_caught = 1;
	if(resultado_caught) { // settear a lo que corresponda en el process_request ANTES de cambiar el esperando_caught
		log_info(logger, "TAMBIEN AL IF ;)");
		list_add(pedido -> entrenador -> pokemons, pedido -> pokemon -> nombre);
	} // que pasa si el el caught es 0

	// sacar el pedido de los pedidos
	eliminar_pedido(pedido);

}

void eliminar_pedido(t_pedido_captura* pedido) {

	bool es_el_pedido(void* un_pedido) {
		t_pedido_captura* otro_pedido = un_pedido;
		if(otro_pedido -> entrenador == pedido -> entrenador) {
			log_info(logger, "SON IGUALES PAPI SON IGUALES PAPI");
		}

		return otro_pedido -> entrenador == pedido -> entrenador && otro_pedido -> pokemon == pedido -> pokemon;
	}

	list_remove_and_destroy_by_condition(pedidos_captura, es_el_pedido, destruir_pedido);
}

void destruir_pedido(void* one_pedido) {
	t_pedido_captura* another_pedido = one_pedido;
	destruir_pokemon(another_pedido -> pokemon);
	free(another_pedido );
}

void destruir_pokemon(t_pokemon_mapa* pokemon) {
	//free(pokemon -> nombre); FALTA MALLOC AL AGREGAR
	free(pokemon);
}


void agarrar_pokemon(t_pedido_captura* pedido) {

	if(pedido -> entrenador -> posicion[0] < pedido -> pokemon -> posicion[0]) {
		pedido -> entrenador -> posicion[0] ++;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else if(pedido -> entrenador -> posicion[0] > pedido -> pokemon -> posicion[0]) {
		pedido -> entrenador -> posicion[0] --;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else if(pedido -> entrenador -> posicion[1] < pedido -> pokemon -> posicion[1]) {
		pedido -> entrenador -> posicion[1] ++;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else if(pedido -> entrenador -> posicion[1] > pedido -> pokemon -> posicion[1]) {
		pedido -> entrenador -> posicion[1] --;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else { // misma posicion
		// tirar el catch (agarrar)
		log_info(logger, "mande el catch para un %s en la posicion [%d,%d]",
				pedido -> pokemon -> nombre, pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);

		esperando_caught = 1;
		esperando_caught = 0; //ESTA ES PARA TEST MIENTRAS NO TENGAMOS BROKER
	}

	pedido -> entrenador -> pasos_a_moverse --;
}

t_pedido_captura* buscar_pedido(t_entrenador* entrenador) {

	bool es_pedido_del_entrenador(void* un_pedido) {
		t_pedido_captura* pedido = un_pedido;

		return pedido -> entrenador == entrenador;
	}

	return list_find(pedidos_captura, es_pedido_del_entrenador);
}


void planificar_entrenadores() {

	if(mapa_pokemons -> elements_count && (estado_new -> head || estado_block -> head)) {

		t_pedido_captura* pedido = malloc(sizeof(t_pedido_captura));
		armar_pedido(pedido);

		eliminar_pokemon_de_mapa(pedido -> pokemon);

		cambiar_a_estado(estado_ready, pedido -> entrenador);
		log_info(logger, "entrenador cambiado a estado ready con su pedido de captura");


		pedido -> entrenador -> pasos_a_moverse = pedido -> distancia + 1; // NO APLICA A RR

		//free(pedido); -- VER CON VALGRIND

		planificar_segun_algoritmo();

	} else {
		// contempla el caso de deadlock
	}
}

void armar_pedido(t_pedido_captura* pedido) {

	pedido -> entrenador = NULL;
	pedido -> pokemon = mapa_pokemons -> head -> data;
	pedido -> distancia = -1;
	matchear_pokemon_con_entrenador(pedido);
	list_add(pedidos_captura, pedido);
}

void planificar_segun_algoritmo() {

	if(string_equals_ignore_case(config -> algoritmo_planificacion, "FIFO")) {

		//ejecutar_fifo_o_rr(); // COMO EL ORTO LLAMADA ACA

	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "RR")) {


	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-CD")) {


	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-SD")) {

	} else {
		log_error(logger, "algoritmo de planificacion recibido: %s", config -> algoritmo_planificacion);
	}
}

void ejecutar_fifo_o_rr() { // ejecutar_un_entrenador, planificar_alg va a ser otra funcion

	if(!estado_exec -> head && estado_ready -> head) {
		t_entrenador* entrenador = estado_ready -> head -> data;

		cambiar_a_estado(estado_exec, entrenador);
		log_info(logger, "entrenador cambiado a estado exec");
		sem_post(&(entrenador -> sem_binario));
	}
}

void eliminar_pokemon_de_mapa(t_pokemon_mapa* pokemon) {

	bool pokemon_a_eliminar(void* un_pokemon) {
		t_pokemon_mapa* otro_pokemon = un_pokemon;
		return pokemon -> posicion == otro_pokemon -> posicion &&
				string_equals_ignore_case(pokemon -> nombre, otro_pokemon -> nombre);
	}
	t_pokemon_mapa* pokemon_a_remover = list_find(mapa_pokemons, pokemon_a_eliminar);


	if(pokemon_a_remover -> cantidad > 1) {
		pokemon_a_remover -> cantidad --;
	} else {
		list_remove_by_condition(mapa_pokemons, pokemon_a_eliminar);
	}
}

void limpiar_mapa(void* pokemon) {

	list_remove_and_destroy_by_condition(mapa_pokemons, no_esta_en_objetivo, destruir_pokemon_mapa);
}

bool no_esta_en_objetivo(void* pokemon) {

	bool es_el_pokemon(void* otro_pokemon) {
		t_pokemon_mapa* un_pokemon = otro_pokemon;

		 return pokemon == un_pokemon -> nombre;
	}

	return list_find(objetivo_global, es_el_pokemon);
}

void destruir_pokemon_mapa(void* un_pokemon) {
	t_pokemon_mapa* pokemon = un_pokemon;
	free(pokemon -> nombre);
	free(pokemon); // acordarse de hacer los malloc cuando se cargan al mapa
}

uint32_t distancia(uint32_t pos1[2], uint32_t pos2[2]) {

	return abs(pos1[0] - pos2[0]) + abs(pos1[1] - pos2[1]);
}


void matchear_pokemon_con_entrenador(t_pedido_captura* pedido) {

	void hallar_match(void* un_entrenador) {

		t_entrenador* entrenador = un_entrenador;

		uint32_t distancia_parcial = distancia(entrenador -> posicion, pedido -> pokemon -> posicion);

		if (pedido -> distancia == -1 || distancia_parcial < pedido -> distancia) {
			pedido -> entrenador = entrenador;
			pedido -> distancia = distancia_parcial;
		}
	}

	t_list* entrenadores_para_ready = list_create();
	list_add_all(entrenadores_para_ready, estado_new);
	list_add_all(entrenadores_para_ready, estado_block);

	remover_entrenadores_en_deadlock(entrenadores_para_ready);

	list_iterate(entrenadores_para_ready, hallar_match);

	list_destroy(entrenadores_para_ready);
}

void remover_entrenadores_en_deadlock(t_list* entrenadores_para_ready){

	void eliminar_entrenador(void* un_entrenador){
		t_entrenador* entrenador = un_entrenador;

		bool es_el_entrenador(void* another_entrenador){

			t_entrenador* otro_entrenador = another_entrenador;

			return otro_entrenador == entrenador;
		}

		if(entrenador -> pokemons -> elements_count == entrenador -> objetivos -> elements_count ){

			list_remove_by_condition(entrenadores_para_ready, es_el_entrenador);
		}
	}


	list_iterate(entrenadores_para_ready, eliminar_entrenador);
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

	} else if(estado == estado_exec) {

		if(esta_en_estado(estado_ready, entrenador)) {
			estado_actual = estado_ready;
		}
	} else if(estado == estado_block) {

		if(esta_en_estado(estado_exec, entrenador)) {
			estado_actual = estado_exec;
		}
	} else if(estado == estado_exit) {

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
	list_destroy(estado_exec);
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
