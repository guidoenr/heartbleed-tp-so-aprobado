#include "team.h"

int main(void) {

	iniciar_programa();

	sem_wait(&fin_programa);
	terminar_programa();
	return 0;
}

//------------------------------- INICIAR -------------------------------//

void iniciar_programa() {
	iniciar_logger("team.log", "team");
	deadlocks_totales = 0;
	deadlocks_resueltos = 0;
	ciclos_cpu_totales = 0;
	cambios_de_contexto = 0;

	leer_config();
	inicializar_estados();
	inicializar_semaforos();
	determinar_objetivo_global();
	crear_listas_globales();
	iniciar_entrenadores();
	iniciar_hilos_ejecucion();

	iniciar_conexion();
	sleep(2);
	conexion_inicial_broker();
}

void crear_listas_globales() {
	mapa_pokemons = list_create();
	pedidos_captura = list_create();
	pedidos_intercambio = list_create();
	especies_ya_localizadas = list_create();
	especies_objetivo_global = list_create();
	objetivo_global_pendiente = list_create();
	mapa_pokemons_pendiente = list_create();
}

void iniciar_hilos_ejecucion() {
	crear_hilo_segun_algoritmo();
	crear_hilo_planificar_entrenadores();
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
	sem_init(&mx_estados, 0, 1);
	sem_init(&mx_estado_exec, 0, 1);
	sem_init(&mx_desalojo_exec, 0, 0);
	sem_init(&entrenadores_ready, 0, 0);
	sem_init(&sem_cont_mapa, 0, 0);
	sem_init(&sem_cont_entrenadores_a_replanif, 0, 0);
	sem_init(&mx_contexto, 0, 1);
	sem_init(&mx_paquete, 0, 1);
	sem_init(&fin_programa, 0, 0);
	sem_init(&semaforo, 0, 1);
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
	config -> puerto_broker = strdup(config_get_string_value(config_team, "PUERTO_BROKER"));
	config -> ip_gameboy = strdup(config_get_string_value(config_team, "IP_GAMEBOY"));
	config -> puerto_gameboy = strdup(config_get_string_value(config_team, "PUERTO_GAMEBOY"));
	config -> quantum = config_get_int_value(config_team, "QUANTUM");
	config -> alpha = atof(config_get_string_value(config_team, "ALPHA"));
	config -> estimacion_inicial = config_get_int_value(config_team, "ESTIMACION_INICIAL");
	config -> log_file = strdup(config_get_string_value(config_team, "LOG_FILE"));
	config -> id_proceso = config_get_int_value(config_team, "ID_PROCESO");

	config_destroy(config_team);
}

t_list* load_entrenadores(t_list* lista_posiciones, t_list* lista_pokemons, t_list* lista_objetivos) {
	t_list* entrenadores = list_create();
	t_link_element* head_posiciones = lista_posiciones -> head;
	t_link_element* head_pokemons = lista_pokemons -> head;
	t_link_element* head_objetivos = lista_objetivos -> head;
	int id = 1;
	while(head_posiciones && head_objetivos) {
		t_entrenador* entrenador = malloc(sizeof(t_entrenador));

		// id
		entrenador -> id = id;
		id++;

		// posicion
		uint32_t pos[2];
		t_list* aux_posiciones = head_posiciones -> data;

		pos[0] = atoi(aux_posiciones -> head -> data);
		pos[1] = atoi(aux_posiciones -> head -> next -> data);

		entrenador -> posicion[0] = pos[0];
		entrenador -> posicion[1] = pos[1];

		// pokemons
		if(!head_pokemons) {
			entrenador -> pokemons = list_create();
		} else {
			entrenador -> pokemons = head_pokemons -> data;
			head_pokemons = head_pokemons -> next;
		}


		// objetivos
		entrenador -> objetivos = head_objetivos -> data;

		// entrenador a entrenadores
		list_add(entrenadores, entrenador);

		// actualizo punteros
		head_posiciones = head_posiciones -> next;
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

void conectarse_a_br(){

	suscribirse_a(LOCALIZED_POKEMON);

	suscribirse_a(APPEARED_POKEMON);

	suscribirse_a(CAUGHT_POKEMON);

	enviar_get_pokemon();
}


void suscribirse_a(op_code cola) {

	uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	t_suscripcion* suscripcion = malloc(sizeof(t_suscripcion));

	if (socket == -1){
			int time = config->tiempo_reconexion;
			log_info(logger,"Intento de reconexion con broker fallido");
			sleep(time);
			log_info(logger,"Inicio de proceso de reintento de comunicacion con el broker");
			suscribirse_a(cola);
	}else {

		suscripcion -> cola_a_suscribir= cola;
		suscripcion -> id_proceso= config -> id_proceso; //ESTE VALOR SE SACA DE CONFIG
		suscripcion -> socket = socket;
		suscripcion -> tiempo_suscripcion = 0; //ESTE VALOR SIEMPRE ES 0

		uint32_t tamanio_suscripcion = size_mensaje(suscripcion, SUBSCRIPTION);

		enviar_mensaje(SUBSCRIPTION, suscripcion, socket, tamanio_suscripcion);
	}

}

void conexion_inicial_broker() {

	uint32_t err = pthread_create(&hilo_broker, NULL, conectarse_a_br, NULL);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}
	pthread_detach(hilo_broker);
}


void iniciar_conexion() {

	uint32_t err = pthread_create(&hilo_game_boy, NULL, conexion_con_game_boy, NULL);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}
	pthread_detach(hilo_game_boy);
}

void* conexion_con_game_boy() {

	iniciar_servidor(config -> ip_gameboy, config -> puerto_gameboy); // puerto robado de la config de gameboy

	return NULL;
}


// ------------------------------- ENTRENADORES -------------------------------//

void iniciar_entrenadores() {

	void crear_hilo_entrenador(void* un_entrenador) {

		t_entrenador* entrenador = un_entrenador;
		agregar_a_estado(estado_new, entrenador);

		entrenador -> pasos_a_moverse = 0;
		entrenador -> tire_accion = 0;
		entrenador -> estimacion = config -> estimacion_inicial;
		entrenador -> ciclos_cpu = 0;
		//entrenador -> socket_mensaje_catch = 0;

		sem_init(&(entrenador -> sem_binario), 0, 0);
		sem_init(&(entrenador -> esperar_caught), 0, 0);

		uint32_t err = pthread_create(&(entrenador -> hilo), NULL, operar_entrenador, (void*) entrenador);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}

		sem_post(&sem_cont_entrenadores_a_replanif);
	}

	list_iterate(config -> entrenadores, crear_hilo_entrenador);

}

