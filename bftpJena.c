#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_RESPONSE_LENGTH 1024
#define SERVER_PORT 8889

// Función para establecer la conexión con el servidor
int establecerConexion(const char* serverIP) {
    int mySocket;
    struct sockaddr_in serverDir;

    // Crear un socket
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket == -1) {
        perror("Error creando el socket");
        return -1;
    }

    // Configurar la dirección del servidor
    serverDir.sin_family = AF_INET;
    serverDir.sin_addr.s_addr = inet_addr(serverIP);
    serverDir.sin_port = htons(SERVER_PORT);

    // Conectar al servidor
    if (connect(mySocket, (struct sockaddr*)&serverDir, sizeof(serverDir)) == -1) {
        perror("Error al conectar al servidor");
        return -1;
    }

    printf("Conexión exitosa con el servidor %s\n", serverIP);

    return mySocket;
}

// Función que maneja la conexión con el servidor en un hilo separado
void* manejoConexion(void* args) {
    int socket = *((int*)args);
    free(args);

    char command[MAX_COMMAND_LENGTH];

    while (1) {
        printf("\nIngrese un comando remoto: ");
        fgets(command, MAX_COMMAND_LENGTH, stdin);

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "close") == 0) {
            break;
        } else if (strncmp(command, "cd", 2) == 0 && strlen(command) > 3 && command[2] == ' ') {
            // Extraer la ruta del directorio del comando
            char directory[MAX_COMMAND_LENGTH];
            strcpy(directory, command + 3);

            // Enviar el comando de cambio de directorio al servidor
            if (send(socket, command, strlen(command), 0) == -1) {
                perror("Error al enviar el comando al servidor");
                break;
            }

            // Recibir la respuesta del servidor
            char response[MAX_RESPONSE_LENGTH];
            ssize_t bytesRead = recv(socket, response, MAX_RESPONSE_LENGTH - 1, 0);
            if (bytesRead == -1) {
                perror("Error al recibir la respuesta del servidor");
                break;
            }
            response[bytesRead] = '\0';

            // Imprimir la respuesta del servidor
            printf("Respuesta del servidor: %s\n", response);
        } else if (strcmp(command, "pwd") == 0) {
            // Enviar el comando "pwd" al servidor
            if (send(socket, command, strlen(command), 0) == -1) {
                perror("Error al enviar el comando al servidor");
                break;
            }

            // Recibir la respuesta del servidor
            char response[MAX_RESPONSE_LENGTH];
            ssize_t bytesRead = recv(socket, response, MAX_RESPONSE_LENGTH - 1, 0);
            if (bytesRead == -1) {
                perror("Error al recibir la respuesta del servidor");
                break;
            }
            response[bytesRead] = '\0';

            // Imprimir la respuesta del servidor
            printf("Directorio activo remoto: %s\n", response);
        } else {
            // Otros comandos remotos
            // procesarComando(socket,command);
        }
    }

    close(socket);
    return NULL;
}

int main() {
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        printf("\nIngrese un comando: ");
        fgets(command, MAX_COMMAND_LENGTH, stdin);

        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "quit", 4) == 0) {
            break;
        } else if (strncmp(command, "open", 4) == 0) {
            char serverIP[16];
            sscanf(command, "open %15s", serverIP);
            int socket = establecerConexion(serverIP);
            if (socket != -1) {
                pthread_t hilo;
                int* socketPtr = malloc(sizeof(int));
                *socketPtr = socket;
                if (pthread_create(&hilo, NULL, manejoConexion, (void*)socketPtr) != 0) {
                    perror("Error creando el hilo");
                    break;
                }
            }
        } else if (strncmp(command, "lcd", 3) == 0 && strlen(command) > 4 && command[3] == ' ') {
            // Extraer la ruta del directorio local del comando
            strcpy(localDirectory, command + 4);

            // Cambiar el directorio local usando chdir()
            if (chdir(localDirectory) == -1) {
                perror("Error al cambiar el directorio local");
            } else {
                printf("Directorio local cambiado a: %s\n", localDirectory);
            }
        } else {
            printf("Comando no reconocido. Los comandos permitidos y válidos son:\n");
        }
    }

    printf("Saliendo del cliente FTP.\n");

    return 0;
}
