.PHONY: all servidor cliente docs limpiar test-cliente test-servidor test-all

all: servidor cliente docs

servidor:
    gcc -Wall -o Servidor/servidor Servidor/main.c
    @echo "Servidor compilado."

cliente:
    cd Cliente && dotnet build
    @echo "Cliente compilado."

docs:
    pdflatex -output-directory=Reporte Reporte/reporte.tex
    @echo "Reporte generado."

limpiar:
    rm -f Servidor/servidor
    cd Cliente && dotnet clean
    rm -f Reporte/*.pdf Reporte/*.aux Reporte/*.log Reporte/*.out Reporte/*.toc
    @echo "Limpieza hecha."

test-cliente:
    cd Cliente && dotnet test

test-servidor:
    gcc -Wall -o Servidor/test_servidor Servidor/test.c
    ./Servidor/test_servidor
    @echo "Pruebas del servidor ejecutadas."

test-all: test-cliente test-servidor
    @echo "Todas las pruebas unitarias ejecutadas."