void* operar_entrenador(void* un_entrenador) {
	t_entrenador* entrenador = un_entrenador;


	while(buscar_en_estados(estados, entrenador) != estado_exit) {

		sem_wait(&(entrenador -> sem_binario));

		t_pedido_captura* pedido_captura = buscar_pedido_captura(entrenador);

		if(pedido_captura) {

			while(!(entrenador -> tire_accion) && entrenador -> pasos_a_moverse > 0 && buscar_en_estados(estados, entrenador) == estado_exec) {

				capturar_pokemon(pedido_captura);
			}

			manejar_desalojo_captura(pedido_captura);

		} else {
			t_pedido_intercambio* pedido_intercambio = buscar_pedido_intercambio(entrenador);

			if(pedido_intercambio) {
				while(entrenador -> pasos_a_moverse > 0) {

					tradear_pokemon(pedido_intercambio);
				}
				if(!(entrenador -> tire_accion)) { // RR Desalojado
					sem_wait(&mx_estados);
					cambiar_a_estado(estado_ready, entrenador);
					log_info(logger, "El entrenador %d fue desalojado por fin de quantum", entrenador -> id);
					sem_post(&mx_estados);
					entrenador -> pasos_a_moverse = ((config -> quantum < pedido_intercambio -> distancia) ? config -> quantum : pedido_intercambio -> distancia);
					sem_post(&entrenadores_ready);
				} else {
					asignar_estado_luego_de_trade(entrenador);
				}
				sem_post(&mx_estado_exec);
			} else {
				asignar_estado_luego_de_trade(entrenador);
			}
		}
	}
	return NULL;
}

void asignar_estado_luego_de_trade(t_entrenador* entrenador) {
	if(cumplio_objetivo_personal(entrenador)) {
		sem_wait(&mx_estados);
		cambiar_a_estado(estado_exit, entrenador);
		log_info(logger, "Luego de ejecutar el trade, el entrenador %d cumplio su objetivo personal y se mueve a exit", entrenador -> id);
		sem_post(&mx_estados);

		if(config -> entrenadores -> elements_count == estado_exit -> elements_count) {
			sem_post(&fin_programa);
		}

	} else {
		if(!esta_en_estado(estado_block, entrenador)) {
			sem_wait(&mx_estados);
			cambiar_a_estado(estado_block, entrenador);
			log_info(logger, "Luego de ejecutar el trade, el entrenador %d no cumplio su objetivo personal y vuelve a block", entrenador -> id);
			sem_post(&mx_estados);
		}
		sem_post(&sem_cont_entrenadores_a_replanif);
	}
	entrenador -> tire_accion = 0;
}

void manejar_desalojo_captura(t_pedido_captura* pedido){
	if(!(pedido -> entrenador -> tire_accion) && pedido -> entrenador -> pasos_a_moverse <= 0) { // RR DESALOJADO
		sem_wait(&mx_estados);
		cambiar_a_estado(estado_ready, pedido -> entrenador);
		log_info(logger, "El entrenador %d termino su quantum y vuelve a ready", pedido -> entrenador -> id);
		sem_post(&mx_estados);
		sem_post(&entrenadores_ready);
		pedido -> entrenador -> pasos_a_moverse = config -> quantum;
		sem_post(&mx_estado_exec);

	} else if(!(pedido -> entrenador -> tire_accion)) { // SJF Con Desalojo DESALOJADO

		log_info(logger, "El entrenador %d fue desalojado por otro entrenador", pedido -> entrenador -> id);
		sem_post(&mx_desalojo_exec);
		sem_post(&mx_estado_exec);

	} else { // FIFO o SJF Sin Desalojo // RR/SJF NO DESALOJADO
		sem_post(&mx_estado_exec);
		sem_wait(&(pedido -> entrenador -> esperar_caught));

		procesar_caught(pedido);

	}

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

	bool es_el_pokemon(void* otro_pokemon){
		char* un_pokemon = otro_pokemon;

		return string_equals_ignore_case(un_pokemon, pedido -> pokemon -> nombre);
		}
	list_remove_by_condition(objetivo_global_pendiente, es_el_pokemon);

	if(pedido -> entrenador -> resultado_caught) {
		//log_info(logger, "...ATRAPE :)");
		list_add(pedido -> entrenador -> pokemons, pedido -> pokemon -> nombre);

		if(tengo_la_mochila_llena(pedido -> entrenador)){

			if(!estoy_en_deadlock(pedido -> entrenador)){
				sem_wait(&mx_estados);
				cambiar_a_estado(estado_exit, pedido -> entrenador);
				log_info(logger, "El entrenador %d completo su objetivo personal y se mueve a exit", pedido -> entrenador -> id);
				sem_post(&mx_estados);
			}

			if(entrenadores_con_mochila_llena()){
				sem_post(&sem_cont_mapa);
				sem_post(&sem_cont_entrenadores_a_replanif);
			}

		} else {
			sem_post(&sem_cont_entrenadores_a_replanif);
		}

	} else {
		//log_info(logger, "...NO ATRAPE :(");
		sem_post(&sem_cont_entrenadores_a_replanif);

		list_add(objetivo_global, pedido -> pokemon -> nombre);
		reagregar_especie_al_mapa_principal(pedido -> pokemon -> nombre);
	}

	pedido -> entrenador -> tire_accion = 0;
	eliminar_pedido_captura(pedido);
}


void reagregar_especie_al_mapa_principal(char* pokemon) {
	bool pokemon_a_eliminar(void* un_pokemon) {
		t_pokemon_mapa* otro_pokemon = un_pokemon;

		return string_equals_ignore_case(pokemon, otro_pokemon -> nombre);
	}

	t_pokemon_mapa* pokemon_a_remover = list_remove_by_condition(mapa_pokemons_pendiente, pokemon_a_eliminar);

	while(pokemon_a_remover) {
		sem_post(&sem_cont_mapa);
		list_add(mapa_pokemons, pokemon_a_remover);
		// pokemon_a_remover = list_remove_by_condition(mapa_pokemons_pendiente, eliminar_del_mapa_original);
	}
}

