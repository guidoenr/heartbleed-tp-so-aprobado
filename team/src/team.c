#include "team.h"

int main(void) {

	iniciar_programa();

	log_info(logger, "La ip es: %s", config -> ip_broker);
	log_info(logger, "El port es: %s", config -> puerto_broker);
	/*
	t_link_element* element = config -> entrenadores -> head;
	t_entrenador* entrenador = config -> entrenadores -> head -> data;
	//list_add(entrenador -> pokemons, "Pikachu");
	list_add(entrenador -> pokemons, "Charmander");
	log_info(logger, "me sobra %d", (int*) le_sobra_pokemon(entrenador,"Charmanderr"));


	list_add(entrenador -> pokemons, "Charmander");
	bool i = estoy_en_deadlock(entrenador);
	log_info(logger, "%d", (int*) i);
	*/


	t_pokemon_mapa* pikachu1 = malloc(sizeof(t_pokemon_mapa));
	pikachu1 -> nombre = "Pikachu";
	pikachu1 -> posicion[0] = 6;
	pikachu1 -> posicion[1] = 6;
	pikachu1 -> cantidad = 1;
	list_add(mapa_pokemons, pikachu1);
	sem_post(&sem_cont_mapa);
	t_pokemon_mapa* pikachu2 = malloc(sizeof(t_pokemon_mapa));
	pikachu2 -> nombre = "Pikachu";
	pikachu2 -> posicion[0] = 2;
	pikachu2 -> posicion[1] = 8;
	pikachu2 -> cantidad = 1;
	list_add(mapa_pokemons, pikachu2);
	sem_post(&sem_cont_mapa);
	t_pokemon_mapa* charmander = malloc(sizeof(t_pokemon_mapa));
	charmander -> nombre = "Charmander";
	charmander -> posicion[0] = 0;
	charmander -> posicion[1] = 0;
	charmander -> cantidad = 1;
	list_add(mapa_pokemons, charmander);
	sem_post(&sem_cont_mapa);

	//pthread_join(hilo_algoritmo, NULL);
	//pthread_join(hilo_entrenadores, NULL);
	sleep(15);
	sem_post(&sem_cont_mapa); // ESTE CUANDO HAY QUE RESOLVER DEADLOCKS
	sleep(30);
	terminar_programa(/*socket*/);
	return 0;
}

//------------------------------- INICIAR -------------------------------//

void iniciar_programa() {
	iniciar_logger("team.log", "team");
	leer_config();
	inicializar_estados();
	inicializar_semaforos();
	determinar_objetivo_global();


	iniciar_entrenadores();
	mapa_pokemons = list_create();
	pedidos_captura = list_create();
	pedidos_intercambio = list_create();

	iniciar_hilos_ejecucion();
	//suscribirme_a_colas();
	//iniciar_conexion_game_boy(); abrir socket con el game_boy (pthread_create)
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
	sem_init(&mx_estado_new, 0, 1);
	sem_init(&mx_estado_ready, 0, 1);
	sem_init(&mx_estado_exec, 0, 1);
	sem_init(&mx_estado_block, 0, 1);
	sem_init(&mx_estado_exit, 0, 1);
	sem_init(&entrenadores_ready, 0, 0);
	sem_init(&sem_cont_mapa, 0, 0);
	sem_init(&sem_cont_entrenadores_a_replanif, 0, 0);
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


//TODO CHEQUEAR, robado de game_card :p
void suscribirse_a(op_code cola) {
	uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	char* suscripcion = string_new(); // se necesita free?
	string_append(&suscripcion, "Subscription from team to: ");
	string_append(&suscripcion, (char*) cola);
	uint32_t size = sizeof(suscripcion) + 1;
	enviar_mensaje(SUBSCRIPTION, suscripcion, socket, size);
}
//TODO CHEQUEAR, robado de game_card :p
void conectarse(int socket) {
	socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	if (socket == -1) {
		log_info(logger, "imposible conectar con broker, reintento en: %d",time);
		sleep(config -> tiempo_reconexion);
		conectarse(socket);
	} else {
		log_info(logger, "conexion exitosa con broker");
	}
}

// ------------------------------- ENTRENADORES -------------------------------//

void iniciar_entrenadores() {

	void crear_hilo_entrenador(void* un_entrenador) {
		pthread_t hilo;
		t_entrenador* entrenador = un_entrenador;
		agregar_a_estado(estado_new, entrenador);

		entrenador -> pasos_a_moverse = 0;
		entrenador -> tire_accion = 0;
		sem_init(&(entrenador -> sem_binario), 0, 0);
		sem_init(&(entrenador -> esperar_caught), 0, 0);

		uint32_t err = pthread_create(&hilo, NULL, operar_entrenador, (void*) entrenador);
		if(err != 0) {
			log_error(logger, "el hilo no pudo ser creado");
		}
		entrenador -> hilo = &hilo;

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
				if(!(entrenador -> tire_accion)) { // RR
					sem_wait(&mx_estado_ready);
					cambiar_a_estado(estado_ready, entrenador);
					log_info(logger, "el entrenador que estaba resolviendo deadlock fue desalojado por fin de quantum");
					sem_post(&mx_estado_ready);
					sem_post(&entrenadores_ready);
				} else {
					asignar_estado_luego_de_trade(entrenador);
				}
			} else {
				asignar_estado_luego_de_trade(entrenador);
			}
			sem_post(&mx_estado_exec);
		}
	}
	//while(1);
	return NULL;
}

