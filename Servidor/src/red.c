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
	cJSON_AddStringToObject(json, "type", "NEW_LIST");

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

void EnviarMensajePrivado(int socket, const char* usuarioRemitente, const char* texto) {}
void EnviarMensajePublico(int socket, const char* usuarioRemitente, const char* texto) {}

void EnviarInvitacion(int socket, const char* usuarioAnfitrion, const char* roomname) {}
void EnviarUnionSala(int socket, const char* roomname, const char* usuario) {}
void EnviarListaSala(int socket, const char* roomname, struct EstadoGlobal* estado) {}
void EnviarTextoSala(int socket, const char* roomname, const char* usuarioRemitente, const char* texto) {}
void EnviarAbandono(int socket, const char* roomname, const char* usuario) {}