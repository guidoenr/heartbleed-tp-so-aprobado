#include "team.h"

int main(void){
	printf("HOLA");
	estado_new = list_create();
	list_add(estado_new, "HOLA");
	iniciar_programa();
	int socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	enviar_mensaje(GET_POKEMON, "Pikachu", socket); // se va

	//t_buffer* recibido = recibir_mensaje(socket, strlen("Hola")+ 1);
	log_info(logger, "El ip es : %s", config -> ip_broker);
	log_info(logger, "El port es : %s ", config -> puerto_broker);
	terminar_programa(socket);


	/* iniciar_logger("team.log", "team");
	inicializar_estados();
	int num1 = 1, num2 = 2, num9 = 9;
	agregar_a_estado(estado_new, &num1);
	agregar_a_estado(estado_new, &num2);
	agregar_a_estado(estado_new, &num9);
	log_info(logger, "new: %d", estado_new -> head -> data);

	if(esta_en_estado(estado_new, &num2)){
		log_info(logger, "esta en new");
	}

	int* data = (int*)(estado_ready -> head -> data);
	cambiar_a_estado(estado_ready, &num9);
	log_info(logger, "ready: %d", *data); */


	return 0;
}

void iniciar_programa(){
	iniciar_logger("team.log", "team");
	leer_config();
	inicializar_estados();
	obtener_objetivo_global();

	printf("asd");
	//crear_hilos_entrenadores(); // iniciar a los entrenadores
	//iniciar_conexion_game_boy(); abrir socket con el game_boy (pthread_create)
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
	config -> algoritmo_planificacion = config_get_string_value(config_team, "ALGORITMO_PLANIFICACION");
	config -> ip_broker = config_get_string_value(config_team, "IP_BROKER");
	config -> puerto_broker = config_get_string_value(config_team, "PUERTO_BROKER");
	config -> estimacion_inicial = config_get_int_value(config_team, "ESTIMACION_INICIAL");
	config -> log_file = config_get_string_value(config_team, "LOG_FILE");

	config_destroy(config_team);
}

void* parsear(char** datos_de_config) {
	t_list* lista = list_create();
	t_list* lista_lista = list_create();

	/*char e;
	char* palabra = "";
	for (char* c = *datos_de_config; c; c=*++datos_de_config) {*/
	while(*datos_de_config != NULL) {
		char** cadena = string_split(*datos_de_config, "|");

		for (char* d = *cadena; d; d=*++cadena) {
			list_add(lista, d);
		}


		/*
		for (char* d = c; d; d++) {
			e = *d;

			if(e != '|' && e) {
				palabra = append(palabra, e);
			} else {
				list_add(lista, palabra);
				palabra = ""; // limpiar char*
			}
			if(!e){
				break;
			}
		}*/
		t_list* lista_aux = list_duplicate(lista);
		list_add(lista_lista, lista_aux);
		list_clean(lista);
		datos_de_config++;
	}
	list_destroy(lista); // not sure
	return lista_lista;
}

char* append(const char *palabra, char c) {
    int largo = strlen(palabra);
    char buf[largo + 2];
    strcpy(buf, palabra);
    buf[largo] = c;
    buf[largo + 1] = 0;
    return strdup(buf);
}

void inciar_entrenadores() {

	void crear_hilo_entrenador(void* entrenador) {
		pthread_t hilo;
		t_entrenador* un_entrenador = entrenador;
		agregar_a_estado(estado_new, &(un_entrenador -> id));
		int err = pthread_create(&hilo, NULL, operar_entrenador, entrenador);
		if(err != 0){
			log_error(logger, "el hilo no pudo ser creado"); // preguntar si estos logs se pueden hacer
		}
	}

	list_iterate(config -> entrenadores, crear_hilo_entrenador);
}

void* operar_entrenador(void* un_entrenador) {
	t_entrenador* entrenador = un_entrenador;
	cambiar_a_estado(estado_ready, &(entrenador -> id));



	// falta una banda

	return entrenador;
}

void agregar_a_estado(t_list* estado, int* id_entrenador) {
	list_add(estado, id_entrenador);
}

void eliminar_de_estado(t_list* estado, int* id_entrenador) {

	bool es_el_entrenador(void* id_ent) {

		return *(int*)id_ent == *id_entrenador;
	}

	list_remove_by_condition(estado, es_el_entrenador);
}

