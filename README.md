### Modelado y Programación

==========================================

# Proyecto 1 Modelado y Programación Chat Cliente-Servidor.

Proyecto desarrollado para la materia de **Modelado y Programación**.
Consiste de un sistema de mensajeria instantanea concurrente, con la arquitectura **Cliente-Servidor** mediante sockets **TCP/IP** y usando un protocolo de comunicación estructurado en formato **JSON**.

==========================================

Para compilar el servidor usa el comando:
- make servidor

Para ejecutar el servidor usa el comando:
- make abrir-servidor

Para compilar el cliente usa el comando:
- make cliente

Para ejecutar el cliente usa el comando:
- make abrir-cliente

Para compilar y generar el reporte usa el comando: *Para el uso de este comando es necesario tener "tectonic" en el dispositivo.*
- make reporte

Para abrir el reporte usa el comando:
- make abrir-reporte

NOTA:
El servidor tambien puede ejecutarse mediante otro puerto con: 
- ./Servidor/servidor <puerto>

El cliente puede conectarse a otro IP y puerto con: 
- cd Cliente && dotnet run <ip_servidor> <puerto>