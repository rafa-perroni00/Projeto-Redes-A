//@rafa-perroni00   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048
#define RCVLENGTH 2172

typedef struct 
{
	char nome[32];
	char destinatario[32];
	char message[LENGTH];
}message;

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
	message mensagemtratada;
	strcpy(mensagemtratada.nome, name);
  char mensagem[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

  while(1) {
  	str_overwrite_stdout();
    fgets(mensagem, LENGTH, stdin);
    str_trim_lf(mensagem, LENGTH);

    if (strcmp(mensagem, "exit") == 0) {
			break;
    } else {
		if(strcmp(mensagem, "/r")==0){
			printf("Destinatario: ");
			fgets(mensagemtratada.destinatario, 32, stdin);
			printf("Para %s: ",mensagemtratada.nome);
			fgets(mensagemtratada.message,LENGTH,stdin);
			printf("De $s para %s: %s",name,mensagemtratada.nome,mensagemtratada.message);
		}
		else{
			bzero(mensagemtratada.destinatario, 32);
			strcpy(mensagemtratada.message, mensagem);
		}
      
      send(sockfd, (char *)&mensagemtratada, strlen((char *)&mensagemtratada), 0);
    }
		bzero(mensagem, LENGTH);
		bzero(mensagemtratada.message, LENGTH);
    	bzero(mensagemtratada.destinatario, 32);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
	char mensagem[LENGTH] = {};
	message mensagemtratada;
  while (1) {
		int receive = recv(sockfd, (char *)&mensagemtratada, RCVLENGTH, 0);
    if (receive > 0) {
      printf("De %s: %s", mensagemtratada.nome, mensagemtratada.message);
      str_overwrite_stdout();
    } else if (receive == 0) {
			break;
    } else {
			// -1
		}
		memset(mensagem, 0, sizeof(mensagem));
  }
}

int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Username: ");
  fgets(name, 32, stdin);
  str_trim_lf(name, strlen(name));


	if (strlen(name) > 32 || strlen(name) < 2){
		printf("Nome deve ter no maximo 30 caracteres e no minimo 2\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);


  // Connect to Server
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, name, 32, 0);

	printf("=== CHATROOM ===\n");

	pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nSaindo...\n");
			break;
    }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}
