#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> // Para el uso de funciones del sistema operativo (Linux)
#include <sys/types.h> // Definiciones de tipos para el sistema (Linux)
#include <sys/wait.h>  // Para usar waitpid (Linux)
#include <signal.h> // Para manejar señales

#define MAX_ARGUMENTOS 21 // Máximo número de argumentos permitidos para un comando
#define MAX_CARACTERES_LINEA 1024 // Máximo longitud de la línea de entrada

// Función manejadora de señal
void manejadorSenal(int i) {
    printf("\n¡Adiós!\n"); // Imprime mensaje de despedida
    exit(i); // Termina el programa
}

// Función para dividir una línea de comandos en tokens
void procesarArgumentos(char* linea, char** argumentos) {
    int i = 0; // Inicializa el contador de argumentos
    char* token = strtok(linea, " "); // Divide la línea en tokens separados por espacios
    while (token != NULL) { // Mientras haya tokens
        if (token[0] == '-' && token[2] == '\'') { // Si el token es una opción con un argumento entre comillas simples
            char* temp = malloc(3 * sizeof(char)); // Reserva memoria para la opción
            temp[0] = '-'; // Establece el primer carácter como '-'
            temp[1] = token[1]; // Establece el segundo carácter como el carácter después de '-'
            temp[2] = '\0'; // Termina la cadena con un carácter nulo
            argumentos[i++] = temp; // Añade la opción a los argumentos
            argumentos[i++] = " "; // Añade un espacio a los argumentos
            token = strtok(NULL, " "); // Salta el próximo token
        } else if ((token[0] == '"' && token[strlen(token) - 1] == '"') || (token[0] == '\'' && token[strlen(token) - 1] == '\'')) {
            // Si el token empieza y termina con comillas dobles o simples
            token[strlen(token) - 1] = '\0'; // Elimina las comillas
            argumentos[i++] = token + 1; // Añade el contenido entre comillas a los argumentos
        } else if (token[0] == '"' || token[0] == '\'') {
            // Si el token empieza con comillas dobles o simples pero no termina con ellas
            char comilla = token[0]; // Guarda el tipo de comillas
            char* final = strchr(token + 1, comilla); // Busca la comilla de cierre en el mismo token
            if (final != NULL) {
                // Si encontró la comilla de cierre en el mismo token
                *final = '\0'; // Elimina las comillas
                argumentos[i++] = token + 1; // Añade el contenido entre comillas a los argumentos
            } else {
                // Si no encontró la comilla de cierre en el mismo token
                char* argumentoLargo = malloc(MAX_CARACTERES_LINEA * sizeof(char)); // Reserva memoria para el argumento largo
                strcpy(argumentoLargo, token + 1); // Copia el contenido después de la comilla de apertura al argumento largo
                while ((token = strtok(NULL, " ")) != NULL && token[strlen(token) - 1] != comilla) {
                    // Mientras haya tokens y el token no termine con la comilla de cierre
                    strcat(argumentoLargo, " "); // Añade un espacio al argumento largo
                    strcat(argumentoLargo, token); // Añade el token al argumento largo
                }
                if (token != NULL) {
                    // Si encontró la comilla de cierre
                    token[strlen(token) - 1] = '\0'; // Elimina la comilla de cierre
                    strcat(argumentoLargo, " "); // Añade un espacio al argumento largo
                    strcat(argumentoLargo, token); // Añade la última parte del argumento al argumento largo
                }
                argumentos[i++] = argumentoLargo; // Añade el argumento largo a los argumentos
            }
        } else {
            argumentos[i++] = token; // Añade el token a los argumentos
        }
        token = strtok(NULL, " "); // Obtiene el próximo token
    }
    argumentos[i] = NULL; // Termina la lista de argumentos con un puntero nulo
}

char* eliminarEspacios(char* cadena) {
    while(isspace((unsigned char)*cadena)) cadena++; // Mientras el primer carácter sea un espacio, avanza en la cadena
    if(*cadena == 0) return cadena; // Si la cadena está vacía (solo tenía espacios), devuelve la cadena

    char* final = cadena + strlen(cadena) - 1; // Obtiene un puntero al último carácter de la cadena
    while(final > cadena && isspace((unsigned char)*final)) final--; // Mientras el último carácter sea un espacio, retrocede en la cadena
    final[1] = '\0'; // Coloca un carácter nulo después del último carácter no espacial para terminar la cadena

    return cadena; // Devuelve la cadena sin espacios al principio ni al final
}

