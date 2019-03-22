#include <stdio.h>
#include <stdbool.h>
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

int main(int argc, char **argv)
{
        int server_socket, peer_socket, fd, sent_bytes, remain_data;
        socklen_t socket_length;
        off_t offset;
        ssize_t len;
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

		/*
        sprintf(file_size, "%ld", file_stat.st_size);
        //Sending file size 
        len = send(peer_socket, file_size, sizeof(file_size), 0);
        if (len < 0)
        {
              fprintf(stderr, "Error sending opening --> %s", strerror(errno));

              exit(EXIT_FAILURE);
        }

        offset = 0;
        remain_data = file_stat.st_size;
	//Sending file data
	printf("Sending File\n");
        while (((sent_bytes = sendfile(peer_socket, fd, &offset, 1)) > 0) && (remain_data > 0))
        {

                remain_data -= sent_bytes;
	}
	printf("File sent\n");*/
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
	//Create thread for receiving messages, continues while loop if message is received
	pthread_t thread_id;
	pthread_create(&thread_id,NULL,receive_runnable,args);

	while(args->running)
	{	
		fgets(input, MSG_BUFFER_SIZE, stdin);
		
		
		//Some way to exit
		if (strstr(input, "/exit") != NULL) 
		{
			send(peer_socket, input, MSG_BUFFER_SIZE, 0);
			break;
		}
		else 
		{
			printf(">>>%s \n", input); 
			send(peer_socket, input, MSG_BUFFER_SIZE, 0);
		}
	}
	if(args->cut_off)
	{
		printf("Other User Disconnected\n");
	}
	else
	{
		printf("Waiting for client to disconnect\n");
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
