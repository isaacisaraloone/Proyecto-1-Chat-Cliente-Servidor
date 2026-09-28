CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./Servidor/include -I./Servidor/lib -I./Servidor/lib/uthash
LDFLAGS = -pthread

.PHONY: all servidor abrir-servidor cliente abrir-cliente reporte abrir-reporte limpiar clienteServidor

all: servidor cliente reporte

servidor:
	$(CC) $(CFLAGS) -o Servidor/servidor Servidor/src/*.c Servidor/lib/cJSON.c $(LDFLAGS)
	@echo "Servidor compilado."

abrir-servidor:
	./Servidor/servidor 1234

cliente:
	cd Cliente && dotnet build
	@echo "Cliente compilado."

abrir-cliente:
	cd Cliente && dotnet run 127.0.0.1 1234

reporte:
	tectonic --outdir Reporte Reporte/reporte.tex
	@echo "Reporte generado."

abrir-reporte:
	xdg-open Reporte/reporte.pdf

limpiar:
	rm -f Servidor/servidor
	cd Cliente && dotnet clean
	rm -f Reporte/*.pdf
	@echo "Limpieza hecha."

clienteServidor:
	$(CC) $(CFLAGS) -o Servidor/servidor Servidor/src/*.c Servidor/lib/cJSON.c $(LDFLAGS)
	cd Cliente && dotnet build
	@echo "Servidor y cliente compilados."