// Función para ejecutar un solo comando
int ejecutarComando(char** argumentos) {
    int estado; // Variable para guardar el estado del proceso
if (strcmp(argumentos[0], "cd") == 0) {
    // Manejar comando cd
    if (argumentos[1] == NULL) {
        fprintf(stderr, "Se necesita un argumento para usar \"cd\"\n");
    } else { // Si hay un segundo argumento
        char ruta[1024]; // Almacenar la ruta
        int i = 1; // Contador para iterar a través de los argumentos
        strcpy(ruta, argumentos[i]); // Copia el primer argumento a la ruta
        i++;
        while(argumentos[i] != NULL) { // Mientras haya más argumentos
            strcat(ruta, " "); // Añade un espacio a la ruta
            strcat(ruta, argumentos[i]); // Añade el siguiente argumento a la ruta
            i++;
        }
        if (chdir(ruta) != 0) { // Intenta cambiar al directorio especificado por la ruta
            perror("Error cambiando de directorio");
        }
    }
    estado = 0; // Establece el estado a 0 para indicar que el comando se ejecutó correctamente
    } else {
        pid_t pid = fork(); // Crea un proceso hijo
        if (pid == 0) {
            // Proceso hijo
            if (execvp(argumentos[0], argumentos) == -1) { // Ejecuta el comando
                perror("Error ejecutando comando"); // Muestra un mensaje de error si execvp falla
                exit(EXIT_FAILURE); // Termina el proceso hijo con error
            }
        } else if (pid < 0) {
            // Error en fork()
            perror("Error creando proceso hijo"); // Muestra un mensaje de error si fork falla
            exit(EXIT_FAILURE); // Termina el programa
        } else {
            // Proceso padre
            if (waitpid(pid, &estado, 0) > 0) { // Espera a que el proceso hijo termine
                if (WIFEXITED(estado) && !WEXITSTATUS(estado)) {
                    return 0;  // El comando se ejecutó con éxito
                } else if (WIFEXITED(estado) && WEXITSTATUS(estado)) {
                    if (WEXITSTATUS(estado) == 127) {
                        // No se encontró el comando
                        return 0;
                    } else {
                        return 1;  // El comando falló
                    }
                }
            }
        }
    }  
    return 0; // Retorna 0 por defecto
}

void ejecutarComandoConPipe(char** argumentos1, char** argumentos2) {
    int pipefd[2]; // Declara un array para los descriptores de archivo de la tubería
    if (pipe(pipefd) == -1) { // Crea la tubería
        perror("Error creando la tubería"); // Si hay un error, imprime un mensaje y termina el programa
        exit(EXIT_FAILURE); // Termina el programa
    }

    pid_t pid1 = fork(); // Crea el primer proceso hijo
    if (pid1 == 0) { // Si estamos en el proceso hijo 1
        close(pipefd[0]); // Cierra el extremo de lectura de la tubería, no lo necesita
        dup2(pipefd[1], STDOUT_FILENO); // Redirige la salida estándar al extremo de escritura de la tubería
        close(pipefd[1]); // Cierra el extremo de escritura de la tubería, ya no lo necesita

        if (execvp(argumentos1[0], argumentos1) == -1) { // Ejecuta el primer comando
            perror("Error ejecutando el primer comando"); // Si hay un error, imprime un mensaje y termina el programa
            exit(EXIT_FAILURE);
        }
    } else if (pid1 > 0) { // Si estamos en el proceso padre
        pid_t pid2 = fork(); // Crea el segundo proceso hijo
        if (pid2 == 0) { // Si estamos en el proceso hijo 2
            close(pipefd[1]); // Cierra el extremo de escritura de la tubería, no lo necesita
            dup2(pipefd[0], STDIN_FILENO); // Redirige la entrada estándar al extremo de lectura de la tubería
            close(pipefd[0]); // Cierra el extremo de lectura de la tubería, ya no lo necesita

            if (execvp(argumentos2[0], argumentos2) == -1) { // Ejecuta el segundo comando
                perror("Error ejecutando el segundo comando"); // Si hay un error, imprime un mensaje y termina el programa
                exit(EXIT_FAILURE);
            }
        } else if (pid2 > 0) { // Si estamos en el proceso padre
            close(pipefd[0]); // Cierra el extremo de lectura de la tubería, ya no lo necesita
            close(pipefd[1]); // Cierra el extremo de escritura de la tubería, ya no lo necesita
            
            wait(NULL); // Espera a que el proceso hijo 1 termine
            wait(NULL); // Espera a que el proceso hijo 2 termine
        } else {
            // Error en fork()
            perror("Error creando proceso hijo 2"); // Si hay un error, imprime un mensaje y termina el programa
            exit(EXIT_FAILURE); 
        }
    } else {
        // Error en fork()
        perror("Error creando proceso hijo 1"); // Si hay un error, imprime un mensaje y termina el programa
        exit(EXIT_FAILURE); 
    }
}

