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

// Función para procesar el comando "cd" en el servidor
void procesarComandoCD(int socket, const char* directory) {
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, MAX_COMMAND_LENGTH, "cd %s", directory);
    if (send(socket, command, strlen(command), 0) == -1) {
        perror("Error al enviar el comando al servidor");
        return;
    }

    // Recibir la respuesta del servidor
    char response[MAX_RESPONSE_LENGTH];
    ssize_t bytesRead = recv(socket, response, MAX_RESPONSE_LENGTH - 1, 0);
    if (bytesRead == -1) {
        perror("Error al recibir la respuesta del servidor");
        return;
    }
    response[bytesRead] = '\0';

    // Imprimir la respuesta del servidor
    printf("Respuesta del servidor: %s\n", response);
}

// Función para procesar el comando "pwd" en el servidor
void procesarComandoPWD(int socket) {
    char command[] = "pwd";
    if (send(socket, command, strlen(command), 0) == -1) {
        perror("Error al enviar el comando al servidor");
        return;
    }

    // Recibir la respuesta del servidor
    char response[MAX_RESPONSE_LENGTH];
    ssize_t bytesRead = recv(socket, response, MAX_RESPONSE_LENGTH - 1, 0);
    if (bytesRead == -1) {
        perror("Error al recibir la respuesta del servidor");
        return;
    }
    response[bytesRead] = '\0';

    // Imprimir la respuesta del servidor
    printf("Directorio activo remoto: %s\n", response);
}

// Función para establecer la conexión con otro peer
int establecerConexion(const char* peerIP) {
    int socketDescriptor;
    struct sockaddr_in serverAddress;

    // Crear un socket
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor == -1) {
        perror("Error creando el socket");
        return -1;
    }

    // Configurar la dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(peerIP);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Conectar al servidor
    if (connect(socketDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error al conectar al servidor");
        return -1;
    }

    printf("Conexión exitosa con el peer %s\n", peerIP);

    return socketDescriptor;
}

// Función que maneja la conexión con otro peer en un hilo separado
void* manejoConexion(void* args) {
    PeerInfo* peerInfo = (PeerInfo*)args;
    int socketDescriptor = peerInfo->socket;
    struct sockaddr_in peerAddress = peerInfo->address;
    free(peerInfo);

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

            // Procesar el comando "cd" en el servidor
            procesarComandoCD(socketDescriptor, directory);
        } else if (strcmp(command, "pwd") == 0) {
            // Procesar el comando "pwd" en el servidor
            procesarComandoPWD(socketDescriptor);
        } else {
            printf("Comando no reconocido.\n");
        }
    }

    close(socketDescriptor);
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
            char peerIP[16];
            sscanf(command, "open %15s", peerIP);
            int socketDescriptor = establecerConexion(peerIP);
            if (socketDescriptor != -1) {
                pthread_t thread;
                PeerInfo* peerInfo = malloc(sizeof(PeerInfo));
                peerInfo->socket = socketDescriptor;
                peerInfo->address.sin_family = AF_INET;
                peerInfo->address.sin_addr.s_addr = inet_addr(peerIP);
                peerInfo->address.sin_port = htons(SERVER_PORT);
                if (pthread_create(&thread, NULL, manejoConexion, (void*)peerInfo) != 0) {
                    perror("Error creando el hilo para manejar la conexión");
                    continue;
                }
                pthread_detach(thread);
            }
        } else {
            printf("Comando no reconocido.\n");
        }
    }

    return 0;
}
