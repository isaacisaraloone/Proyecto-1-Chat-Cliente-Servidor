#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/red.h"
#include "../lib/cJSON.h"

static void EnviarLimpiarJSON(int socket, cJSON* json) {
	char* textoPlano = cJSON_PrintUnformatted(json);

	if (textoPlano != NULL) {
		size_t len = strlen(textoPlano);

		char* salida = (char*)malloc(len + 2);
		if (salida != NULL) {
			snprintf(salida, len + 2, "%s\n", textoPlano);
			send(socket, salida, len + 1, 0);
			free(salida);
		}

		free(textoPlano);
	}

	cJSON_Delete(json);
}

void EnviarRespuesta(int socket, const char* operacion, const char* resultado, const char* extra) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "RESPONSE");
	cJSON_AddStringToObject(json, "operation", operacion);
	cJSON_AddStringToObject(json, "result", resultado);

	if (extra != NULL) {
		cJSON_AddStringToObject(json, "extra", extra);
	}
	EnviarLimpiarJSON(socket, json);
}

void EnviarErrorCritico(int socket) {
	const char* errorMensaje = "{\"type\":\"RESPONSE\",\"operation\":\"INVALID\",\"result\":\"INVALID\"}\n";
	send(socket, errorMensaje, strlen(errorMensaje), 0);
	close(socket);
}

void EnviarNuevoUsuario(int socket, const char* usuario) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "NEW_USER");
	cJSON_AddStringToObject(json, "username", usuario);
	EnviarLimpiarJSON(socket, json);
}

void EnviarNuevoEstado(int socket, const char* usuario, const char* estado) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "NEW_STATUS");
	cJSON_AddStringToObject(json, "username", usuario);
	cJSON_AddStringToObject(json, "status", estado);
	EnviarLimpiarJSON(socket, json);
}

void EnviarListaUsuarios(int socket, struct EstadoGlobal* estado) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "USER_LIST");

	cJSON* listaUsuarios = cJSON_CreateObject();

	pthread_mutex_lock(&estado->mutexUsuarios);
	struct Usuario *usuarioActual,*usuarioTemp;
	HASH_ITER(hh, estado->listaUsuarios, usuarioActual, usuarioTemp){
		cJSON_AddStringToObject(listaUsuarios, usuarioActual->username, usuarioActual->status);
	}
	pthread_mutex_unlock(&estado->mutexUsuarios);

	cJSON_AddItemToObject(json, "users", listaUsuarios);
	EnviarLimpiarJSON(socket, json);
}

void EnviarDesconexion(int socket, const char* usuario) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "DISCONNECTED");
	cJSON_AddStringToObject(json, "username", usuario);
	EnviarLimpiarJSON(socket, json);
}

void EnviarMensajePrivado(int socket, const char* usuarioRemitente, const char* texto) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "TEXT_FROM");
	cJSON_AddStringToObject(json, "username", usuarioRemitente);
	cJSON_AddStringToObject(json, "text", texto);
	EnviarLimpiarJSON(socket, json);
}
void EnviarMensajePublico(int socket, const char* usuarioRemitente, const char* texto) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "PUBLIC_TEXT_FROM");
	cJSON_AddStringToObject(json, "username", usuarioRemitente);
	cJSON_AddStringToObject(json, "text", texto);
	EnviarLimpiarJSON(socket, json);
}

void EnviarInvitacion(int socket, const char* usuarioAnfitrion, const char* roomname) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "INVITATION");
	cJSON_AddStringToObject(json, "username", usuarioAnfitrion);
	cJSON_AddStringToObject(json, "roomname", roomname);
	EnviarLimpiarJSON(socket, json);
}

void EnviarUnionSala(int socket, const char* roomname, const char* usuario) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "JOINED_ROOM");
	cJSON_AddStringToObject(json, "roomname", roomname);
	cJSON_AddStringToObject(json, "username", usuario);
	EnviarLimpiarJSON(socket, json);
}

void EnviarListaSala(int socket, const char* roomname, struct EstadoGlobal* estado) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "ROOM_USER_LIST");
	cJSON_AddStringToObject(json, "roomname", roomname);

	cJSON* listaUsuarios = cJSON_CreateObject();

	pthread_mutex_lock(&estado->mutexSalas);
	struct Sala* salaActual = NULL;
	HASH_FIND_STR(estado->listaSalas, roomname, salaActual);

	if (salaActual != NULL) {
		struct MiembroSala *miembroActual, *miembroTemp;
		HASH_ITER(hh, salaActual->activos, miembroActual, miembroTemp){
			const char* estado = (miembroActual->usuario != NULL) ? miembroActual->usuario->status : "ACTIVE";
			cJSON_AddStringToObject(listaUsuarios, miembroActual->username, estado);
		}
	}
	pthread_mutex_unlock(&estado->mutexSalas);

	cJSON_AddItemToObject(json, "users", listaUsuarios);
	EnviarLimpiarJSON(socket, json);
}