void asignar_estado_luego_de_trade(t_entrenador* entrenador) {
	if(cumplio_objetivo_personal(entrenador)) {
		sem_wait(&mx_estado_exit);
		cambiar_a_estado(estado_exit, entrenador);
		log_info(logger, "luego de ejecutar el trade, el entrenador cumplio su objetivo personal y se mueve a exit");
		sem_post(&mx_estado_exit);
	} else {
		if(!esta_en_estado(estado_block, entrenador)) {
			sem_wait(&mx_estado_block);
			cambiar_a_estado(estado_block, entrenador);
			log_info(logger, "luego de ejecutar el trade, el entrenador no cumplio su objetivo personal y vuelve a block");
			sem_post(&mx_estado_block);
		}
	}
}

void manejar_desalojo_captura(t_pedido_captura* pedido){
	if(!(pedido -> entrenador -> tire_accion) && pedido -> entrenador -> pasos_a_moverse <= 0) { // RR DESALOJADO
		sem_wait(&mx_estado_ready);
		cambiar_a_estado(estado_ready, pedido -> entrenador);
		sem_post(&mx_estado_ready);
		sem_post(&entrenadores_ready);
		log_info(logger, "el entrenador termino su quantum y vuelve a ready");
		pedido -> entrenador -> pasos_a_moverse = config -> quantum;


	} else if(!(pedido -> entrenador -> tire_accion)) { // SJF Con Desalojo DESALOJADO


	} else { // FIFO o SJF Sin Desalojo // RR/SJF NO DESALOJADO
		//sem_wait(&(pedido -> entrenador -> esperar_caught)); // darle verde en el process request
		procesar_caught(pedido);

	}
	sem_post(&mx_estado_exec);
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

	//log_info(logger, "ENTRE A PROCESAR CAUGHT"); // delete
	pedido -> entrenador -> resultado_caught = 1; // delete
	if(pedido -> entrenador -> resultado_caught) { // settear a lo que corresponda en el process_request ANTES de cambiar el esperando_caught
		log_info(logger, "ATRAPE :)"); // delete
		list_add(pedido -> entrenador -> pokemons, pedido -> pokemon -> nombre);

		if(tengo_la_mochila_llena(pedido -> entrenador)){

			if(!estoy_en_deadlock(pedido -> entrenador)){
				sem_wait(&mx_estado_exit);
				cambiar_a_estado(estado_exit, pedido -> entrenador);
				sem_post(&mx_estado_exit);
				log_info(logger, "El entrenador completo su objetivo personal y se mueve a exit");
			} else {
				sem_post(&sem_cont_entrenadores_a_replanif);
			}

		} else{
			sem_post(&sem_cont_entrenadores_a_replanif);
		}

	} else {
		log_info(logger, "NO ATRAPE :("); // delete
		list_add(objetivo_global, pedido -> pokemon -> nombre);
		sem_post(&sem_cont_entrenadores_a_replanif);
	}
	pedido -> entrenador -> tire_accion = 0;
	// sacar el pedido de los pedidos
	eliminar_pedido_captura(pedido);

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
		return otro_pedido -> entrenador == pedido -> entrenador && otro_pedido -> pokemon == pedido -> pokemon;
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

		return es_el_mismo_pedido_intercambio(pedido, un_pedido);
	}

	list_remove_by_condition(pedidos_intercambio, es_el_pedido);
}

