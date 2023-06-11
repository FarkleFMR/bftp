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

int establecerConeccion(const char* serverIP){
    int mySocket;
    struct sockaddr_in serverDir;
    
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket == -1){
        perror("Error creando el socket");
    }

    serverDir.sin_family = AF_INET;
    serverDir.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port	= htons(SERVER_PORT);

    if (connect(mySocket,(struct sockaddr*)&serverDir,sizeof(serverDir)) == -1){
        perror("Error al conectar al servidor");
        return -1;
    }
    
    printf("Coneccion exitosa con el servidor %s\n", serverIP);

    return mySocket;
}

void* manejoConeccion(void* args){
   int socket = *((int*)args);
   free(args);

   char command[MAX_COMMAND_LENGTH];

    while(1){
        printf("\n Ingrese un comando remoto: ");
        fgets(command, MAX_COMMAND_LENGTH,stdin);

        command[strcspn(cmmand, "\n")] = '\0';

        if (strcmp(command,"close") == 0){
	    break;
        }

	//procesarComando(socket,command);
    }

    close(socket);
    return NULL;
}


int main(){
   char command[MAX_COMMAND_LENGTH];
   
   while(1){
       printf("\nIngrese un comando: 	");
       fgets(command,MAX_COMMAND_LENGTH,stdin);
       
       command[strcspn(command,"\n")] = '\0';

       if (strncmp(command,"quit") == 0){
            break;
       } else if (strncmp(command,"open",4) == 0){
            char serverIP[16];
            sscanf(command,"open %15s",serverIP);
            int socket = establecerConeccion(serverIP);
            if (socket != -1){
                pthread_t hilo;
                int* socketPtr = malloc(sizeof(int));
                socketPtr = socket;
                if (pthread_create(&hilo,NULL,manejoConeccion,(void*)socketPtr) != 0){
                     perror("Error creando el hilo")
                     break;
                }
            }
       } else {
               printf("Comando no recoocido. Los comandos permitidos y validos son: \n");
       }
   }
   printf("Saliendo del cliente FTP.\n");

   return 0;
}
