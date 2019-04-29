#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <time.h>

#define SERVER_ADDRESS  "127.0.0.1"
#define MSG_BUFFER_SIZE 2048

typedef struct 
{
	pthread_t thread1;
	pthread_t thread2;
} threads;

typedef struct 
{
	char *rec;
	char *sen;
	int socket;
	int running;
	int cut_off;
	int secret;
} rec_struct;

void *receive_runnable(void *vargp);
int dh(int p,int g,int s);
char* decryption(char mess[], int key);
char* encryption(char mess[], int key);
void scrubber (char* input);
void send_it(char* input, int client_socket, int secret, rec_struct *args);
void *thread_killer(void *vargp);
void *send_runnable(void *vargp);
void set_up_send_receive(rec_struct *args);





void scrubber (char* input) 
{
	int i;
	for (i = 0; i < sizeof(input); i++)
	{
		if (input[i] < 32 || input[i] > 126)
		{
			int j;
			int k = i;
			for (j = i+1; j < sizeof(input); j++)
			{
				input[k] = input[j];
				k++;
			}
		}
	}
}

char* encryption(char mess[], int key){
	int i;
	for(i=0; mess[i]!='\0'; i++){
		mess[i] = (mess[i]+key)%256;
	}
	return mess;
}

char* decryption(char mess[], int key){
	int i;

	for(i = 0; mess[i] != '\0'; i++){
		if (mess[i] < key)
		{
			mess[i] = 256-(key-mess[i]);
		}
		else
		{
			mess[i] = mess[i]-key;
		}
	}
	return mess;
}

void *receive_runnable(void *vargp) 
{ 
	
	rec_struct *real_rec_struct = vargp;
	while(true)
	{
		recv(real_rec_struct->socket, real_rec_struct->rec, MSG_BUFFER_SIZE, 0); 
		
		decryption(real_rec_struct->rec,real_rec_struct->secret);
		if(strstr(real_rec_struct->rec,"/exit") != NULL)
		{
			real_rec_struct->running = false;
			real_rec_struct->cut_off = true;
			break;
		}
		printf("\n<<<%s\n", real_rec_struct->rec);
		memset(real_rec_struct->rec,0, MSG_BUFFER_SIZE);
	}
	return NULL; 
} 

void *send_runnable(void *vargp) 
{
	rec_struct *real_rec_struct = vargp;
	while(true)
	{	
		//Prompt for input
		fgets(real_rec_struct->sen, MSG_BUFFER_SIZE, stdin);
		//Exit or send
		if (strstr(real_rec_struct->sen, "/exit") != NULL) 
		{
			encryption(real_rec_struct->sen, real_rec_struct->secret);
			printf("Encrypted Text: %s", real_rec_struct->sen);
			send(real_rec_struct->socket, real_rec_struct->sen, MSG_BUFFER_SIZE, 0);
			break;
		}
		else if(real_rec_struct->cut_off)
		{
			printf("Other User Disconnected\n");
			break;
		}
		else 
		{
			printf(">>>%s \n", real_rec_struct->sen);
			scrubber(real_rec_struct->sen);
			encryption(real_rec_struct->sen, real_rec_struct->secret);
			send(real_rec_struct->socket, real_rec_struct->sen, MSG_BUFFER_SIZE, 0);
			memset(real_rec_struct->sen,0, MSG_BUFFER_SIZE);
		}
	}
}

int dh(int p,int g,int s){
	return (int)(pow(g,s)) % p;
}
/*
void send_it(char* input, int client_socket, int secret, rec_struct *args)
{
	while(true)
	{	
		//Prompt for input
		fgets(input, MSG_BUFFER_SIZE, stdin);
		
		//Exit or send
		if (strstr(input, "/exit") != NULL) 
		{
			encryption(input, secret);
			send(client_socket, input, MSG_BUFFER_SIZE, 0);
			break;
		}
		else if(args->cut_off)
		{
			printf("Other User Disconnected\n");
			break;
		}
		else 
		{
			printf(">>>%s \n", input);
			scrubber(input);
			encryption(input, secret);
			
			send(client_socket, input, MSG_BUFFER_SIZE, 0);
		}
	}
}*/

void *thread_killer(void *vargp) 
{ 
	
	threads *threads = vargp;
	pthread_join(threads->thread1,NULL);
	pthread_cancel(threads->thread2);
	printf("Other User Disconnected\n");
	
	return NULL; 
} 

void set_up_send_receive(rec_struct *args)
{
	//Create thread for receiving messages, continues while loop if message is received
	pthread_t rec_thread;
	pthread_create(&rec_thread,NULL,receive_runnable,args);
	
	//Create thread for sending messages, continues while loop if message is sent
	pthread_t send_thread;
	pthread_create(&send_thread,NULL,send_runnable,args);
	
	//Create thread for waiting for one thread to finish
	threads *thread_args = malloc(sizeof *thread_args);
	pthread_t thread_murderer;
	thread_args->thread1 = rec_thread;
	thread_args->thread2 = send_thread;
	pthread_create(&thread_murderer,NULL,thread_killer,thread_args);

	//Wait for 
	pthread_join(send_thread,NULL);
	
	pthread_cancel(thread_murderer);
	pthread_cancel(rec_thread);
	
	//Free up the struct
	free(args);
	free(thread_args);
}