bool estoy_en_deadlock(t_entrenador* entrenador){

	if(!tengo_la_mochila_llena(entrenador)){

		return 0;
	}

	list_sort(entrenador -> pokemons, comparar_pokemon);
	list_sort(entrenador -> objetivos, comparar_pokemon);

	t_link_element* cabeza_pokemons = entrenador -> pokemons -> head;
	t_link_element* cabeza_objetivos = entrenador -> objetivos -> head;

	while(cabeza_objetivos){

		if(!string_equals_ignore_case(cabeza_pokemons -> data, cabeza_objetivos -> data)){

			return 1;
		}

		cabeza_pokemons = cabeza_pokemons -> next;
		cabeza_objetivos = cabeza_objetivos -> next;
	}

	return 0;
}

bool comparar_pokemon(void* another_pokemon, void* otro_pokemon) {
	char* pokemon = another_pokemon;
	char* un_pokemon = otro_pokemon;

	if(strcmp(pokemon, un_pokemon) > 0) {
		return 1;
	}
	return 0;

}

void eliminar_pedido_captura(t_pedido_captura* pedido) {

	bool es_el_pedido(void* un_pedido) {
		t_pedido_captura* otro_pedido = un_pedido;
		return otro_pedido == pedido;
	}

	list_remove_and_destroy_by_condition(pedidos_captura, es_el_pedido, destruir_pedido_captura);
}

void destruir_pedido_captura(void* one_pedido) {
	t_pedido_captura* another_pedido = one_pedido;
	destruir_pokemon(another_pedido -> pokemon);
	free((t_pedido_captura*) one_pedido); // aca capaz flasheamo
}

void eliminar_pedido_intercambio(t_pedido_intercambio* pedido) {

	bool es_el_pedido(void* otro_pedido) {

		t_pedido_intercambio* un_pedido = otro_pedido;

		return pedido == un_pedido;
	}

	list_remove_by_condition(pedidos_intercambio, es_el_pedido);
}

void destruir_pokemon(t_pokemon_mapa* pokemon) {
	//free(pokemon -> nombre); FALTA MALLOC AL AGREGAR
	free(pokemon);
}

void capturar_pokemon(t_pedido_captura* pedido) {

	if(pedido -> entrenador -> posicion[0] < pedido -> pokemon -> posicion[0]) {
		pedido -> entrenador -> posicion[0] ++;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador -> id, pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else if(pedido -> entrenador -> posicion[0] > pedido -> pokemon -> posicion[0]) {
		pedido -> entrenador -> posicion[0] --;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador -> id, pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else if(pedido -> entrenador -> posicion[1] < pedido -> pokemon -> posicion[1]) {
		pedido -> entrenador -> posicion[1] ++;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador -> id, pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else if(pedido -> entrenador -> posicion[1] > pedido -> pokemon -> posicion[1]) {
		pedido -> entrenador -> posicion[1] --;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador -> id, pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
	} else {

		log_info(logger, "El entrenador %d tiro el catch para un %s en la posicion [%d,%d]",
					pedido -> entrenador -> id,
					pedido -> pokemon -> nombre, pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);

		sem_wait(&mx_estados);
		cambiar_a_estado(estado_block, pedido -> entrenador);
		log_info(logger, "El entrenador %d se mueve a block para esperar el resultado del catch", pedido -> entrenador -> id);
		sem_post(&mx_estados);
		enviar_mensaje_catch(pedido);
		//sleep(config -> retardo_cpu * 4); //dsp decomentar CREO
	}

	pedido -> entrenador -> ciclos_cpu ++;
	sleep(config -> retardo_cpu);
	pedido -> entrenador -> pasos_a_moverse --;
}

uint32_t recibir_id_de_mensaje_enviado(uint32_t socket_cliente) {
  uint32_t id = 0;

  recv(socket_cliente, &id, sizeof(uint32_t), MSG_WAITALL);
  log_info(logger, "El ID de mensaje enviado es: %d", id);

  close(socket_cliente);
  return id;
}

void tradear_pokemon(t_pedido_intercambio* pedido){
	if(pedido -> entrenador_buscando -> posicion[0] < pedido -> entrenador_esperando -> posicion[0]) {
		pedido -> entrenador_buscando -> posicion[0] ++;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador_buscando -> id, pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando-> posicion[1]);
	} else if(pedido -> entrenador_buscando -> posicion[0] > pedido -> entrenador_esperando -> posicion[0]) {
		pedido -> entrenador_buscando -> posicion[0] --;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador_buscando -> id, pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando -> posicion[1]);
	} else if(pedido -> entrenador_buscando -> posicion[1] < pedido -> entrenador_esperando -> posicion[1]) {
		pedido -> entrenador_buscando -> posicion[1] ++;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador_buscando -> id, pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando -> posicion[1]);
	} else if(pedido -> entrenador_buscando -> posicion[1] > pedido -> entrenador_esperando -> posicion[1]) {
		pedido -> entrenador_buscando -> posicion[1] --;
		pedido -> distancia --;
		log_info(logger, "El entrenador %d se mueve a [%d,%d]", pedido -> entrenador_buscando -> id, pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando-> posicion[1]);
	} else {
		pedido -> entrenador_buscando -> tire_accion = 1;
		ejecutar_trade(pedido);
		sem_post(&(pedido -> entrenador_esperando -> sem_binario));
		sleep(config -> retardo_cpu * 4);
		pedido -> entrenador_buscando -> ciclos_cpu += 4;
	}
	pedido -> entrenador_buscando -> pasos_a_moverse --;
	pedido -> entrenador_buscando -> ciclos_cpu ++;
	sleep(config -> retardo_cpu);
}

void ejecutar_trade(t_pedido_intercambio* pedido) {

	bool es_el_pokemon_a_dar(void* un_pokemon) {

		char* pokemon_a_dar = un_pokemon;

		return string_equals_ignore_case(pokemon_a_dar, pedido -> pokemon_a_dar);
	}

	list_remove_by_condition(pedido -> entrenador_buscando -> pokemons, es_el_pokemon_a_dar);

	bool es_el_pokemon_a_recibir(void* un_pokemon){

		char* pokemon_a_recibir = un_pokemon;

		return string_equals_ignore_case(pokemon_a_recibir, pedido -> pokemon_a_recibir);
	}

	list_remove_by_condition(pedido -> entrenador_esperando -> pokemons, es_el_pokemon_a_recibir);

	list_add(pedido -> entrenador_buscando -> pokemons, pedido -> pokemon_a_recibir);
	list_add(pedido -> entrenador_esperando -> pokemons, pedido -> pokemon_a_dar);

	log_info(logger, "Intercambio entre el entrenador %d (%s) con el %d (%s)",
			pedido -> entrenador_buscando -> id, pedido -> pokemon_a_dar,
			pedido -> entrenador_esperando -> id, pedido -> pokemon_a_recibir);

	eliminar_pedido_intercambio(pedido);
}