bool es_el_mismo_pedido_intercambio(t_pedido_intercambio* pedido, t_pedido_intercambio* un_pedido) {

	return pedido -> entrenador_buscando == un_pedido -> entrenador_buscando &&
			pedido -> entrenador_esperando == un_pedido -> entrenador_esperando &&
			string_equals_ignore_case(pedido -> pokemon_a_dar, pedido -> pokemon_a_dar) &&
			string_equals_ignore_case(pedido -> pokemon_a_dar, pedido -> pokemon_a_recibir);
}

void destruir_pokemon(t_pokemon_mapa* pokemon) {
	//free(pokemon -> nombre); FALTA MALLOC AL AGREGAR
	free(pokemon);
}

void capturar_pokemon(t_pedido_captura* pedido) {

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
		sem_wait(&mx_estado_block);
		cambiar_a_estado(estado_block, pedido -> entrenador);
		sem_post(&mx_estado_block);

		log_info(logger, "mande el catch para un %s en la posicion [%d,%d] y se mueve a block",
				pedido -> pokemon -> nombre, pedido -> entrenador -> posicion[0], pedido -> entrenador -> posicion[1]);
		pedido -> entrenador -> tire_accion = 1;
	}
	sleep(config -> retardo_cpu);// RETARDO_CICLO_CPU
	pedido -> entrenador -> pasos_a_moverse --;
}

void tradear_pokemon(t_pedido_intercambio* pedido){
	if(pedido -> entrenador_buscando -> posicion[0] < pedido -> entrenador_esperando -> posicion[0]) {
		pedido -> entrenador_buscando -> posicion[0] ++;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando-> posicion[1]);
	} else if(pedido -> entrenador_buscando -> posicion[0] > pedido -> entrenador_esperando -> posicion[0]) {
		pedido -> entrenador_buscando -> posicion[0] --;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando -> posicion[1]);
	} else if(pedido -> entrenador_buscando -> posicion[1] < pedido -> entrenador_esperando -> posicion[1]) {
		pedido -> entrenador_buscando -> posicion[1] ++;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando -> posicion[1]);
	} else if(pedido -> entrenador_buscando -> posicion[1] > pedido -> entrenador_esperando -> posicion[1]) {
		pedido -> entrenador_buscando -> posicion[1] --;
		pedido -> distancia --;
		log_info(logger, "me movi a [%d,%d]", pedido -> entrenador_buscando -> posicion[0], pedido -> entrenador_buscando-> posicion[1]);
	} else { // misma posicion

		pedido -> entrenador_buscando -> tire_accion = 1;
		ejecutar_trade(pedido);

	}
	pedido -> entrenador_buscando -> pasos_a_moverse --;
	sleep(config -> retardo_cpu);
}

void ejecutar_trade(t_pedido_intercambio* pedido) {

	bool es_el_pokemon_a_dar(void* un_pokemon){

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

	log_info(logger, "intercambio %s por %s", pedido -> pokemon_a_dar, pedido -> pokemon_a_recibir);

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
		sem_wait(&sem_cont_mapa); //hace el signal cuando llega un mensaje de broker/gameboy
		sem_wait(&sem_cont_entrenadores_a_replanif);
		if(mapa_pokemons -> elements_count) {

			t_pedido_captura* pedido = malloc(sizeof(t_pedido_captura));
			armar_pedido_captura(pedido);

			eliminar_pokemon_de_mapa(pedido -> pokemon);
			planificar_segun_algoritmo(pedido);

			//free(pedido); -- VER CON VALGRIND


		} else {
			//matar hilo gameboy
			//matar suscripciones broker
			resolver_deadlocks_fifo_o_sjf();
			deadlocks = 0;
		}
	}

	return 0;
}

