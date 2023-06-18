#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
#include <pthread.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_RESPONSE_LENGTH 1024
#define SERVER_PORT 8889

typedef struct {
    int socket;
    struct sockaddr_in address;
} PeerInfo;

// Función para procesar el comando "cd" en el servidor
void procesarComandoCD(int clientSocket, const char* directory) {
    if (chdir(directory) == 0) {
        send(clientSocket, "Directorio cambiado correctamente", strlen("Directorio cambiado correctamente"), 0);
    } else {
        send(clientSocket, "Error al cambiar el directorio", strlen("Error al cambiar el directorio"), 0);
    }
}

// Función para procesar el comando "pwd" en el servidor
void procesarComandoPWD(int clientSocket) {
    char currentDirectory[PATH_MAX];
    if (getcwd(currentDirectory, sizeof(currentDirectory)) != NULL) {
        send(clientSocket, currentDirectory, strlen(currentDirectory), 0);
    } else {
        send(clientSocket, "Error al obtener el directorio actual", strlen("Error al obtener el directorio actual"), 0);
    }
}

// Función para procesar los comandos recibidos del cliente
void procesarComando(int clientSocket, const char* command) {
    if (strncmp(command, "cd", 2) == 0 && strlen(command) > 3 && command[2] == ' ') {
        char directory[MAX_COMMAND_LENGTH];
        strcpy(directory, command + 3);
        procesarComandoCD(clientSocket, directory);
    } else if (strcmp(command, "pwd") == 0) {
        procesarComandoPWD(clientSocket);
    } else {
        send(clientSocket, "Comando no reconocido", strlen("Comando no reconocido"), 0);
    }
}

// Función para manejar la conexión con un cliente en un hilo separado
void* manejoConexion(void* args) {
    PeerInfo* peerInfo = (PeerInfo*)args;
    int clientSocket = peerInfo->socket;

    char command[MAX_COMMAND_LENGTH];

    while (1) {
        ssize_t bytesRead = recv(clientSocket, command, MAX_COMMAND_LENGTH - 1, 0);
        if (bytesRead == -1) {
            perror("Error al recibir el comando del cliente");
            break;
        } else if (bytesRead == 0) {
            // El cliente ha cerrado la conexión
            break;
        }

        command[bytesRead] = '\0';

        if (strcmp(command, "close") == 0) {
            break;
        } else {
            procesarComando(clientSocket, command);
        }
    }

    close(clientSocket);
    free(peerInfo);
    return NULL;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;

    // Crear un socket para el servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creando el socket del servidor");
        return -1;
    }

    // Configurar la dirección del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Enlazar el socket del servidor a la dirección
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al enlazar el socket del servidor");
        return -1;
    }

    // Escuchar las conexiones entrantes
    if (listen(serverSocket, 5) == -1) {
        perror("Error al escuchar las conexiones entrantes");
        return -1;
    }

    printf("Servidor FTP peer-to-peer esperando conexiones en el puerto %d...\n", SERVER_PORT);

    while (1) {
        // Aceptar una nueva conexión entrante
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Error al aceptar la conexión entrante");
            continue;
        }

        PeerInfo* peerInfo = malloc(sizeof(PeerInfo));
        peerInfo->socket = clientSocket;
        peerInfo->address = clientAddr;

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("Cliente conectado desde %s\n", clientIP);

        // Crear un hilo para manejar la conexión con el cliente
        pthread_t thread;
        if (pthread_create(&thread, NULL, manejoConexion, (void*)peerInfo) != 0) {
            perror("Error creando el hilo para manejar la conexión");
            continue;
        }

        // Hilo creado, continuar esperando nuevas conexiones
        pthread_detach(thread);
    }

    close(serverSocket);

    return 0;
}