t_pedido_intercambio* buscar_pedido_intercambio(t_entrenador* entrenador){

	bool es_pedido_del_entrenador(void* un_pedido) {
		t_pedido_intercambio* pedido = un_pedido;

		return pedido -> entrenador_buscando == entrenador;
	}

	return list_find(pedidos_intercambio, es_pedido_del_entrenador);
}

t_pedido_captura* buscar_pedido_captura(t_entrenador* entrenador) {

	bool es_pedido_del_entrenador(void* un_pedido) {
		t_pedido_captura* pedido = un_pedido;

		return pedido -> entrenador == entrenador;
	}

	return list_find(pedidos_captura, es_pedido_del_entrenador);
}


void* planificar_entrenadores() {
	int deadlocks = 1;
	while(deadlocks){
		sem_wait(&sem_cont_mapa);
		sem_wait(&sem_cont_entrenadores_a_replanif);
		if(!entrenadores_con_mochila_llena()) {

			t_pedido_captura* pedido = malloc(sizeof(t_pedido_captura));
			armar_pedido_captura(pedido);
			eliminar_del_objetivo_global(pedido -> pokemon);
			eliminar_pokemon_de_mapa(pedido -> pokemon);
			planificar_segun_algoritmo(pedido);

			//free(pedido); -- VER CON VALGRIND

		} else {
			log_info(logger, "Inicio del algoritmo de deteccion de deadlocks");

			for(int i = 0; i <= estado_block -> elements_count; i++) {
				sem_post(&sem_cont_entrenadores_a_replanif);
			}
			loggear_entrenadores_deadlock();
			planificar_deadlocks();
			deadlocks = 0;
		}
	}

	return 0;
}

void loggear_entrenadores_deadlock() {

	void imprimir_deadlock(void* un_entrenador) {
		t_entrenador* entrenador = un_entrenador;

		log_info(logger, "El entrenador %d esta en deadlock", entrenador -> id);
	}
	list_iterate(estado_block, imprimir_deadlock);

	deadlocks_totales ++;
}

bool entrenadores_con_mochila_llena() {

	bool tiene_la_mochila_llena(void* un_entrenador) {
		t_entrenador* entrenador = un_entrenador;

		return entrenador -> pokemons -> elements_count >= entrenador -> objetivos -> elements_count;
	}

	return list_all_satisfy(config -> entrenadores, tiene_la_mochila_llena);
}

void eliminar_del_objetivo_global(t_pokemon_mapa* pokemon) {

	bool es_el_pokemon(void* otro_pokemon){
		char* un_pokemon = otro_pokemon;

		return string_equals_ignore_case(un_pokemon, pokemon -> nombre);
	}

	list_remove_by_condition(objetivo_global, es_el_pokemon);

	list_add(objetivo_global_pendiente, pokemon -> nombre);
}

void planificar_deadlocks() {

	while(estado_exit -> elements_count < config -> entrenadores -> elements_count) {

		sem_wait(&sem_cont_entrenadores_a_replanif);
		sem_wait(&sem_cont_entrenadores_a_replanif);

		if(config -> entrenadores -> elements_count - estado_exit -> elements_count < 2) {
			log_error(logger, "Tengo poca gente para resolver deadlocks!!");
		}
		if(estado_block -> elements_count < 2) {
			log_error(logger, "Me mandaste a planificar deadlock y no tengo 2 pibes en block!!");
		}

		t_pedido_intercambio* pedido = armar_pedido_intercambio_segun_algoritmo();
		if(!pedido) {
			sleep(1);
			sem_post(&sem_cont_entrenadores_a_replanif);
			sem_post(&sem_cont_entrenadores_a_replanif);
			continue;
		}
		list_add(pedidos_intercambio, pedido);

		sem_wait(&mx_estados);
		cambiar_a_estado(estado_ready, pedido -> entrenador_buscando);
		log_info(logger, "El entrenador %d se mueve a ready para realizar el intercambio con el %d", pedido -> entrenador_buscando -> id, pedido -> entrenador_esperando -> id);
		sem_post(&entrenadores_ready);
		sem_post(&mx_estados);
	}
}

t_pedido_intercambio* armar_pedido_intercambio_segun_algoritmo(){

	//log_info(logger, "....somos %d en deadlock, armo pedido", estado_block -> elements_count);
	t_pedido_intercambio* pedido = malloc(sizeof(t_pedido_intercambio));

	t_link_element* cabeza_block = estado_block -> head;

	while(estoy_esperando_trade(cabeza_block -> data)) {

		cabeza_block = cabeza_block -> next;

		if(!cabeza_block){
			log_error(logger, "NO HAY NADIE DESOCUPADO EN BLOCK");
		}
	}

	pedido -> entrenador_buscando = cabeza_block -> data;

	pedido -> pokemon_a_recibir = encontrar_pokemon_faltante(pedido -> entrenador_buscando);

	bool entrenador_que_le_sobra_pokemon_y_esta_libre(void* un_entrenador){

		t_entrenador* entrenador = un_entrenador;

		return le_sobra_pokemon(entrenador, pedido -> pokemon_a_recibir) && !estoy_esperando_trade(cabeza_block -> data);
	}

	pedido -> entrenador_esperando = list_find(estado_block, entrenador_que_le_sobra_pokemon_y_esta_libre);

	if(!(pedido -> entrenador_esperando)) {
		return NULL;
		bool entrenador_que_le_sobra_pokemon(void* un_entrenador){
			t_entrenador* entrenador = un_entrenador;

			return le_sobra_pokemon(entrenador, pedido -> pokemon_a_recibir);
		}
		pedido -> entrenador_esperando = list_find(estado_block, entrenador_que_le_sobra_pokemon);

		if(!(pedido -> entrenador_esperando)) {
			return NULL;
			log_error(logger, "A nadie le sobra mi pokemon!! (lo debe tener alguien que se este moviendo)"); // si se llega a este log handlear el case para perseguir.
		}
		log_error(logger, "Mira q lindo como pase y nadie me vio");
	}

	pedido -> pokemon_a_dar = encontrar_pokemon_sobrante(pedido -> entrenador_buscando);

	pedido -> distancia = distancia(pedido -> entrenador_buscando -> posicion, pedido -> entrenador_esperando -> posicion) + 1;

	if(string_equals_ignore_case(config -> algoritmo_planificacion, "RR")) {
		pedido -> entrenador_buscando -> pasos_a_moverse = config -> quantum;

	} else {
		pedido -> entrenador_buscando -> pasos_a_moverse = pedido -> distancia;
	}

	return pedido;
}