void cambiar_a_estado(t_list* estado, int* id_entrenador) {
	t_list* estado_actual;
	t_list* estados_a_buscar = list_create();

	if(estado == estado_ready) {

		list_add(estados_a_buscar, estado_new);
		list_add(estados_a_buscar, estado_exec);
		list_add(estados_a_buscar, estado_block);
		estado_actual = buscar_en_estados(estados_a_buscar, id_entrenador);

	} else if(estado == estado_exec){

		if(esta_en_estado(estado_ready, id_entrenador)){
			estado_actual = estado_ready;
		}
	} else if(estado == estado_block){

		if(esta_en_estado(estado_exec, id_entrenador)){
			estado_actual = estado_exec;
		}
	} else if(estado == estado_exit){

		list_add(estados_a_buscar, estado_exec);
		list_add(estados_a_buscar, estado_block);
		estado_actual = buscar_en_estados(estados_a_buscar, id_entrenador);

	} else {

		log_error(logger, "ESTOY TRATANDO DE CAMBIAR A UN ESTADO QUE NO DEBERIA: id: %d", &id_entrenador);
	}

	list_destroy(estados_a_buscar);
	eliminar_de_estado(estado_actual, id_entrenador);
	agregar_a_estado(estado, id_entrenador);
}

bool esta_en_estado(t_list* estado, int* id_entrenador) {

	bool es_el_entrenador(void* id_ent) {

		return *(int*)id_ent == *id_entrenador;
	}

	if(list_find(estado, es_el_entrenador)) {

		return 1;

	} else{

		return 0;
	}
}

t_list* buscar_en_estados(t_list* estados_a_buscar, int* id_entrenador) {

	t_link_element* cabeza = estados_a_buscar -> head;

	while(cabeza != NULL) {

		if(esta_en_estado(cabeza -> data, id_entrenador)) {

			return cabeza -> data;
		}
		cabeza = cabeza -> next;
	}

	return NULL;

}

t_list* load_entrenadores(t_list* lista_posiciones, t_list* lista_pokemons, t_list* lista_objetivos) {
	t_list* entrenadores = list_create();
	t_link_element* head_posiciones = lista_posiciones -> head;
	t_link_element* head_pokemons = lista_pokemons -> head;
	t_link_element* head_objetivos = lista_objetivos -> head;

	int id_entrenador = 1;

	while(head_posiciones != NULL && head_pokemons != NULL && head_objetivos != NULL){
		t_entrenador* entrenador = malloc(sizeof(t_entrenador));

		entrenador -> id = id_entrenador;
		entrenador -> pokemons = list_create();
		entrenador -> objetivos = list_create();

		// posicion
		int pos[2];
		t_list* aux_posiciones = head_posiciones -> data;

		pos[0] = atoi(aux_posiciones -> head -> data);
		pos[1] = atoi(aux_posiciones -> head -> next -> data);

		entrenador -> posicion[0] = pos[0];
		entrenador -> posicion[1] = pos[1];

		// pokemons
		t_list* aux_pokemons = head_pokemons -> data;
		t_link_element* cabeza_pokemons = aux_pokemons -> head;

		cargar_pokemons_a_entrenador(aux_pokemons, cabeza_pokemons, entrenador -> pokemons);

		// objetivos
		t_list* aux_objetivos = head_objetivos -> data;
		t_link_element* cabeza_objetivos = aux_objetivos -> head;

		cargar_pokemons_a_entrenador(aux_objetivos, cabeza_objetivos, entrenador -> objetivos);

		// entrenador a entrenadores
		list_add(entrenadores, entrenador);

		// actualizo punteros
		id_entrenador++;

		head_posiciones = head_posiciones -> next;
		head_pokemons = head_pokemons -> next;
		head_objetivos = head_objetivos-> next;
	}

	//REVER ESTOS DESTROYS:
	list_destroy(lista_posiciones);
	liberar_lista_de_lista_de_strings(lista_pokemons);
	liberar_lista_de_lista_de_strings(lista_objetivos);

	return entrenadores;
}

void obtener_objetivo_global() {
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
}


void cargar_pokemons_a_entrenador(t_list* aux, t_link_element* cabeza, t_list* destino){

	while(cabeza != NULL){

		list_add(destino, cabeza -> data);
		cabeza = cabeza -> next;
	}
}

void inicializar_estados(){
	 estado_new = list_create();
	 estado_ready = list_create();
	 estado_exec = list_create();
	 estado_block = list_create();
	 estado_exit = list_create();
}


void process_request(int cod_op, int cliente_fd) {
	int size;
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

		list_destroy_and_destroy_elements(lista_de_strings, free);
		list_destroy(lista_de_strings);

	}

	list_destroy_and_destroy_elements(lista_de_lista, destruir_lista_de_strings);
	list_destroy(lista_de_lista);
}

void liberar_entrenadores() {

	void destruir_entrenador(void* un_entrenador) {

		t_entrenador* entrenador = un_entrenador;

		list_destroy(entrenador -> pokemons);
		list_destroy(entrenador -> objetivos);

		free(un_entrenador);
	}

	list_destroy_and_destroy_elements(config -> entrenadores, destruir_entrenador);
	list_destroy(config -> entrenadores);
}

void terminar_programa(int conexion) {
	list_destroy(objetivo_global);
	liberar_logger();
	liberar_conexion(conexion);
	liberar_config();
}