void resolver_deadlocks_fifo_o_sjf() {
	while(estado_exit -> elements_count < config -> entrenadores -> elements_count) {

		if(config -> entrenadores -> elements_count - estado_exit -> elements_count < 2) {
			log_error(logger, "Tengo poca gente para resolver deadlocks!");
		}

		t_pedido_intercambio* pedido = armar_pedido_intercambio_segun_algoritmo();

		cambiar_a_estado(estado_exec, pedido -> entrenador_buscando);
		sem_post(&(pedido -> entrenador_buscando -> sem_binario));

		sem_wait(&mx_estado_exec);
		sem_post(&(pedido -> entrenador_esperando -> sem_binario));
		sem_wait(&mx_estado_exec);
	}
}

void resolver_deadlocks_rr() {



}

t_pedido_intercambio* armar_pedido_intercambio_segun_algoritmo(){
	if(estado_block -> elements_count > 1) { // SE VA CON EL COMM DE ABAJO
	log_info(logger, "somos %d en block, armo pedido", estado_block -> elements_count);
	t_pedido_intercambio* pedido = malloc(sizeof(t_pedido_intercambio));
	pedido -> entrenador_buscando = estado_block -> head -> data; // CHEQUEAR QUE ESTE NO ESTE ESPERANDO

	pedido -> pokemon_a_recibir = encontrar_pokemon_faltante(pedido -> entrenador_buscando);

	bool entrenador_que_le_sobra_pokemon(void* un_entrenador){

		t_entrenador* entrenador = un_entrenador;

		return le_sobra_pokemon(entrenador, pedido -> pokemon_a_recibir);
	}

	pedido -> entrenador_esperando = list_find(estado_block, entrenador_que_le_sobra_pokemon);

	if(!(pedido -> entrenador_esperando)){
		log_error(logger, "a nadie le sobra mi pokemon");
	}

	pedido -> distancia = distancia(pedido -> entrenador_buscando -> posicion, pedido -> entrenador_esperando -> posicion) + 1;

	if(string_equals_ignore_case(config -> algoritmo_planificacion, "RR")) {
		pedido -> entrenador_buscando -> pasos_a_moverse = config -> quantum;

	} else {
		pedido -> entrenador_buscando -> pasos_a_moverse = pedido -> distancia;
	}

	pedido -> pokemon_a_dar = encontrar_pokemon_sobrante(pedido -> entrenador_buscando);

	list_add(pedidos_intercambio, pedido);
	sem_wait(&mx_estado_ready);
	cambiar_a_estado(estado_ready, pedido -> entrenador_buscando);
	sem_post(&mx_estado_ready);
	return pedido;
	}
	return NULL;
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

	return pokemon_faltante;
}

bool tengo_la_mochila_llena(t_entrenador* entrenador) {
	return entrenador -> pokemons -> elements_count >= entrenador -> objetivos -> elements_count;
}

int estado_block_replanificable_no_interbloqueado() {
	bool encontrar_replanificable_no_interbloqueado(void* un_entrenador) {
		t_entrenador* entrenador = un_entrenador;
		if(!(entrenador -> tire_accion) && !tengo_la_mochila_llena(entrenador)) {
			return 1;
		}
		return 0;
	}

	if(list_find(estado_block, encontrar_replanificable_no_interbloqueado)) {

		return 1;
	}
	return 0;
}


void planificar_segun_algoritmo(t_pedido_captura* pedido) {

	pedido -> entrenador -> pasos_a_moverse = distancia_segun_algoritmo(pedido);

	if(string_equals_ignore_case(config -> algoritmo_planificacion, "FIFO") ||
			string_equals_ignore_case(config -> algoritmo_planificacion, "RR")) {

		planificar_fifo_o_rr(pedido);

	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-SD")) {

		planificar_sjf_sd();

	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-CD")) {

		planificar_sjf_cd();
	}
}

void planificar_fifo_o_rr(t_pedido_captura* pedido){
	sem_wait(&mx_estado_ready);
	cambiar_a_estado(estado_ready, pedido -> entrenador);
	log_info(logger, "entrenador cambiado a estado ready con su pedido de captura");
	sem_post(&mx_estado_ready);
	sem_post(&entrenadores_ready);
}

void planificar_sjf_sd(){

}

void planificar_sjf_cd(){

}

