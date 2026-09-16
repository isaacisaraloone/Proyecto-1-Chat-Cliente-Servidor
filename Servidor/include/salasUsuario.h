#ifndef SALASUSUARIO_H
#define SALASUSUARIO_H

#include <pthread.h>
#include <stdbool.h>
#include "uthash.h"

#define MAX_USERNAME_LEN 9
#define MAX_ROOMNAME_LEN 17
#define MAX_STATUS_LEN 10

struct Usuario {
	char username[MAX_USERNAME_LEN];
	int socket;
	char status[MAX_STATUS_LEN];
	UT_hash_handle hh;
};

struct MiembroSala {
	char roomname[MAX_ROOMNAME_LEN];
	struct Usuario* usuario;
	UT_hash_handle hh;
};

struct Sala
{
	char roomname[MAX_ROOMNAME_LEN];
	struct MiembroSala* activos;
	struct MiembroSala* invitados;
	UT_hash_handle hh;
};

struct EstadoGlobal {
	struct Usuario* listaUsuarios;
	struct Sala* listaSalas;

	pthread_mutex_t mutexUsuarios;
	pthread_mutex_t mutexSalas;
};

void Inicializar(struct EstadoGlobal* estado);
void Destruir(struct EstadoGlobal* estado);

bool RegistrarUsuario(struct EstadoGlobal* estado, const char* username, int socket);
bool ExisteUsuario(struct EstadoGlobal* estado, const char* username);
void EliminarUsuario(struct EstadoGlobal* estado, const char* username);
void ActualizarEstadoUsuario();

int CrearSala();
int Invitacion();
int UnirseSala();
void DejarSala();

#endif // SALASUSUARIO_H