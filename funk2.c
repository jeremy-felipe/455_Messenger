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
#define MSG_BUFFER_SIZE 1000

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
	long int *e;
	long int *temp;
	long int *enrec;
	long int *ensen;
	int nonce;
	long int j;
	long int *m;
	long int *d;
	int running;
	int cut_off;
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

int prime(long int, long int*);
void encryption_key(int, int, int, int*, long int *, long int *, long int *,int*);
long int cd(long int, int);
void encrypt(long int *, char *, int, long int *, long int *, long int *, long int *);
void decrypt(long int, long int *, long int *, int, long int *, long int*);
void print_encrypt(long int *);





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
	int i;
	rec_struct *real_rec_struct = vargp;
	while(true)
	{
		recv(real_rec_struct->socket, real_rec_struct->ensen, MSG_BUFFER_SIZE, 0); 
		for(i = 0; real_rec_struct->ensen[i] != '\0'; i++)
		{
			real_rec_struct->m[i] = real_rec_struct->ensen[i];
		}
		decrypt(real_rec_struct->j, real_rec_struct->temp, real_rec_struct->ensen, real_rec_struct->nonce, real_rec_struct->m, real_rec_struct->d);
		for (i = 0; real_rec_struct->m[i] != '\0'; i++)
		{
			real_rec_struct->rec[i] = real_rec_struct->m[i];
		}
		real_rec_struct->rec[i] = real_rec_struct->m[i];
		if(strstr(real_rec_struct->rec[i],"/exit") != NULL)
		{
			real_rec_struct->running = false;
			real_rec_struct->cut_off = true;
			break;
		}
		//printf("\n<<<%s\n", real_rec_struct->m);
		memset(real_rec_struct->ensen,0, MSG_BUFFER_SIZE);
	}
	return NULL; 
} 

void *send_runnable(void *vargp) 
{
	
	int i;
	rec_struct *real_rec_struct = vargp;

	while(true)
	{	
		//Prompt for input
		//fgets(real_rec_struct->sen, MSG_BUFFER_SIZE, stdin);
		//scrubber(real_rec_struct->sen);
		scanf(" %[^\n]", real_rec_struct->sen);
		for(i = 0; real_rec_struct->sen[i] != '\0'; i++)
		{
			real_rec_struct->m[i] = real_rec_struct->sen[i];
		}
		//Exit or send
		if (strstr(real_rec_struct->sen, "/exit") != NULL) 
		{
			encrypt(real_rec_struct->e, real_rec_struct->sen, real_rec_struct->nonce, real_rec_struct->temp, real_rec_struct->ensen,real_rec_struct->m, &real_rec_struct->j);
			send(real_rec_struct->socket, real_rec_struct->ensen, MSG_BUFFER_SIZE, 0);
			break;
		}
		else if(real_rec_struct->cut_off)
		{
			printf("Other User Disconnected\n");
			break;
		}
		else 
		{			
			encrypt(real_rec_struct->e, real_rec_struct->sen, real_rec_struct->nonce, real_rec_struct->temp, real_rec_struct->ensen, real_rec_struct->m, &real_rec_struct->j);
			send(real_rec_struct->socket, real_rec_struct->ensen, MSG_BUFFER_SIZE, 0);
			//memset(real_rec_struct->sen,0, MSG_BUFFER_SIZE);
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

int prime(long int prime, long int *j)
{
	int i;
	*j = sqrt(prime);
	for(i = 2; i <= *j; i++)
	{
		if(prime % i == 0)
		{
			return 0;
		}
	}
	return 1;
}


//function to generate encryption key
void encryption_key(int first_prime, int second_prime, int phi_primes, int *flag, long int *e, long int *d, long int *j, int *i)
{
	int k;
	k = 0;
	for(*i = 2; *i < phi_primes; (*i)++)
	{

		if(phi_primes % *i == 0)
		{
			continue;
		}
		*flag = prime(*i,j);
		if(*flag == 1 && *i != first_prime && *i != second_prime)
		{
			e[k] = *i;
			*flag = cd(e[k],phi_primes);
			if(*flag > 0)
			{
				d[k] = *flag;
				k++;
			}
			if(k == 99)
			{
				break;
			}
		}
	}
}


long int cd(long int a, int phi_primes)
{
	long int k = 1;
	while(1)
	{
		k = k + phi_primes;
		if(k % a == 0)
		 return(k / a);
	}
}





//function to encrypt the message
void encrypt(long int *e, char *msg, int nonce, long int *temp, long int *en, long int *m, long int *j)
{
	long int pt, ct, key = e[0], k, len, i;
	i = 0;
	len = strlen(msg);
	while(i != len)
	{
		pt = m[i];
		pt = pt - 96;
		k = 1;
		for(*j = 0; *j < key; (*j)++)
		{
		 k = k * pt;
		 k = k % nonce;
		}
	 temp[i] = k;
	 ct = k + 96;
	 en[i] = ct;
	 i++;
	}
	en[i] = -1;
	printf("\n\nTHE ENCRYPTED MESSAGE IS\n");
	for(i = 0; en[i] != -1; i++)
		printf("%c", en[i]);
	printf("\n\n");
}





//function to decrypt the message
void decrypt(long int j, long int *temp, long int *en, int nonce, long int *m, long int *d)
{
	long int pt, ct, key = d[0], k, i;
	i = 0;
	while(en[i] != -1)
	{
		ct = temp[i];
		k = 1;
		for(j = 0; j < key; j++)
		{
			k = k * ct;
			k = k % nonce;
		}
	 pt = k + 96;
	 m[i] = pt;
	 i++;
	}
	m[i] = -1;
	printf("\n\nTHE DECRYPTED MESSAGE IS\n");
	for(i = 0; m[i] != -1; i++)
	 printf("%c", m[i]);
	printf("\n");
	printf("\n");
}

void print_encrypt(long int *en)
{
	int i;
	for (i = 0; en[i] != '\0'; i++)
	{
		printf("%c",en[i]);
	}
	printf("\n");
}