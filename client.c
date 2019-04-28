#include "./funk.c"


	
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
	args->sen = input;
	args->rec = received;
	args->socket = client_socket;
	args->running = true;
	args->cut_off = false;
	args->secret = secret;

	//Sets up the entirety of send and receive
	set_up_send_receive(args);
	
	//Shut down socket output
	shutdown(client_socket, SHUT_WR);
	
	close(client_socket);
	//fclose(received_file);
	return 0;
}