int distancia_segun_algoritmo(t_pedido_captura* pedido) {
	if(string_equals_ignore_case(config -> algoritmo_planificacion, "FIFO") ||
			string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-SD")) {

		return pedido -> distancia + 1;
	}
	if(string_equals_ignore_case(config -> algoritmo_planificacion, "RR")) {
		return config -> quantum;
	}
	if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-CD")) {
		return pedido -> distancia + 1; // NO SE SI ESTE VA ASI.
	}
	log_error(logger, "algoritmo de planificacion recibido: %s", config -> algoritmo_planificacion);
	return 0;
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
			string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-SD")) {
		uint32_t err = pthread_create(&hilo_algoritmo, NULL, ejecutar_fifo_o_rr_o_sjf_sd, NULL);
		if(err != 0) {
			log_error(logger, "el hilo no pudo ser creado"); // preguntar si estos logs se pueden hacer
		}
	} else if(string_equals_ignore_case(config -> algoritmo_planificacion, "SJF-CD")) {
		log_error(logger, "NO ESTA CODEADO SJF-CD");
		uint32_t err = pthread_create(&hilo_algoritmo, NULL, ejecutar_sjf_cd, NULL);
		if(err != 0) {
			log_error(logger, "el hilo no pudo ser creado");
		}

	} else {
		log_error(logger, "algoritmo de planificacion recibido: %s", config -> algoritmo_planificacion);
	}
}

void crear_hilo_planificar_entrenadores() {

	uint32_t err = pthread_create(&hilo_entrenadores, NULL, planificar_entrenadores, NULL);
	if(err != 0) {
		log_error(logger, "el hilo no pudo ser creado");
	}
}

void* ejecutar_fifo_o_rr_o_sjf_sd() {

	while(config -> entrenadores -> elements_count > estado_exit -> elements_count) { // matamos este hilo o muere con el programa?

		sem_wait(&mx_estado_exec);
		sem_wait(&entrenadores_ready);
		if(!estado_exec -> head && estado_ready -> head) {
			t_entrenador* entrenador = estado_ready -> head -> data;
			cambiar_a_estado(estado_exec, entrenador);
			log_info(logger, "entrenador cambiado a estado exec");
			sem_post(&(entrenador -> sem_binario));
		}
	}
	return 0;
}

void* ejecutar_sjf_cd() {
	// VER QUE AGARRE LA CABEZA EN DEADLOCK


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
			t_get_pokemon* mensaje_recibido = desempaquetar_get_pokemon(msg);
			agregar_mensaje(GET_POKEMON, size, mensaje_recibido, cliente_fd);
			free(msg);
			break;
		case LOCALIZED_POKEMON:
			msg = malloc(sizeof(t_localized_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			t_localized_pokemon* mensaje_recibido = desempaquetar_localized_pokemon(msg);
			agregar_mensaje(LOCALIZED_POKEMON, size, mensaje_recibido, cliente_fd);
			free(msg);
			break;
		case CAUGHT_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			t_caught_pokemon* mensaje_recibido = despaquetar_caught_pokemon(msg);
			agregar_mensaje(CAUGHT_POKEMON, size,  mensaje_recibido, cliente_fd);
			free(msg);
			break;
		case APPEARED_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			t_appeared_pokemon* mensajeRecibido = despaquetar_appeared_pokemon(msg);
			agregar_mensaje(APPEARED_POKEMON, size, mensajeRecibido, cliente_fd);
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
	list_destroy(estados);
}

void liberar_semaforos() {
	sem_destroy(&mx_estado_new);
	sem_destroy(&mx_estado_exec);
	sem_destroy(&mx_estado_block);
	sem_destroy(&mx_estado_exit);
	sem_destroy(&mx_estado_ready);
	sem_destroy(&entrenadores_ready);
	sem_destroy(&sem_cont_mapa);
	sem_destroy(&sem_cont_entrenadores_a_replanif);
}

void liberar_listas(){
	list_destroy(objetivo_global); // puede ser solo free
	list_destroy(pedidos_intercambio);
	list_destroy(pedidos_captura);
}

void terminar_programa(/*uint32_t conexion*/) {
	free(mapa_pokemons); // ver, se va destruyendo como objetivo global?
	liberar_logger();
	//liberar_conexion(conexion);
	liberar_listas();
	liberar_estados();
	liberar_semaforos();
	liberar_config();
}