void EnviarTextoSala(int socket, const char* roomname, const char* usuarioRemitente, const char* texto) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "ROOM_TEXT_FROM");
	cJSON_AddStringToObject(json, "roomname", roomname);
	cJSON_AddStringToObject(json, "username", usuarioRemitente);
	cJSON_AddStringToObject(json, "text", texto);
	EnviarLimpiarJSON(socket, json);
}

void EnviarAbandono(int socket, const char* roomname, const char* usuario) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "LEFT_ROOM");
	cJSON_AddStringToObject(json, "roomname", roomname);
	cJSON_AddStringToObject(json, "username", usuario);
	EnviarLimpiarJSON(socket, json);
}

static void TransmitirGlobalJSON(struct EstadoGlobal* estado, cJSON* json, const char* usuarioExcluido) {
	char* textoPlano = cJSON_PrintUnformatted(json);
	if (textoPlano != NULL) {
		size_t len = strlen(textoPlano);
		char* salida = (char*)malloc(len + 2);
		
		if (salida != NULL) {
			snprintf(salida, len + 2, "%s\n", textoPlano);
			
			pthread_mutex_lock(&estado->mutexUsuarios);
			struct Usuario *usuarioActual, *usuarioTemp;
			
			HASH_ITER(hh, estado->listaUsuarios, usuarioActual, usuarioTemp) {

				if (usuarioExcluido == NULL || strcmp(usuarioActual->username, usuarioExcluido) != 0) {
					send(usuarioActual->socket, salida, len + 1, 0);
				}
			}
			pthread_mutex_unlock(&estado->mutexUsuarios);
			free(salida);
		}
		free(textoPlano);
	}
	cJSON_Delete(json);
}

void TransmitirNuevoUsuario(struct EstadoGlobal* estado, const char* usuarioNuevo) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "NEW_USER");
	cJSON_AddStringToObject(json, "username", usuarioNuevo);
	TransmitirGlobalJSON(estado, json, usuarioNuevo);
}

void TransmitirNuevoEstado(struct EstadoGlobal* estado, const char* usuario, const char* nuevoEstado) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "NEW_STATUS");
	cJSON_AddStringToObject(json, "username", usuario);
	cJSON_AddStringToObject(json, "status", nuevoEstado);
	TransmitirGlobalJSON(estado, json, usuario);
}

void TransmitirDesconexion(struct EstadoGlobal* estado, const char* usuario) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "DISCONNECTED");
	cJSON_AddStringToObject(json, "username", usuario);
	TransmitirGlobalJSON(estado, json, usuario);
}

void TransmitirMensajePublico(struct EstadoGlobal* estado, const char* usuarioRemitente, const char* texto) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "PUBLIC_TEXT_FROM");
	cJSON_AddStringToObject(json, "username", usuarioRemitente);
	cJSON_AddStringToObject(json, "text", texto);
	TransmitirGlobalJSON(estado, json, usuarioRemitente);
}

static void TransmitirSalaJSON(struct EstadoGlobal* estado, const char* roomname, cJSON* json, const char* usuarioExcluido) {
	char* textoPlano = cJSON_PrintUnformatted(json);
	
	if (textoPlano != NULL) {
		size_t len = strlen(textoPlano);
		char* salida = (char*)malloc(len + 2);
		
		if (salida != NULL) {
			snprintf(salida, len + 2, "%s\n", textoPlano);
			
			pthread_mutex_lock(&estado->mutexSalas);
			struct Sala* salaActual = NULL;
			HASH_FIND_STR(estado->listaSalas, roomname, salaActual);
			
			if (salaActual != NULL) {
				struct MiembroSala *miembroActual, *miembroTemp;
				HASH_ITER(hh, salaActual->activos, miembroActual, miembroTemp) {
					if (usuarioExcluido == NULL || strcmp(miembroActual->username, usuarioExcluido) != 0) {

						if (miembroActual->usuario != NULL) {
							send(miembroActual->usuario->socket, salida, len + 1, 0);
						}
					}
				}
			}
			pthread_mutex_unlock(&estado->mutexSalas);
			free(salida);
		}
		free(textoPlano);
	}
	cJSON_Delete(json);
}

void TransmitirTextoSala(struct EstadoGlobal* estado, const char* roomname, const char* usuarioRemitente, const char* texto) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "ROOM_TEXT_FROM");
	cJSON_AddStringToObject(json, "roomname", roomname);
	cJSON_AddStringToObject(json, "username", usuarioRemitente);
	cJSON_AddStringToObject(json, "text", texto);
	TransmitirSalaJSON(estado, roomname, json, usuarioRemitente);
}

void AvisoUnionSala(struct EstadoGlobal* estado, const char* roomname, const char* username) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "JOINED_ROOM");
	cJSON_AddStringToObject(json, "roomname", roomname);
	cJSON_AddStringToObject(json, "username", username);
	TransmitirSalaJSON(estado, roomname, json, NULL);
}

void AvisoAbandonoSala(struct EstadoGlobal* estado, const char* roomname, const char* username) {
	cJSON* json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "type", "LEFT_ROOM");
	cJSON_AddStringToObject(json, "roomname", roomname);
	cJSON_AddStringToObject(json, "username", username);
	TransmitirSalaJSON(estado, roomname, json, username);
}