int main() {
    signal(SIGINT, manejadorSenal); // Configura el manejador de señal para SIGINT (Ctrl+C)
    char linea[MAX_CARACTERES_LINEA]; // Declara un array para almacenar la línea de entrada
    char* argumentos[MAX_ARGUMENTOS]; // Declara un array para almacenar los argumentos del comando

    while (1) { // Bucle infinito para la shell
        printf("MiniShell> "); // Imprime el prompt de la shell
        fflush(stdout); // Limpia el buffer de salida

        if (fgets(linea, sizeof(linea), stdin) == NULL) { // Lee una línea de la entrada estándar
            printf("\n¡Adiós!\n"); // Si se detecta el final de la entrada (Ctrl+D), imprime un mensaje de despedida
            break; // y sale del bucle
        }

        linea[strcspn(linea, "\n")] = '\0'; // Elimina el salto de línea al final de la línea de entrada

        if (strlen(linea) == 0) { // Si la línea está vacía (solo un salto de línea)
            continue; // vuelve al principio del bucle
        }

        if (strcmp(linea, "salir") == 0) { // Si el comando es "salir"
            printf("¡Adiós!\n"); // imprime un mensaje de despedida
            break; // y sale del bucle
        }

        char* andPos = strstr(linea, "&&"); // Busca el operador &&
        char* orPos = strstr(linea, "||"); // Busca el operador ||
        char* pipePos = strchr(linea, '|'); // Busca el operador |

        if (orPos != NULL) { // Si se encontró el operador ||
            *orPos = '\0'; // divide la línea en dos comandos
            char* comando1 = linea; // el primer comando es el inicio de la línea
            char* comando2 = orPos + 2; // el segundo comando es después del operador ||

            char* argumentos1[MAX_ARGUMENTOS]; // Declara un array para los argumentos del primer comando
            char* argumentos2[MAX_ARGUMENTOS]; // Declara un array para los argumentos del segundo comando
            procesarArgumentos(comando1, argumentos1); // Procesa los argumentos del primer comando
            procesarArgumentos(comando2, argumentos2); // Procesa los argumentos del segundo comando

            int estado = ejecutarComando(argumentos1); // Ejecuta el primer comando

            if (!WIFEXITED(estado) || WEXITSTATUS(estado) != 0) { // Si el primer comando falló
                ejecutarComando(argumentos2); // ejecuta el segundo comando
            }
        } else if (andPos != NULL) { // Si se encontró el operador &&
            *andPos = '\0'; // divide la línea en dos comandos
            char* comando1 = linea; // el primer comando es el inicio de la línea
            char* comando2 = andPos + 2; // el segundo comando es después del operador &&

            char* argumentos1[MAX_ARGUMENTOS]; // Declara un array para los argumentos del primer comando
            char* argumentos2[MAX_ARGUMENTOS]; // Declara un array para los argumentos del segundo comando
            procesarArgumentos(comando1, argumentos1); // Procesa los argumentos del primer comando
            procesarArgumentos(comando2, argumentos2); // Procesa los argumentos del segundo comando

            int estado = ejecutarComando(argumentos1); // Ejecuta el primer comando

            if (WIFEXITED(estado) && WEXITSTATUS(estado) == 0) { // Si el primer comando tuvo éxito
                ejecutarComando(argumentos2); // ejecuta el segundo comando
            }
        } else if (pipePos != NULL) { // Si se encontró el operador |
            *pipePos = '\0'; // divide la línea en dos comandos
            char* comando1 = eliminarEspacios(linea); // el primer comando es el inicio de la línea, sin espacios al principio ni al final
            char* comando2 = eliminarEspacios(pipePos + 1); // el segundo comando es después del operador |, sin espacios al principio ni al final

            char* argumentos1[MAX_ARGUMENTOS]; // Declara un array para los argumentos del primer comando
            char* argumentos2[MAX_ARGUMENTOS]; // Declara un array para los argumentos del segundo comando
            procesarArgumentos(comando1, argumentos1); // Procesa los argumentos del primer comando
            procesarArgumentos(comando2, argumentos2); // Procesa los argumentos del segundo comando

            ejecutarComandoConPipe(argumentos1, argumentos2); // Ejecuta los dos comandos en un pipeline
        } else { // Si no se encontró ningún operador
            procesarArgumentos(linea, argumentos); // Procesa los argumentos del comando
            ejecutarComando(argumentos); // Ejecuta el comando
        }
    }
    return 0; // Retorna 0 al finalizar el programa
}