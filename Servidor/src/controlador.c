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
	struct Cliente* cliente = (struct Cliente*)argumento;
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
					EnrutarMensaje(cliente, mensajeJSON);
					printf("%s\n",mensajeJSON);
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
        
        if (this->identificado) {
            EnviarRespuesta(this->socket, "IDENTIFY", "ALREADY_IDENTIFIED", NULL);
            return; 
        }

        cJSON* itemUsuario = cJSON_GetObjectItemCaseSensitive(json, "username");

        if(cJSON_IsString(itemUsuario) && itemUsuario->valuestring != NULL) {
            if (strlen(itemUsuario->valuestring) <= 8) {
                bool registrado = RegistrarUsuario(this->estado, itemUsuario->valuestring, this->socket);

                if (registrado) {
                    this->identificado = true;
                    strcpy(this->username, itemUsuario->valuestring);

                    EnviarRespuesta(this->socket, "IDENTIFY", "SUCCESS", NULL);
                    TransmitirNuevoUsuario(this->estado, this->username);
                } else {
                    EnviarRespuesta(this->socket, "IDENTIFY", "USER_ALREADY_EXISTS", NULL);
                }
            } else {
                EnviarErrorCritico(this->socket);
            }
        } else {
            EnviarErrorCritico(this->socket);
        }
        
    } else if (strcmp(tipo, "STATUS") == 0){
		cJSON* itemUsuario = cJSON_GetObjectItemCaseSensitive(json, "status");

		if (cJSON_IsString(itemUsuario) && itemUsuario->valuestring != NULL) {
			char* nuevoEstado = itemUsuario->valuestring;

			if (strcmp(nuevoEstado, "ACTIVE") == 0 ||
				strcmp(nuevoEstado, "AWAY") == 0 ||
				strcmp(nuevoEstado, "BUSY") == 0){
					ActualizarEstadoUsuario(this->estado, this->username, nuevoEstado);
					TransmitirNuevoEstado(this->estado, this->username, nuevoEstado);
				} else {
					EnviarErrorCritico(this->socket);
				}
		} else {
			EnviarErrorCritico(this->socket);
		}
	} else if (strcmp(tipo, "USERS") == 0) {

		EnviarListaUsuarios(this->socket, this->estado);

 	} else if (strcmp(tipo, "TEXT") == 0) {
		cJSON* destinatario = cJSON_GetObjectItemCaseSensitive(json, "username");
		cJSON* texto = cJSON_GetObjectItemCaseSensitive(json, "text");

		if (cJSON_IsString(destinatario) && cJSON_IsString(texto)) {
			if (ExisteUsuario(this->estado, destinatario->valuestring)) {
                int dest_socket = ObtenerSocketUsuario(this->estado, destinatario->valuestring);
				EnviarMensajePrivado(dest_socket, this->username, texto->valuestring);
			} else {
				EnviarRespuesta(this->socket, "TEXT", "NO_SUCH_USER", NULL);
			}
		} else {
			EnviarErrorCritico(this->socket);
		}

	} else if (strcmp(tipo, "PUBLIC_TEXT") == 0) {
		cJSON* texto = cJSON_GetObjectItemCaseSensitive(json, "text");
		if (cJSON_IsString(texto)) {
			TransmitirMensajePublico(this->estado, this->username, texto->valuestring);
		} else {
			EnviarErrorCritico(this->socket);
		}

	} else if (strcmp(tipo, "NEW_ROOM") == 0) {
		cJSON* roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
		if (cJSON_IsString(roomname) && strlen(roomname->valuestring) <= 16) {
			int resultado = CrearSala(this->estado, roomname->valuestring, this->username);
			if (resultado == 1) {
				EnviarRespuesta(this->socket, "NEW_ROOM", "SUCCESS", NULL);
			} else if (resultado == 2) {
				EnviarRespuesta(this->socket, "NEW_ROOM", "ROOM_ALREADY_EXISTS", NULL);
			}
		} else {
			EnviarErrorCritico(this->socket);
		}

	} else if (strcmp(tipo, "INVITE") == 0) {
		cJSON* roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
		cJSON* invitado = cJSON_GetObjectItemCaseSensitive(json, "username");

		if (cJSON_IsString(roomname) && cJSON_IsString(invitado)) {
			int resultado = Invitacion(this->estado, roomname->valuestring, invitado->valuestring, this->username);
			if (resultado == 1) {
                int dest_socket = ObtenerSocketUsuario(this->estado, invitado->valuestring);
				EnviarInvitacion(dest_socket, this->username, roomname->valuestring);
			} else if (resultado == 2) {
				EnviarRespuesta(this->socket, "INVITE", "NO_SUCH_ROOM", NULL);
			} else if (resultado == 4) {
                EnviarRespuesta(this->socket, "INVITE", "NOT_JOINED", NULL);
            }
		} else {
			EnviarErrorCritico(this->socket);
		}

	} else if (strcmp(tipo, "JOIN_ROOM") == 0) {
		cJSON* roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
		if (cJSON_IsString(roomname)) {
			int resultado = UnirseSala(this->estado, roomname->valuestring, this->username);
			if (resultado == 1) {
				EnviarRespuesta(this->socket, "JOIN_ROOM", "SUCCESS", NULL);
				AvisoUnionSala(this->estado, roomname->valuestring, this->username);

			} else if (resultado == 2) {
				EnviarRespuesta(this->socket, "JOIN_ROOM", "NO_SUCH_ROOM", NULL);
			} else if (resultado == 3) {
				EnviarRespuesta(this->socket, "JOIN_ROOM", "NOT_INVITED", NULL);
			}
		} else {
			EnviarErrorCritico(this->socket);
		}

	} else if (strcmp(tipo, "ROOM_USERS") == 0) {
		cJSON* roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
		if (cJSON_IsString(roomname)) {
			EnviarListaSala(this->socket, roomname->valuestring, this->estado);
		} else {
			EnviarErrorCritico(this->socket);
		}

	} else if (strcmp(tipo, "ROOM_TEXT") == 0) {
		cJSON* roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
		cJSON* texto = cJSON_GetObjectItemCaseSensitive(json, "text");
		if (cJSON_IsString(roomname) && cJSON_IsString(texto)) {
			EnviarTextoSala(this->socket, roomname->valuestring, this->username, texto->valuestring);
		} else{
			EnviarErrorCritico(this->socket);
		}
 	} else if (strcmp(tipo, "LEAVE_ROOM") == 0) {
		cJSON* roomname = cJSON_GetObjectItemCaseSensitive(json, "roomname");
		if (cJSON_IsString(roomname)) {
			DejarSala(this->estado, roomname->valuestring, this->username);
			EnviarRespuesta(this->socket, "LEAVE_ROOM", "SUCCESS", NULL);
			AvisoAbandonoSala(this->estado, roomname->valuestring, this->username);
		} else{
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
		EliminarUsuarioDeSalas(this->estado, this->username);
		EliminarUsuario(this->estado, this->username);
		TransmitirDesconexion(this->estado,this->username);
		this->identificado = false;
	}
	if (this->socket != -1) {
		close(this->socket);
		this->socket = -1;
	}
}

