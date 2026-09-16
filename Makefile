CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./Servidor/include -I./Servidor/lib
LDFLAGS = -pthread

.PHONY: all servidor abrir-servidor cliente abrir-cliente reporte abrir-reporte limpiar test-cliente test-servidor test-all clienteServidor

all: servidor cliente reporte

servidor:
	$(CC) $(CFLAGS) -o Servidor/servidor Servidor/src/*.c Servidor/lib/cJSON.c $(LDFLAGS)
	@echo "Servidor compilado."

abrir-servidor:
	./Servidor/servidor

cliente:
	cd Cliente && dotnet build
	@echo "Cliente compilado."

abrir-cliente:
	cd Cliente && dotnet run

reporte:
	pdflatex -output-directory=Reporte Reporte/reporte.tex
	@echo "Reporte generado."

abrir-reporte:
	xdg-open Reporte/reporte.pdf

limpiar:
	rm -f Servidor/servidor
	cd Cliente && dotnet clean
	rm -f Reporte/*.pdf Reporte/*.aux Reporte/*.log Reporte/*.out Reporte/*.toc Reporte/*.fls Reporte/*.fdb_latexmk
	@echo "Limpieza hecha."

test-cliente:
	cd Cliente && dotnet test

test-servidor:
	$(CC) $(CFLAGS) -o Servidor/test_servidor Servidor/test.c $(LDFLAGS)
	./Servidor/test_servidor
	@echo "Pruebas del servidor ejecutadas."

test-all: test-cliente test-servidor
	@echo "Todas las pruebas unitarias ejecutadas."

clienteServidor:
	$(CC) $(CFLAGS) -o Servidor/servidor Servidor/src/*.c Servidor/lib/cJSON.c $(LDFLAGS)
	cd Cliente && dotnet build
	@echo "Servidor y cliente compilados."