bool estoy_esperando_trade(t_entrenador* entrenador) {

	bool esta_esperando(void* otro_pedido) {

		t_pedido_intercambio* un_pedido = otro_pedido;

		return un_pedido -> entrenador_esperando == entrenador;
	}

	void* resultado = list_find(pedidos_intercambio, esta_esperando);

	if(!resultado){
		return 0;
	}

	return 1;
}

bool le_sobra_pokemon(t_entrenador* entrenador, char* pokemon_original){

	t_list* pokemons = list_duplicate(entrenador -> pokemons);
	t_list* objetivos = list_duplicate(entrenador -> objetivos);
	bool pokemon_sobrante = 0;

	void remover_del_objetivo(void* un_pokemon){

		char* pokemon = un_pokemon;

		bool es_el_pokemon(void* another_pokemon){
			char* otro_pokemon = another_pokemon;

			return string_equals_ignore_case(otro_pokemon, pokemon);
		}

		void* resultado = list_remove_by_condition(objetivos, es_el_pokemon);

		if(!resultado && string_equals_ignore_case(pokemon, pokemon_original)){

			pokemon_sobrante = 1;
		}
	}

	list_iterate(pokemons, remover_del_objetivo);

	list_destroy(objetivos);
	list_destroy(pokemons);
	return pokemon_sobrante;

}

char* encontrar_pokemon_sobrante(t_entrenador* entrenador){

	t_list* pokemons = list_duplicate(entrenador -> pokemons);
	t_list* objetivos = list_duplicate(entrenador -> objetivos);
	char* pokemon_sobrante = NULL;

	void remover_del_objetivo(void* un_pokemon){

		char* pokemon = un_pokemon;

		bool es_el_pokemon(void* another_pokemon){
			char* otro_pokemon = another_pokemon;

			return string_equals_ignore_case(otro_pokemon, pokemon);
		}

		void* resultado = list_remove_by_condition(objetivos, es_el_pokemon);

		if(!resultado){

			pokemon_sobrante = pokemon;
		}
	}

	list_iterate(pokemons, remover_del_objetivo);

	list_destroy(pokemons);
	list_destroy(objetivos);

	return pokemon_sobrante;
}

char* encontrar_pokemon_faltante(t_entrenador* entrenador){

	t_list* pokemons = list_duplicate(entrenador -> pokemons);
	t_list* objetivos = list_duplicate(entrenador -> objetivos);
	char* pokemon_faltante = NULL;

	void remover_del_objetivo(void* un_pokemon){

		char* pokemon = un_pokemon;

		bool es_el_pokemon(void* another_pokemon){
			char* otro_pokemon = another_pokemon;

			return string_equals_ignore_case(otro_pokemon, pokemon);
		}

		void* resultado = list_remove_by_condition(pokemons, es_el_pokemon);

		if(!resultado){

			pokemon_faltante = pokemon;
		}
	}

	list_iterate(objetivos, remover_del_objetivo);

	list_destroy(pokemons);
	list_destroy(objetivos);

	return pokemon_faltante;
}

bool tengo_la_mochila_llena(t_entrenador* entrenador) {
	return entrenador -> pokemons -> elements_count >= entrenador -> objetivos -> elements_count;
}

void planificar_segun_algoritmo(t_pedido_captura* pedido) {

	pedido -> entrenador -> pasos_a_moverse = distancia_segun_algoritmo(pedido);

	if(string_equals_ignore_case(config -> algoritmo_planificacion, "FIFO") ||
			string_equals_ignore_case(config -> algoritmo_planificacion, "RR")) {

		planificar_fifo_o_rr(pedido);

	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-SD")) {

		planificar_sjf_sd(pedido);

	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-CD")) {

		planificar_sjf_cd(pedido);
	} else {
		log_error(logger, "Tengo un algoritmo de mierda, qué rompimo?");
	}
}

void planificar_fifo_o_rr(t_pedido_captura* pedido){
	sem_wait(&mx_estados);
	cambiar_a_estado(estado_ready, pedido -> entrenador);
	log_info(logger, "El entrenador %d fue cambiado a estado ready con su pedido de captura", pedido -> entrenador -> id);
	sem_post(&mx_estados);
	sem_post(&entrenadores_ready);
}

void planificar_sjf_sd(t_pedido_captura* pedido){

	sem_wait(&mx_estados);
	cambiar_a_estado(estado_ready, pedido -> entrenador);
	log_info(logger, "El entrenador %d fue cambiado a estado ready con su pedido de captura", pedido -> entrenador -> id);
	calcular_estimaciones_ready();
	ordenar_ready_segun_estimacion();
	sem_post(&mx_estados);
	sem_post(&entrenadores_ready);
}

void calcular_estimaciones_ready() {

	void calcular_estimacion(void* un_entrenador) {

		t_entrenador* entrenador = un_entrenador;

		uint32_t estimacion_anterior = entrenador -> estimacion;

		entrenador -> estimacion = config -> alpha * entrenador -> pasos_a_moverse + (1 - config -> alpha) * estimacion_anterior;
	}

	list_iterate(estado_ready, calcular_estimacion);
}

void ordenar_ready_segun_estimacion() {

	if(estado_ready -> elements_count >= 2){
		bool estimacion_menor(void* primer_entrenador, void* segundo_entrenador){

			t_entrenador* un_entrenador = primer_entrenador;
			t_entrenador* otro_entrenador = segundo_entrenador;

			return un_entrenador -> estimacion < otro_entrenador -> estimacion;
		}

		list_sort(estado_ready, estimacion_menor);
	}
}

void planificar_sjf_cd(t_pedido_captura* pedido) {

	sem_wait(&mx_estados);
	cambiar_a_estado(estado_ready, pedido -> entrenador);
	log_info(logger, "El entrenador %d fue cambiado a estado ready con su pedido de captura", pedido -> entrenador -> id);

	desalojar_ejecucion();
	calcular_estimaciones_ready();
	ordenar_ready_segun_estimacion();
	sem_post(&mx_estados);
	sem_post(&entrenadores_ready);
}

void desalojar_ejecucion() {

	if(estado_exec -> elements_count > 0){
		cambiar_a_estado(estado_ready, estado_exec -> head -> data);
		sem_wait(&mx_desalojo_exec);
		sem_post(&entrenadores_ready);
	}
}

