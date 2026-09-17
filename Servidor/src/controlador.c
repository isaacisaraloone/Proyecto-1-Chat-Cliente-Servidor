#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "../include/controlador.h"
#include "../include/estadosEnrutamiento.h"

#define MAX_MSG_SIZE 1048576
#define CHUNK_SIZE 4096

void* ControladorCliente_IniciarHilo(void* argumento) {
	struct ControladorCliente* cliente = (struct ControladorCliente*)argumento;
	ControladorCliente_IniciarLectura(cliente);
	ControladorCliente_ManejarDesconexion(cliente);
	free(cliente);
	return NULL;
}

void ControladorCliente_IniciarLectura(struct ControladorCliente* cliente) {
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