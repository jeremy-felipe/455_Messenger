/* Client code */
/* TODO : Modify to meet your need */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define SERVER_ADDRESS  "127.0.0.1"
#define FILENAME        "outputfile.html"
#define MSG_BUFFER_SIZE 2048


typedef struct 
{
	char *rec;
	int socket;
	int running;
	int cut_off;
} rec_struct;

void *receive_runnable(void *vargp) 
{ 
	
	rec_struct *real_rec_struct = vargp;
	while(true)
	{
		recv(real_rec_struct->socket, real_rec_struct->rec, MSG_BUFFER_SIZE, 0); 
		if(strstr(real_rec_struct->rec,"/exit"))
		{
			real_rec_struct->running = false;
			real_rec_struct->cut_off = true;
			break;
		}
		printf("<<<%s", real_rec_struct->rec);
		memset(real_rec_struct->rec,0, MSG_BUFFER_SIZE);
	}
    return NULL; 
} 


int dh(int p,int g,int s){
	return (int)(pow(g,s)) % p;
}

char* encryption (char mess[], int key){
	int i;
	printf("WHY ARE YOU DOING THIS?!");
	for(i=0; mess[i]!='\0'; i++){
		mess[i] = (mess[i]+key)%26;
	}
	return mess;
}

char* decryption (char mess[], int key){
	int i;

	for(i = 0; mess[i] != '\0'; i++){
		mess[i] = (mess[i]-key)%26;
	}
	return mess;

}
	
int main(int argc, char **argv)
{
        int client_socket, file_size, remain_data, dhKey;
        ssize_t len;
	time_t t; 
        struct sockaddr_in remote_addr;
        char buffer[BUFSIZ];
        FILE *received_file;

	if (argc < 2) {
		printf("Usage: prog2client <port number>\n");
		exit(0);
	}

        remain_data = 0;
        // Zeroing remote_addr struct
        memset(&remote_addr, 0, sizeof(remote_addr));

        // Construct remote_addr struct
        remote_addr.sin_family = AF_INET;
        inet_pton(AF_INET, SERVER_ADDRESS, &(remote_addr.sin_addr));
        remote_addr.sin_port = htons(atoi(argv[1]));

        //Create client socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1)
        {
                fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));

                exit(EXIT_FAILURE);
        }
	printf("Connecting to server\n");

        //Connect to the server
        if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
        {
                fprintf(stderr, "Error connecting --> %s\n", strerror(errno));

                exit(EXIT_FAILURE);
        }

	printf("Connected to server\n");
	
	//Diffie Hellman Authentication with random number
	int p = 19;
	int g = 13;
	int s = 5;
	
	int difhel = dh(p,g,s);
	sprintf(buffer, "%d", difhel);
	if(send(client_socket, buffer, BUFSIZ, 0)<0){
		fprintf(stderr, "Error sending --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(recv(client_socket, buffer, BUFSIZ, 0)<0){
		fprintf(stderr, "error recieving --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf ("%s\n",buffer);
	dhKey = atoi(buffer);
	int secret = dh(p, dhKey, s);

	char input[MSG_BUFFER_SIZE];
	char received[MSG_BUFFER_SIZE];

	memset(input, 0, strlen(input));
	memset(received, 0, strlen(input));
	rec_struct *args = malloc(sizeof *args);
	args->rec = received;
	args->socket = client_socket;
	args->running = true;
	args->cut_off = false;
	//Create thread for receiving messages, continues while loop if message is received
	pthread_t thread_id;
	pthread_create(&thread_id,NULL,receive_runnable,args);
	
	while(args->running)
	{	
		//Prompt for input
		fgets(input, MSG_BUFFER_SIZE, stdin);
		
		//Exit or send
		if (strstr(input, "/exit") != NULL) 
		{
			send(client_socket, input, MSG_BUFFER_SIZE, 0);
			break;
		}
		else 
		{
			encryption(input, dhKey);
			printf(">>>%s \n", input); 
			send(client_socket, input, MSG_BUFFER_SIZE, 0);
			
		}
	}
	if(args->cut_off)
	{
		printf("Other User Disconnected\n");
	}
	else
	{
		
	}
	pthread_cancel(thread_id);
	//Free up the struct
	free(args);
	//Shut down socket output
	shutdown(client_socket, SHUT_WR);
	
	close(client_socket);
	//fclose(received_file);
	return 0;
}
