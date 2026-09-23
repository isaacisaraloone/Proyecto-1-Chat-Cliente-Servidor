#ifndef RED_H
#define RED_H

#include "estado.h"

void EnviarRespuesta(int socket, const char* operacion, const char* resultado, const char* extra);

void EnviarErrorCritico(int socket);

void EnviarNuevoUsuario(int socket, const char* usuario);
void EnviarNuevoEstado(int socket, const char* usuario, const char* estado);
void EnviarListaUsuarios(int socket, struct EstadoGlobal* estado);
void EnviarDesconexion(int socket, const char* usuario);

void EnviarMensajePrivado(int socket, const char* usuarioRemitente, const char* texto);
void EnviarMensajePublico(int socket, const char* usuarioRemitente, const char* texto);

void EnviarInvitacion(int socket, const char* usuarioAnfitrion, const char* roomname);
void EnviarUnionSala(int socket, const char* roomname, const char* usuario);
void EnviarListaSala(int socket, const char* roomname, struct EstadoGlobal* estado);
void EnviarTextoSala(int socket, const char* roomname, const char* usuarioRemitente, const char* texto);
void EnviarAbandono(int socket, const char* roomname, const char* usuario);

#endif // RED_H