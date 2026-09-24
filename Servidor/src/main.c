#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>

#include "../include/estado.h"
#include "../include/controlador.h"

volatile bool servidorActivo = true;
int servidorEnchufe;

void Comprobador(int senal) {
	if (senal == SIGINT) {
		printf("\nLimpiando recursos...\n");
		servidorActivo = false;

		shutdown(servidorEnchufe, SHUT_RDWR);
		close(servidorEnchufe);
	}
}

struct EstadoGlobal estadoGlobal;

int main(int argc, char **argv) {
	
	if (argc != 2) {
		fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int puerto = atoi(argv[1]);
	if (puerto < 1025 || puerto > 65535) {
		fprintf(stderr, "Puerto inválido. Debe estar entre 1025 y 65535.\n");
		exit(EXIT_FAILURE);
	}

	Inicializar(&estadoGlobal);

	/*
	CODIGO ROBADO DE: https://github.com/parthnan/FullTCP-Chat-in-C/
	*/

	struct sockaddr_in svr, clt;
	int reuse = 1;

	if ((servidorEnchufe = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Fallo en socket()");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(servidorEnchufe, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror("Fallo en setsockopt()");
		exit(EXIT_FAILURE);
	}

	bzero(&svr, sizeof(svr));
	svr.sin_family = AF_INET;
	svr.sin_addr.s_addr = htonl(INADDR_ANY);
	svr.sin_port = htons(puerto); //a direfencia del codigo original, el puerto lo asigna el usuario.

	if (bind(servidorEnchufe, (struct sockaddr*)&svr, sizeof(svr)) < 0) {
		perror("Fallo en bind()");
		exit(EXIT_FAILURE);
	}

	if (listen(servidorEnchufe, 10) < 0) {
		perror("Fallo en listen()");
		exit(EXIT_FAILURE);
	}
	/*
	FIN DEL CODIGO ROBADO.
	*/

	signal(SIGINT, Comprobador);

	printf("Servidor escuchando en el puerto %d...\n", puerto);

	while (servidorActivo) {
		socklen_t clen = sizeof(clt);
		int clienteEnchufe = accept(servidorEnchufe, (struct sockaddr *)&clt, &clen);
		if (clienteEnchufe < 0) {
			if (!servidorActivo) break;
			perror("Error aceptando conexion");
			continue;
		}

		printf("Cliente aceptado\n");

		struct Cliente* controlador = (struct Cliente*)malloc(sizeof(struct Cliente));
		if (controlador == NULL) {
			perror("Error asignando memoria para cliente.\n");
			close(clienteEnchufe);
			continue;
		}

		controlador->socket = clienteEnchufe;
		controlador->estado = &estadoGlobal;
		controlador->identificado = false;
		memset(controlador->username, 0, sizeof(controlador->username));

		pthread_t hiloCliente;
		if (pthread_create(&hiloCliente, NULL, IniciarHilo, (void*)controlador) != 0) {
			perror("Error al crear el hilo");
			close(clienteEnchufe);
			free(controlador);
		} else {
			pthread_detach(hiloCliente);
		}
	}

	printf("Cerrando el servidor");	
	close(servidorEnchufe);
	Destruir(&estadoGlobal);
	return 0;
}
