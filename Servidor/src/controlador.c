#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "../include/controlador.h"
#include "../include/red.h"
#include "../lib/cJSON.h"

#define MAX_MSG_SIZE 1048576
#define CHUNK_SIZE 4096

void* IniciarHilo(void* argumento) {
	struct ControladorCliente* cliente = (struct ControladorCliente*)argumento;
	IniciarLectura(cliente);
	ManejarDesconexion(cliente);
	free(cliente);
	return NULL;
}

void IniciarLectura(struct Cliente* cliente) {
	char* buffer = (char*)malloc(MAX_MSG_SIZE);
	if (buffer == NULL) {
		fprintf(stderr, "Memoria insuficiente para el bufer.");
		return;
	}
	
	size_t buffer_len = 0;

	while (true) {
		size_t espacioLibre = MAX_MSG_SIZE - buffer_len;

		if (espacioLibre == 0){
			break;
		}

		size_t bytesPorLeer = (espacioLibre < CHUNK_SIZE) ? espacioLibre : CHUNK_SIZE;

		ssize_t n_recv = recv(cliente->socket, buffer + buffer_len, bytesPorLeer, 0);

		if (n_recv <= 0){
			break;
		}

		buffer_len += n_recv;

		while (true){
			char* nl = (char*)memchr(buffer, '\n', buffer_len);
			
			if (!nl){
				break;
			}
			
			size_t mensajeLong = (size_t)(nl - buffer);

			char* mensajeJSON = (char*)malloc(mensajeLong + 1);
			if (mensajeJSON != NULL){
				memcpy(mensajeJSON, buffer, mensajeLong);
				mensajeJSON[mensajeLong] = '\0';

				if (mensajeLong > 0 && mensajeJSON[0] != '\r'){
					ControladorCliente_EnrutarMensaje(cliente, mensajeJSON);
					printf(mensajeJSON);
				}

				free(mensajeJSON);
			}

			size_t restantes = buffer_len - (mensajeLong + 1);
			memmove(buffer, nl + 1, restantes);
			buffer_len = restantes;
		}
	}

	free(buffer);
}

void EnrutarMensaje (struct Cliente* this, char* mensajeBruto){
	cJSON* json = cJSON_Parse(mensajeBruto);

	if (json == NULL) {
		EnviarErrorCritico(this->socket);
		return;
	}

	cJSON* item = cJSON_GetObjectItemCaseSensitive(json, "type");
	if (!cJSON_IsString(item) || item->valuestring == NULL) {
		cJSON_Delete(json);
		EnviarErrorCritico(this->socket);
		return;
	}

	char* tipo = item->valuestring;

	if(!this->identificado && strcmp(tipo, "IDENTIFY") != 0) {
		cJSON_Delete(json);
		EnviarErrorCritico(this->socket);
		return;
	}

	if (strcmp(tipo, "IDENTIFY") == 0) {
		cJSON* item = cJSON_GetObjectItemCaseSensitive(json, "username");

		if(cJSON_IsString(item) && item->valuestring != NULL) {
			if (strlen(item->valuestring) <= 8) {
				bool registrado = RegistrarUsuario(this->estado, item->valuestring, this->socket);

				if (registrado) {
					this->identificado = true;
					strcpy(this->username, item->valuestring);

				} else {

				}
			} else {
				EnviarErrorCritico(this->socket);
			}
		} else {
			EnviarErrorCritico(this->socket);
		}
	} else if (strcmp(tipo, "STATUS") == 0){
		cJSON* item = cJSON_GetObjectItemCaseSensitive(json, "status");

		if (cJSON_IsString(item) && item->valuestring != NULL) {
			char* nuevoEstado = item->valuestring;

			if (strcmp(nuevoEstado, "ACTIVE") == 0 ||
				strcmp(nuevoEstado, "AWAY") == 0 ||
				strcmp(nuevoEstado, "BUSY") == 0){
					ActualizarEstadoUsuario(this->estado, this->username, nuevoEstado);
					EnviarNuevoEstado(this->estado, this->username, nuevoEstado);
				} else {
					EnviarErrorCritico(this->socket);
				}
		} else {
			EnviarErrorCritico(this->socket);
		}
	} else if (strcmp(tipo, "DISCONNECT") == 0) {
		ManejarDesconexion(this);
	} else {
		EnviarErrorCritico(this->socket);
	}

	cJSON_Delete(json);
}

void ManejarDesconexion(struct Cliente* this) {
	if (this->identificado){
		EliminarUsuario(this->estado, this->username);
		EnviarDesconexion(this->estado,this->username);
		this->identificado = false;
	}
	if (this->socket != -1) {
		close(this->socket);
		this->socket = -1;
	}
}

