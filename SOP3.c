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


#define BUFFER_SIZE 1024


void enviar_archivo(int socket, const char *nombre_archivo) {
   FILE *archivo;
   char buffer[BUFFER_SIZE];
   ssize_t bytes_leidos, bytes_enviados;


   archivo = fopen(nombre_archivo, "rb");
   if (archivo == NULL) {
       perror("Error al abrir el archivo");
       exit(1);
   }


   // Leer y enviar fragmentos del archivo hasta que se llegue al final
   while ((bytes_leidos = fread(buffer, 1, BUFFER_SIZE, archivo)) > 0) {
       bytes_enviados = send(socket, buffer, bytes_leidos, 0);
       if (bytes_enviados < 0) {
           perror("Error al enviar los datos a través del socket");
           exit(1);
       }
   }


   fclose(archivo);
}


void recibir_archivo(int socket, const char *nombre_archivo) {
   FILE *archivo;
   char buffer[BUFFER_SIZE];
   ssize_t bytes_recibidos, bytes_escritos;


   archivo = fopen(nombre_archivo, "wb");
   if (archivo == NULL) {
       perror("Error al crear el archivo");
       exit(1);
   }


   // Recibir y escribir fragmentos del archivo hasta que se llegue al final
   while ((bytes_recibidos = recv(socket, buffer, BUFFER_SIZE, 0)) > 0) {
       bytes_escritos = fwrite(buffer, 1, bytes_recibidos, archivo);
       if (bytes_escritos < bytes_recibidos) {
           perror("Error al escribir los datos en el archivo");
           exit(1);
       }
   }


   fclose(archivo);
}


int establecerConeccion(const char* serverIP){
   int mySocket;
   struct sockaddr_in serverDir;
  
   mySocket = socket(AF_INET, SOCK_STREAM, 0);
   if (mySocket == -1){
       perror("Error creando el socket");
   }


   serverDir.sin_family = AF_INET;
   serverDir.sin_addr.s_addr = inet_addr(serverIP);
   serverDir.sin_port  = htons(SERVER_PORT);


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


       command[strcspn(command, "\n")] = '\0';


       if (strcmp(command,"close") == 0){
       break;
       }else if(strcmp(command,"get") == 0){
           recibir_archivo(socket,"Archivo");
       }else if(strcmp(command,"put") == 0){
           enviar_archivo(socket,"Archivo");
       }


   //procesarComando(socket,command);
   }


   close(socket);
   return NULL;
}




int main(){
  char command[MAX_COMMAND_LENGTH];
 
  while(1){
      printf("\nIngrese un comando:    ");
      fgets(command,MAX_COMMAND_LENGTH,stdin);
     
      command[strcspn(command,"\n")] = '\0';


      if (strncmp(command,"quit",4) == 0){
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
                    perror("Error creando el hilo");
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