int distancia_segun_algoritmo(t_pedido_captura* pedido) {
	if(string_equals_ignore_case(config -> algoritmo_planificacion, "RR")) {
		return config -> quantum;
	}
	return pedido -> distancia + 1;
}

void armar_pedido_captura(t_pedido_captura* pedido) {

	pedido -> entrenador = NULL;
	pedido -> pokemon = mapa_pokemons -> head -> data;
	pedido -> distancia = -1;
	matchear_pokemon_con_entrenador(pedido);
	list_add(pedidos_captura, pedido);
}

void crear_hilo_segun_algoritmo() {

	if(string_equals_ignore_case(config -> algoritmo_planificacion, "FIFO") ||
			string_equals_ignore_case(config -> algoritmo_planificacion, "RR") ||
			string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-SD") ||
			string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-CD")) {

		uint32_t err = pthread_create(&hilo_algoritmo, NULL, ejecutar_algoritmo, NULL);

		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}

	} else {
		log_error(logger, "wtf?? Algoritmo de planificacion recibido: %s", config -> algoritmo_planificacion);
	}
}

void crear_hilo_planificar_entrenadores() {

	uint32_t err = pthread_create(&hilo_planificar, NULL, planificar_entrenadores, NULL);
	if(err != 0) {
		log_error(logger, "El hilo no pudo ser creado!!");
	}
}

void* ejecutar_algoritmo() {

	while(config -> entrenadores -> elements_count > estado_exit -> elements_count) { // matamos este hilo o muere con el programa?

		sem_wait(&mx_estado_exec);
		sem_wait(&entrenadores_ready);
		sem_wait(&mx_estados);
		if(!estado_exec -> head && estado_ready -> head) {
			t_entrenador* entrenador = estado_ready -> head -> data;
			cambiar_a_estado(estado_exec, entrenador);
			log_info(logger, "El entrenador %d fue cambiado a estado exec", entrenador -> id);
			sem_post(&(entrenador -> sem_binario));
		} else {
			log_error(logger, "Me mandaste a correr pero no tengo ningun pibe pa!!");
		}
		sem_post(&mx_estados);
	}

	return 0;
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

	bool es_el_pokemon(void* otro_pokemon) {
		char* un_pokemon = otro_pokemon;
		return string_equals_ignore_case(pokemon -> nombre, un_pokemon);
	}

	bool eliminar_del_mapa_original(void* otro_pokemon) {
		t_pokemon_mapa* un_pokemon = otro_pokemon;
		return string_equals_ignore_case(pokemon -> nombre, un_pokemon -> nombre);
	}

	if(!list_find(objetivo_global, es_el_pokemon)) {

		pokemon_a_remover = list_remove_by_condition(mapa_pokemons, eliminar_del_mapa_original);

		while(pokemon_a_remover) {
			sem_wait(&sem_cont_mapa);
			list_add(mapa_pokemons_pendiente, pokemon_a_remover);
			pokemon_a_remover = list_remove_by_condition(mapa_pokemons, eliminar_del_mapa_original);
		}

	}
}

bool no_esta_en_objetivo(void* pokemon) {

	bool es_el_pokemon(void* otro_pokemon) {
		t_pokemon_mapa* un_pokemon = otro_pokemon;

		 return pokemon == un_pokemon -> nombre;
	}

	return list_find(objetivo_global, es_el_pokemon);
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

		if(tengo_la_mochila_llena(entrenador)){

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
		t_entrenador* otro_entrenador = un_entrenador;
		return otro_entrenador == entrenador;
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

			sem_wait(&mx_contexto);
			cambios_de_contexto ++;
			sem_post(&mx_contexto);

		} else {
			log_error(logger, "ESTOY TRATANDO DE CAMBIAR A EXEC DESDE UN ESTADO QUE NO DEBERIA.");
		}
	} else if(estado == estado_block) {

		if(esta_en_estado(estado_exec, entrenador)) {
			estado_actual = estado_exec;
		} else {
			log_error(logger, "ESTOY TRATANDO DE CAMBIAR A BLOCK DESDE UN ESTADO QUE NO DEBERIA.");
		}
	} else if(estado == estado_exit) {

		list_add(estados_a_buscar, estado_exec);
		list_add(estados_a_buscar, estado_block);
		estado_actual = buscar_en_estados(estados_a_buscar, entrenador);
		if(!estado_actual) {
			log_error(logger, "ESTOY TRATANDO DE CAMBIAR A EXIT DESDE UN ESTADO QUE NO DEBERIA.");
		}
	} else {
		log_error(logger, "ESTOY TRATANDO DE CAMBIAR A UN ESTADO QUE NO DEBERIA.");
	}

	if(estado_actual == estado_exec){
		sem_wait(&mx_contexto);
		cambios_de_contexto ++;
		sem_post(&mx_contexto);
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
		t_entrenador* otro_entrenador = un_entrenador;
		return otro_entrenador == entrenador;
	}

	if(list_find(estado, es_el_entrenador)) {
		return 1;
	}
	return 0;
}

// ------------------------------- MENSAJES ------------------------------- //

void enviar_mensaje_catch(t_pedido_captura* pedido) {

	t_catch_pokemon* mensaje = malloc(sizeof(t_catch_pokemon));

	mensaje -> pokemon = pedido -> pokemon -> nombre;
	mensaje -> posicion[0] = pedido -> pokemon -> posicion[0];
	mensaje -> posicion[1] = pedido -> pokemon -> posicion[1];
	mensaje -> id_mensaje = 0;

	uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);

	pedido -> entrenador -> tire_accion = 1;

	if(socket != -1) {

		uint32_t tamanio_mensaje = size_mensaje(mensaje, CATCH_POKEMON);
		enviar_mensaje(CATCH_POKEMON, mensaje, socket, tamanio_mensaje);

		//pedido -> entrenador -> socket_mensaje_catch = socket; // ver comentario t_entrenador
		pedido -> entrenador -> id_espera_catch = recibir_id_de_mensaje_enviado(socket);

	} else { // Comportamiento default *ATRAPÓ*
		log_info(logger, "No se pudo establecer la conexion con broker, se realiza el comportamiento default del catch");
		pedido -> entrenador -> resultado_caught = 1;
		sem_post(&(pedido -> entrenador -> esperar_caught));
	}

}

