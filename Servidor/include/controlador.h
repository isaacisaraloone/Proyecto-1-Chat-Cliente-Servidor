#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include <stdbool.h>
#include "estado.h"

struct Cliente {
	int socket;
	struct EstadoGlobal* estado;

	bool identificado;
	char username[MAX_USERNAME_LEN];
};

void* IniciarHilo(void* argumento);

void IniciarLectura(struct Cliente* this);

void EnrutarMensaje(struct Cliente* this, char* mensajeBruto);

void ManejarDesconexion(struct Cliente* this);

#endif // CONTROLADOR_H