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



#define SERVER_ADDRESS  "127.0.0.1"
#define FILE_TO_SEND    "input.html"
#define MSG_BUFFER_SIZE 2048



typedef struct 
{
	char *rec;
	int socket;
	int running;
	int cut_off;
	int secret;
} rec_struct;

char* encryption (char mess[], int key){
	int i;
	for(i=0; mess[i]!='\0'; i++){
		mess[i] = (mess[i]+key)%256;
	}
	return mess;
}

char* decryption (char mess[], int key){
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
	while(real_rec_struct->running && !real_rec_struct->cut_off)
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

int dh(int p,int g,int s){
	return (int)(pow(g,s)) % p;
}

int main(int argc, char **argv)
{
        int server_socket, peer_socket, fd, sent_bytes, remain_data, dhKey;
        socklen_t socket_length;
        off_t offset;
        ssize_t len;
	time_t t; 
        struct sockaddr_in server_addr, peer_addr;
        struct stat file_stat;
        char file_size[BUFSIZ];
	char *conn_check;

	if (argc < 2) {
		printf("Usage: prog2server <port number>\n");
		exit(0);
	}
	
	conn_check = "0";
        sent_bytes = 0;

        // Create server socket 
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1)
        {
                fprintf(stderr, "Error creating socket --> %s", strerror(errno));

                exit(EXIT_FAILURE);
        }

        //Zeroing server_addr 
        memset(&server_addr, 0, sizeof(server_addr));
        //Construct server_addr 
        server_addr.sin_family = AF_INET;
        inet_pton(AF_INET, SERVER_ADDRESS, &(server_addr.sin_addr));
        server_addr.sin_port = htons(atoi(argv[1]));

         //Bind socket
        if ((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1)
        {
                fprintf(stderr, "Error binding --> %s", strerror(errno));

                exit(EXIT_FAILURE);
        }

        //Listening to incoming connections 
        if ((listen(server_socket, 5)) == -1)
        {
                fprintf(stderr, "Error listening for clients --> %s", strerror(errno));

                exit(EXIT_FAILURE);
        }
	printf("Listening for clients\n");



        socket_length = sizeof(struct sockaddr_in);
        //Accepting incoming peers
        peer_socket = accept(server_socket, (struct sockaddr *)&peer_addr, &socket_length);
        if (peer_socket == -1)
        {
                fprintf(stderr, "Error accepting --> %s", strerror(errno));

                exit(EXIT_FAILURE);
        }

	printf("Client is connecting\n");
        fprintf(stdout, "Accept peer --> %s\n", inet_ntoa(peer_addr.sin_addr));
	
	//Diffie Hellman Authentication with random number
	int p = 19;
	int g = 13;
	int s = 2; 

	int difhel = dh(p,g,s);

	if(recv(peer_socket, file_size, BUFSIZ, 0)<0){
		fprintf(stderr, "error receiving --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	printf("%s\n", file_size);
	dhKey = atoi(file_size);
	sprintf(file_size, "%d", difhel);

	if(send(peer_socket, file_size, BUFSIZ, 0)<0){
		fprintf(stderr, "error sending --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	int secret = dh(p, dhKey, s);

	printf("Connected to client\n");
	char input[MSG_BUFFER_SIZE];
	char received[MSG_BUFFER_SIZE];
	memset(input, 0, MSG_BUFFER_SIZE);
	memset(received, 0, MSG_BUFFER_SIZE);
	rec_struct *args = malloc(sizeof *args);
	args->rec = received;
	args->socket = peer_socket;
	args->running = true;
	args->cut_off = false;
	args->secret = secret;

	//Create thread for receiving messages, continues while loop if message is received
	pthread_t thread_id;
	pthread_create(&thread_id,NULL,receive_runnable,args);

	while(args->running)
	{	
		fgets(input, MSG_BUFFER_SIZE, stdin);
		
		
		//Some way to exit
		if (strstr(input, "/exit") != NULL) 
		{
			encryption(input, secret);
			send(peer_socket, input, MSG_BUFFER_SIZE, 0);
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
			encryption(input,secret);
			
			send(peer_socket, input, MSG_BUFFER_SIZE, 0);
		}
	}
	pthread_cancel(thread_id);
	//Free up the struct
	free(args);
	
	

	while(send(peer_socket, conn_check, sizeof(conn_check), 0) > -1) {
		sleep(1);
	}
	printf("Initiating disconnect from server side\n");
        close(peer_socket);
        close(server_socket);

        return 0;
}

