#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include <stdbool.h>
#include "salasUsuario.h"

struct ControladorCliente {
	int socket;
	struct EstadoGlobal* estado;

	bool identificado;
	char username[MAX_USERNAME_LEN];
};

void* ControladorCliente_IniciarHilo(void* argumento);

void ControladorCliente_IniciarLectura(struct ControladorCliente* this);

void ControladorCliente_EnrutarMensaje(struct ControladorCliente* this, char* mensajeBruto);

void ControladorCliente_ManejarDesconexion(struct ControladorCliente* this);

#endif // CONTROLADOR_H