#include "./funk2.c"


	
int main(int argc, char **argv)
{
	int client_socket, file_size, remain_data, dhKey;
	ssize_t len;
	time_t t; 
	struct sockaddr_in remote_addr;
	
	
	
	int first_prime, second_prime, nonce, phi_primes, i, flag;
	long int server_public_key[500], e[500], d[500], rectemp[500], sentemp[500], j, m[500], enrec[500],ensen[500];
	

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
	
	//RSA
	//first_prime = 100043;
	first_prime = 100109;
	second_prime = 23;
	flag = prime(second_prime, &j);
	nonce = first_prime * second_prime;
	phi_primes = (first_prime-1) * (second_prime-1);
	memset(e,0,500);
	encryption_key(first_prime,second_prime,phi_primes,&flag,e,d,&j,&i);
	
	
	///////////////////////////////////////////////////////////////////
	
	if(send(client_socket, d, sizeof(d), 0)<0){
		fprintf(stderr, "Error sending --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	printf("Sending Public Key\n");
	
	if(recv(client_socket, server_public_key, sizeof(server_public_key), 0)<0){
		fprintf(stderr, "error recieving --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	printf("Server Public Key Received\n");
	printf("Secure Connection Established\n");
	


	char input[MSG_BUFFER_SIZE];
	char received[MSG_BUFFER_SIZE];

	memset(input, 0, strlen(input));
	memset(received, 0, strlen(input));
	rec_struct *args = malloc(sizeof *args);
	args->sen = input;
	args->rec = received;
	args->socket = client_socket;
	args->e = e;
	args->rectemp = rectemp;
	args->sentemp = sentemp;
	args->enrec = enrec;
	args->ensen = ensen;
	args->nonce = nonce;
	args->j = j;
	args->m = m;
	args->d = d;
	args->running = true;
	args->cut_off = false;
	
	memset(args->rec,0, MSG_BUFFER_SIZE);
	memset(args->sen,0, MSG_BUFFER_SIZE);
	
	//Sets up the entirety of send and receive
	set_up_send_receive(args);
	
	//Shut down socket output
	shutdown(client_socket, SHUT_WR);
	
	close(client_socket);
	//fclose(received_file);
	return 0;
}
