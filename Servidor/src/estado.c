#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/estado.h"

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

void Destruir(struct EstadoGlobal* estado){
	struct Usuario *usuarioActual, *usuarioTemp;
	struct Sala *salaActual, *salaTemp;
	struct MiembroSala *miembroActual, *miembroTemp;

	pthread_mutex_lock(&estado->mutexUsuarios);
	HASH_ITER(hh, estado->listaUsuarios, usuarioActual, usuarioTemp){
		HASH_DEL(estado->listaUsuarios, usuarioActual);
		free(usuarioActual);
	}
	pthread_mutex_unlock(&estado->mutexUsuarios);

	pthread_mutex_lock(&estado->mutexSalas);

	HASH_ITER(hh, estado->listaSalas, salaActual, salaTemp){

		HASH_ITER(hh, salaActual->activos, miembroActual, miembroTemp){
			HASH_DEL(salaActual->activos, miembroActual);
			free(miembroActual);
		}

		HASH_ITER(hh, salaActual->invitados, miembroActual, miembroTemp){
			HASH_DEL(salaActual->invitados, miembroActual);
			free(miembroActual);
		}
		HASH_DEL(estado->listaSalas, salaActual);
		free(salaActual);
	}

	pthread_mutex_unlock(&estado->mutexSalas);

	pthread_mutex_destroy(&estado->mutexUsuarios);
	pthread_mutex_destroy(&estado->mutexSalas);
}

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

void ActualizarEstadoUsuario(struct EstadoGlobal* estado, const char* username, const char* nuevoEstatus){
	struct Usuario* usuario = NULL;

	pthread_mutex_lock(&estado->mutexUsuarios);
	HASH_FIND_STR(estado->listaUsuarios, username, usuario);

	if (usuario != NULL){
		strncpy(usuario->status, nuevoEstatus, MAX_STATUS_LEN - 1);
		usuario->status[MAX_STATUS_LEN - 1] = '\0';
	}

	pthread_mutex_unlock(&estado->mutexUsuarios);
}

int CrearSala(struct EstadoGlobal* estado, const char* roomname, const char* creadorUsername){
	struct Sala* salaExistente = NULL;
	int resultado = 0;

	pthread_mutex_lock(&estado->mutexSalas);
	HASH_FIND_STR(estado->listaSalas, roomname, salaExistente);

	if (salaExistente != NULL) {
		resultado = 2;
	} else {
		struct Sala* nuevaSala = (struct Sala*)malloc(sizeof(struct Sala));
		if (nuevaSala != NULL) {
			strncpy(nuevaSala->roomname, roomname, MAX_ROOMNAME_LEN - 1);
			nuevaSala->roomname[MAX_ROOMNAME_LEN - 1] = '\0';
			nuevaSala->activos = NULL;
			nuevaSala->invitados = NULL;

			struct MiembroSala* creador = (struct MiembroSala*)malloc(sizeof(struct MiembroSala));
			strncpy(creador->username, creadorUsername, MAX_USERNAME_LEN - 1);
			creador->username[MAX_USERNAME_LEN - 1] = '\0';
			creador->usuario = NULL;

			HASH_ADD_STR(nuevaSala->activos, username, creador);
			HASH_ADD_STR(estado->listaSalas, roomname, nuevaSala);

			resultado = 1;

		}
		
	}

	pthread_mutex_unlock(&estado->mutexSalas);
	return resultado;
}

int Invitacion(struct EstadoGlobal* estado, const char* roomname, const char* username, const char* invitadoPor){
	struct Sala* sala = NULL;
	int resultado = 0;

	pthread_mutex_lock(&estado->mutexSalas);
	HASH_FIND_STR(estado->listaSalas, roomname, sala);

	if (sala == NULL){
		resultado = 2;
	} else {
		struct MiembroSala* anfitrion = NULL;
		HASH_FIND_STR(sala->activos, invitadoPor, anfitrion);

		if (anfitrion == NULL) {
			resultado = 4;
		} else {
			struct MiembroSala* yaDentro = NULL;
			struct MiembroSala* yaInvitado = NULL;
			HASH_FIND_STR(sala->activos, username, yaDentro);
			HASH_FIND_STR(sala->invitados, username, yaInvitado);

			if (yaDentro != NULL || yaInvitado != NULL){
				resultado = 3;
			} else {
				struct MiembroSala* nuevoInvitado = (struct MiembroSala*)malloc(sizeof(struct MiembroSala));
				strncpy(nuevoInvitado->username, username, MAX_USERNAME_LEN - 1);
				nuevoInvitado->username[MAX_USERNAME_LEN - 1] = '\0';

				HASH_ADD_STR(sala->invitados, username, nuevoInvitado);
				resultado = 1;
			}
		}
	}
	pthread_mutex_unlock(&estado->mutexSalas);
	return resultado;
}

int UnirseSala(struct EstadoGlobal* estado, const char* roomname, const char* username){
	struct Sala* sala = NULL;
	int resultado = 0;
	pthread_mutex_lock(&estado->mutexSalas);
	HASH_FIND_STR(estado->listaSalas, roomname, sala);

	if (sala == NULL){
		resultado = 2;
	} else{
		struct MiembroSala* invitado = NULL;
		HASH_FIND_STR(sala->invitados, username, invitado);

		if (invitado == NULL) {
			resultado = 3;
		} else {
			HASH_DEL(sala->invitados,invitado);
			HASH_ADD_STR(sala->activos, username, invitado);
			resultado = 1;
		}
	}
	
	pthread_mutex_unlock(&estado->mutexSalas);
	return resultado;	
}

void DejarSala(struct EstadoGlobal* estado, const char* roomname, const  char* username){
	struct Sala* sala = NULL;

	pthread_mutex_lock(&estado->mutexSalas);
	HASH_FIND_STR(estado->listaSalas, roomname, sala);

	if (sala != NULL) {
		struct MiembroSala* miembro = NULL;
		HASH_FIND_STR(sala->activos, username, miembro);

		if (miembro != NULL) {
			HASH_DEL(sala->activos, miembro);
			free(miembro);

			if (sala->activos == NULL) {

				struct MiembroSala *invitadoActual, *invitadoTemp;
				HASH_ITER(hh, sala->invitados, invitadoActual, invitadoTemp) {
					HASH_DEL(sala->invitados, invitadoActual);
					free(invitadoActual);
				}

				HASH_DEL(estado->listaSalas, sala);
				free(sala);
			}
		}
		
	}

	pthread_mutex_unlock(&estado->mutexSalas);

}