#include "broker.h"


void enviar_mensajes_get(){
	void enviar_mensaje_get(void* mensaje){
		t_mensaje* mensaje_a_enviar = malloc(sizeof(t_mensaje));
	    mensaje_a_enviar = mensaje;
	    if(mensaje_a_enviar == EN_ESPERA) {
		enviar_mensaje(GET_POKEMON, mensaje_a_enviar-> mensaje, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		free(mensaje_a_enviar);
	    }

	}
	//Habría que ver como iterar on la lista de mensajes, porque estamos usando la lista de suscriptores
	list_iterate(listas_de_suscriptos -> lista_suscriptores_get, enviar_mensaje_get); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?



}


/*REVISAR 0_0
void enviar_mensajes_catch(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes catch y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_catch, 0);

	void enviar_mensaje_catch(void* socket){
		enviar_mensaje(CATCH_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_catch, enviar_mensaje_catch); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);
}

void enviar_mensajes_localized(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes localized y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_localized, 0);

	void enviar_mensaje_localized(void* socket){
		enviar_mensaje(LOCALIZED_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_localized, enviar_mensaje_localized); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);

}

void enviar_mensajes_caught(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes caught y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_caught, 0);

	void enviar_mensaje_caught(void* socket){
		enviar_mensaje(CAUGHT_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_caught, enviar_mensaje_caught); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);
}

void enviar_mensajes_appeared(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes appeared y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_appeared, 0);

	void enviar_mensaje_appeared(void* socket){
		enviar_mensaje(APPEARED_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_appeared, enviar_mensaje_appeared); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);
}*/
