#include "team.h"

int main(void) {

	iniciar_programa();
	//abrir_conexion(); abrir socket con el gameBoy
	int socket = crear_conexion(config -> ip_broker, config -> puerto_broker);

	enviar_mensaje(TE_GET_POKEMON_BR, "Get Pokemon", socket);

	//t_buffer* recibido = recibir_mensaje(socket, strlen("Hola")+ 1);
	log_info(logger, "El ip es : %s", config -> ip_broker);
	log_info(logger, "El port es : %s ", config -> puerto_broker);
	terminar_programa(socket, logger, config);
}

void iniciar_programa(){
	iniciar_logger("team.log", "team");
	leer_config(); // aca agregamos los elementos a los campos q corresponde
	//objetivo_global = obtener_objetivo_global();
	//crear_hilos_entrenadores(); // iniciar a los entrenadores
}

/*t_list* obtener_objetivo_global(){ // usar el iterate

// tener en cuenta que necesitamos saber la especie y cantidad de cada uno

	t_list* objetivos;
	t_list* aux_lista_lista = config -> objetivos_entrenadores;
	t_list* aux_lista = aux_lista_lista -> pokemons;

	while(aux_lista_lista -> next != NULL){
		while(aux_lista -> next != NULL){

			objetivos -> pokemon = aux_lista -> pokemon;
			objetivos -> next = aux_lista -> next;
			aux_lista = aux_lista -> next;
		}

		aux_lista_lista = aux_lista_lista -> next;
	}

	return objetivos;

}*/

void leer_config(void) {

	t_config_team* config_team = malloc(sizeof(t_config_team));

	t_config* config = config_create("Debug/team.config");

	config_team -> log_file = strdup(config_get_string_value(config, "LOG_FILE"));

	char** posiciones = config_get_array_value(config, "POSICIONES_ENTRENADORES");
	char** pokemons = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** objetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	void* posicion = parsear(posiciones);
	posicion = parsear(pokemons);


	config_team -> tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	config_team -> retardo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team -> algoritmo_planificacion = strdup(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
	config_team -> ip_broker = strdup(config_get_string_value(config, "IP_BROKER"));
	config_team -> puerto_broker = strdup(config_get_string_value(config, "PUERTO_BROKER"));
	config_team -> estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");

	//parsear(posiciones); //si esto retorna un t_list* podria usarse add all
	//parsear(pokemons);
	//parsear(objetivos);

	config_destroy(config);

}

void* parsear(char** datos_de_config) { // no se si void o q retorne lo parseado y asignarlo al struct en leer_config
	t_list* lista = list_create();
	printf("%d", sizeof(datos_de_config));
	char e;
	char* palabra;
	for (char* c = *datos_de_config; c; c=*++datos_de_config) {
		for (char* d = c; d; d++) {
			e = *d;

			if(!e) {
				break;
			} else if(e != '|') {
				concatenar(palabra, e);
				//string_append(&palabra, (char*) e);
			} else {
				//list_add(lista, palabra);
				palabra = ""; // limpiar char *
			}
		}
	}
}

/*void crear_hilos_entrenadores(){

	while(posiciones_entrenadores != NULL){ // o iterate si se puede
		int err = pthread_create(hilo, NULL, iniciar_entrenador, entrenador);
		if(id =! 0){
			el hilo se creo mal
			quizas retornar err para tratar el error con lo de las commons
		}
	}
}*/

//void* iniciar_entrenador() {

//}

void concatenar(char* palabra, char caracter) {
	int len = string_length(palabra);
	palabra[len] = caracter;
}

void liberar_config(t_config_team* config) {
	free(config -> algoritmo_planificacion);
	free(config -> log_file);
	free(config -> ip_broker);
	free(config -> puerto_broker);
	free(config);
}

void terminar_programa(int conexion,t_log* logger,t_config_team* config) {
	liberar_config(config);
	liberar_logger(logger);
	liberar_conexion(conexion);
}