void enviar_mensaje_get(char* pokemon) {

	t_get_pokemon* mensaje = malloc(sizeof(t_get_pokemon));

	mensaje -> id_mensaje = 0;
	mensaje -> pokemon = malloc(strlen(pokemon)+1);
	mensaje -> pokemon = pokemon;

	uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);

	if(socket != -1) {
		uint32_t tamanio_mensaje = size_mensaje(mensaje, GET_POKEMON);

		enviar_mensaje(GET_POKEMON, mensaje, socket, tamanio_mensaje);

		int id = recibir_id_de_mensaje_enviado(socket);

	} else{
		log_info(logger, "No se pudo establecer la conexion con broker, se realiza el comportamiento default del get");
	}
}

void enviar_get_pokemon() {

	void agregar_si_no_encuentra(void* otro_pokemon) {
		char* pokemon = otro_pokemon;

		bool es_el_pokemon(void* another_pokemon) {
			char* un_pokemon = another_pokemon;
			return string_equals_ignore_case(pokemon, un_pokemon);
		}

		if(list_find(especies_objetivo_global, es_el_pokemon) == NULL) {
			list_add(especies_objetivo_global, pokemon);
		}
	}

	list_iterate(objetivo_global, agregar_si_no_encuentra);

	/* ---------- */
	void enviar_mensaje_por_especie(void* pokemon) {

		uint32_t err = pthread_create(&hilo_get, NULL, hilo_mensaje_get, pokemon);
			if(err != 0) {
				log_error(logger, "El hilo no pudo ser creado!!");
			}
		pthread_detach(hilo_get);

		//sleep(2);
	}

	list_iterate(especies_objetivo_global, enviar_mensaje_por_especie);
}

void* hilo_mensaje_get(void* un_pokemon) {

	enviar_mensaje_get((char*) un_pokemon);

	return NULL;
}

void process_request(uint32_t cod_op, uint32_t cliente_fd) {

	uint32_t size = 0; // check if 0, si no inicializo se queja valgrind
	op_code* codigo_op = malloc(sizeof(op_code));


	void* stream = recibir_paquete(cliente_fd, &size, codigo_op);
	sem_wait(&mx_paquete);

	cod_op = (*codigo_op);

	void* mensaje_recibido = deserealizar_paquete(stream, *codigo_op, size);

	switch (cod_op) {
		case LOCALIZED_POKEMON:
			log_info(logger, "Se recibio un mensaje LOCALIZED con id %d y pokemon %s",
					((t_localized_pokemon*) mensaje_recibido) -> id_mensaje, ((t_localized_pokemon*) mensaje_recibido) -> pokemon); // faltaria la lista de posiciones
			procesar_localized((t_localized_pokemon*) mensaje_recibido);
			enviar_ack_broker(((t_localized_pokemon*) mensaje_recibido) -> id_mensaje, LOCALIZED_POKEMON);
			break;
		case CAUGHT_POKEMON:
			log_info(logger, "Se recibio un mensaje CAUGHT con id %d y resultado %d",
					((t_caught_pokemon*) mensaje_recibido) -> id_mensaje, ((t_caught_pokemon*) mensaje_recibido) -> resultado);
			procesar_mensaje_caught((t_caught_pokemon*) mensaje_recibido);
			enviar_ack_broker(((t_caught_pokemon*) mensaje_recibido) -> id_mensaje, CAUGHT_POKEMON);
			break;
		case APPEARED_POKEMON:
			log_info(logger, "Se recibio un mensaje APPEARED con id %d, pokemon %s y posicion [%d,%d]",
					((t_appeared_pokemon*) mensaje_recibido) -> id_mensaje, ((t_appeared_pokemon*) mensaje_recibido) -> pokemon,
					((t_appeared_pokemon*) mensaje_recibido) -> posicion[0], ((t_appeared_pokemon*) mensaje_recibido) -> posicion[1]);
			procesar_mensaje_appeared((t_appeared_pokemon*) mensaje_recibido);
			enviar_ack_broker(((t_appeared_pokemon*) mensaje_recibido) -> id_mensaje, APPEARED_POKEMON); // y si viene del gameboy?
			break;
		case 0:
			log_error(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);

		case -1:
			pthread_exit(NULL);
	}
	sem_post(&mx_paquete);
	free(codigo_op);
	free(stream);
}

void enviar_ack_broker(uint32_t id_mensaje, op_code codigo) {

	t_ack* ack = malloc(sizeof(t_ack));

	ack -> id_mensaje = id_mensaje;
	ack -> tipo_mensaje = codigo;
	ack -> id_proceso = config -> id_proceso;

	uint32_t size_mensaje = size_ack(ack);

	uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);

	if(socket != -1) {
		enviar_mensaje(ACK, ack, socket, size_mensaje);
		close(socket);
	}

}

void procesar_mensaje_appeared(t_appeared_pokemon* mensaje_recibido) {

	t_pokemon_mapa* pokemon_mapa = malloc(sizeof(t_pokemon_mapa));

	bool es_el_pokemon(void* another_pokemon) {
		char* un_pokemon = another_pokemon;

		return string_equals_ignore_case(mensaje_recibido -> pokemon, un_pokemon);
	}

	if(list_find(objetivo_global, es_el_pokemon)) {

		pokemon_mapa -> nombre = mensaje_recibido -> pokemon;
		pokemon_mapa -> posicion[0] = mensaje_recibido -> posicion[0];
		pokemon_mapa -> posicion[1] = mensaje_recibido -> posicion[1];

		bool esta_en_el_mapa(void* another_pokemon_mapa) {
			t_pokemon_mapa* un_pokemon_mapa = another_pokemon_mapa;
			return pokemon_mapa -> nombre == un_pokemon_mapa -> nombre &&
					pokemon_mapa -> posicion[0] == un_pokemon_mapa -> posicion[0] &&
					pokemon_mapa -> posicion[1] == un_pokemon_mapa -> posicion[1];
		}
		t_pokemon_mapa* otro_pokemon_mapa = list_find(mapa_pokemons, esta_en_el_mapa);

		if(otro_pokemon_mapa == NULL) {
			pokemon_mapa -> cantidad = 1;
			list_add(mapa_pokemons, pokemon_mapa);
		} else {
			(otro_pokemon_mapa -> cantidad)++;
		}

		sem_post(&sem_cont_mapa);

	} else if(list_find(objetivo_global_pendiente, es_el_pokemon)) {

		pokemon_mapa -> nombre = mensaje_recibido -> pokemon;
		pokemon_mapa -> posicion[0] = mensaje_recibido -> posicion[0];
		pokemon_mapa -> posicion[1] = mensaje_recibido -> posicion[1];

		bool esta_en_el_mapa(void* another_pokemon_mapa) {
			t_pokemon_mapa* un_pokemon_mapa = another_pokemon_mapa;
			return pokemon_mapa -> nombre == un_pokemon_mapa -> nombre &&
					pokemon_mapa -> posicion[0] == un_pokemon_mapa -> posicion[0] &&
					pokemon_mapa -> posicion[1] == un_pokemon_mapa -> posicion[1];
		}
		t_pokemon_mapa* otro_pokemon_mapa = list_find(mapa_pokemons_pendiente, esta_en_el_mapa);

		if(otro_pokemon_mapa == NULL) {
			pokemon_mapa -> cantidad = 1;
			list_add(mapa_pokemons_pendiente, pokemon_mapa);
		} else {
			(otro_pokemon_mapa -> cantidad)++;
		}
	}

	free(mensaje_recibido);
}

