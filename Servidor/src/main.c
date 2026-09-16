#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/salasUsuario.h"

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
}
