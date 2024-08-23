# Nombre del ejecutable
EXECUTABLE = MiniShell

# Compilador
CC = gcc

# Flags del compilador
CFLAGS = -Wall -Wextra -pedantic -std=c11

# Archivos fuente
SRC = MiniShell.c

# Archivos objeto
OBJ = $(SRC:.c=.o)

# Regla por defecto para compilar todo el proyecto
all: $(EXECUTABLE)

# Regla para compilar el ejecutable
$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Regla para compilar archivos .o a partir de archivos .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar los archivos intermedios y el ejecutable
clean:
	rm -f $(OBJ) $(EXECUTABLE)

# Declarar las reglas que no son archivos
.PHONY: all clean