void procesar_mensaje_caught(t_caught_pokemon* mensaje_recibido) {

	bool tiene_el_id_recibido(void* un_entrenador) {
		t_entrenador* entrenador = un_entrenador;
		return entrenador -> id_espera_catch == mensaje_recibido -> id_mensaje;
	}

	t_entrenador* entrenador = list_find(estado_block, tiene_el_id_recibido);

	if(entrenador != NULL) {
		entrenador -> resultado_caught = mensaje_recibido -> resultado;
		sem_post(&(entrenador -> esperar_caught));
	} else {
		log_error(logger, "NO TENGO AL ENTRENADOR DEL MENSAJE RECIBIDO EN BLOCK.");
	}

	free(mensaje_recibido);
}

void procesar_localized(t_localized_pokemon* mensaje_recibido) {

	bool es_el_pokemon(void* another_pokemon) {
		char* un_pokemon = another_pokemon;
		return string_equals_ignore_case(mensaje_recibido -> pokemon, un_pokemon);
	}

	if(list_find(especies_ya_localizadas, es_el_pokemon) == NULL && list_find(especies_objetivo_global, es_el_pokemon)) {
		list_add(especies_ya_localizadas, mensaje_recibido -> pokemon);
		list_remove_by_condition(especies_objetivo_global, es_el_pokemon);
		agregar_localized_al_mapa(mensaje_recibido);
	}

	free(mensaje_recibido);
}

void agregar_localized_al_mapa(t_localized_pokemon* mensaje_recibido) {

	t_link_element* cabeza_lista = mensaje_recibido -> posiciones -> head;

	cabeza_lista = cabeza_lista -> next;

	while(cabeza_lista) {
		t_pokemon_mapa* pokemon_mapa = malloc(sizeof(t_pokemon_mapa));

		pokemon_mapa -> nombre = mensaje_recibido -> pokemon;
		pokemon_mapa -> posicion[0] = *(int*)cabeza_lista -> data;
		cabeza_lista = cabeza_lista -> next;

		if(!cabeza_lista) {
			log_error(logger, "el LOCALIZED tiene cantidad de posiciones impar");
		}

		pokemon_mapa -> posicion[1] = *(int*)cabeza_lista -> data;
		pokemon_mapa -> cantidad = 1;

		list_add(mapa_pokemons, pokemon_mapa);
		sem_post(&sem_cont_mapa);

		cabeza_lista = cabeza_lista -> next;
	}

}

// ------------------------------- TERMINAR ------------------------------- //

void liberar_config() {
	liberar_entrenadores();
	free(config -> algoritmo_planificacion);
	free(config -> log_file);
	free(config -> ip_broker);
	free(config -> puerto_broker);
	free(config -> ip_gameboy);
	free(config -> puerto_gameboy);
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
	list_destroy(estados);
}

void liberar_semaforos() {
	sem_destroy(&mx_estados);
	sem_destroy(&mx_estado_exec);
	sem_destroy(&mx_desalojo_exec);
	sem_destroy(&entrenadores_ready);
	sem_destroy(&sem_cont_mapa);
	sem_destroy(&sem_cont_entrenadores_a_replanif);
	sem_destroy(&mx_contexto);
	sem_destroy(&mx_paquete);
	sem_destroy(&fin_programa);
	sem_destroy(&semaforo);
}

void liberar_listas() {
	list_destroy(objetivo_global);
	list_destroy(pedidos_intercambio);
	list_destroy(pedidos_captura);
	list_destroy(especies_objetivo_global);
	list_destroy(especies_ya_localizadas);
	list_destroy(objetivo_global_pendiente);
	list_destroy(mapa_pokemons_pendiente);
}

void terminar_hilos() {
	terminar_hilos_entrenadores();
	pthread_cancel(hilo_algoritmo); // ambos bloqueados por semaforos pq terminaron los entrenadores y no van a recibir post
	pthread_cancel(hilo_planificar); // ambos bloqueados por semaforos pq terminaron los entrenadores y no van a recibir post
	pthread_cancel(hilo_game_boy);
	pthread_cancel(hilo_broker);
}

void terminar_hilos_entrenadores() {

	void esperar_hilo_entrenador(void* un_entrenador) {
		t_entrenador* entrenador = un_entrenador;
		pthread_join(entrenador -> hilo, NULL);
	}

	list_iterate(config -> entrenadores, esperar_hilo_entrenador);
}

void loggear_resultados() {

	void ciclos_por_entrenador(void* un_entrenador) {
		t_entrenador* entrenador = un_entrenador;

		log_info(logger, "La cantidad de ciclos de cpu del entrenador %d es %d", entrenador -> id, entrenador -> ciclos_cpu);

		ciclos_cpu_totales += entrenador -> ciclos_cpu;
	}

	list_iterate(config -> entrenadores, ciclos_por_entrenador);

	log_info(logger, "La cantidad de ciclos de cpu totales es %d", ciclos_cpu_totales);

	sem_wait(&mx_contexto);
	log_info(logger, "La cantidad de cambios de contexto es %d", cambios_de_contexto);
	sem_post(&mx_contexto);

	log_info(logger, "La cantidad de deadlocks producidos es %d y la cantidad de resueltos es %d", deadlocks_totales, deadlocks_resueltos);
}

void terminar_programa() {
	deadlocks_resueltos ++;
	terminar_hilos();
	loggear_resultados();
	free(mapa_pokemons);
	liberar_listas();
	liberar_estados();
	liberar_semaforos();
	liberar_config();
	log_info(logger, "El team completo su objetivo");
	log_info(logger, "El team finalizo correctamente");
	liberar_logger();
}	
