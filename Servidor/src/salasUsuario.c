#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/salasUsuario.h"

void Incializar(struct EstadoGlobal* estado) {
	estado->listaUsuarios = NULL;
	estado->listaSalas = NULL;
	
	if (pthread_mutex_init(&estado->mutexUsuarios, NULL) != 0) {
		perror("Fallo al inicializar mutexUsuarios");
		exit(EXIT_FAILURE);
	}
	if (pthread_mutex_init(&estado->mutexSalas, NULL) != 0) {
		perror("Fallo al inicializar mutexSalas");
		exit(EXIT_FAILURE);
	}
}

void Destruir(){}

bool RegistrarUsuario(struct EstadoGlobal* estado, const char* username, int socket){
	struct Usuario* usuarioExistente = NULL;
	bool registroExitoso = false;

	pthread_mutex_lock(&estado->mutexUsuarios);
	HASH_FIND_STR(estado->listaUsuarios, username, usuarioExistente);

	if (usuarioExistente == NULL) {
		struct Usuario* nuevoUsuario = (struct Usuario*)malloc(sizeof(struct Usuario));
		strncpy(nuevoUsuario->username, username, MAX_USERNAME_LEN - 1);
		nuevoUsuario->username[MAX_USERNAME_LEN - 1] = '\0';
		nuevoUsuario->socket = socket;
		strcpy(nuevoUsuario->status, "ACTIVE");

		HASH_ADD_STR(estado->listaUsuarios, username, nuevoUsuario);
		registroExitoso = true;
	}

	pthread_mutex_unlock(&estado->mutexUsuarios);
	return registroExitoso;
}

bool ExisteUsuario(struct EstadoGlobal* estado, const char* username) {
	struct Usuario* usuarioExistente = NULL;
	bool existe = false;

	pthread_mutex_lock(&estado->mutexUsuarios);
	HASH_FIND_STR(estado->listaUsuarios, username, usuarioExistente);
	if (usuarioExistente != NULL) {
		existe = true;
	}
	pthread_mutex_unlock(&estado->mutexUsuarios);
	return existe;
}

void EliminarUsuario(struct EstadoGlobal* estado, const char* username){
	struct Usuario* usuarioExistente = NULL;
	pthread_mutex_lock(&estado->mutexUsuarios);
	HASH_FIND_STR(estado->listaUsuarios, username, usuarioExistente);
	if (usuarioExistente != NULL) {
		HASH_DEL(estado->listaUsuarios, usuarioExistente);
		free(usuarioExistente);
	}
	pthread_mutex_unlock(&estado->mutexUsuarios);
};

void ActualizarEstadoUsuario(){}

int CrearSala(){}

int Invitacion(){}

int UnirseSala(){}

void DejarSala(){}