#include "./funk2.c"

int main(int argc, char **argv)
{
	int server_socket, peer_socket, fd, sent_bytes, remain_data, dhKey;
	socklen_t socket_length;
	off_t offset;
	ssize_t len;
	time_t t; 
	struct sockaddr_in server_addr, peer_addr;
	
	char *conn_check;
	
	int first_prime, second_prime, nonce, phi_primes, i, flag;
	long int client_public_key[500], e[500], d[500], rectemp[500], sentemp[500], j, m[500], enrec[500],ensen[500];
	

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
	printf("Connected to client\n");
	
	//RSA
	//first_prime = 104729;
	//first_prime = 23;
	//second_prime = 17;
	first_prime = 13;
	second_prime = 23;
	flag = prime(second_prime, &j);
	nonce = first_prime * second_prime;
	phi_primes = (first_prime-1) * (second_prime-1);
	encryption_key(first_prime,second_prime,phi_primes,&flag,e,d,&j,&i);
	

	
	if(recv(peer_socket, client_public_key, sizeof(client_public_key), 0)<0){
		fprintf(stderr, "error receiving --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	printf("Client Public Key Received\n");
	printf("Sending Public Key\n");
	
	if(send(peer_socket, d, sizeof(e), 0)<0){
		fprintf(stderr, "Error sending --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}


	printf("Secure Connection Established\n");

	
	char input[MSG_BUFFER_SIZE];
	char received[MSG_BUFFER_SIZE];
	memset(input, 0, MSG_BUFFER_SIZE);
	memset(received, 0, MSG_BUFFER_SIZE);
	rec_struct *args = malloc(sizeof *args);
	args->sen = input;
	args->rec = received;
	args->socket = peer_socket;
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
	
	
	//Sets up the entirety of send and receive
	set_up_send_receive(args);

	while(send(peer_socket, conn_check, sizeof(conn_check), 0) > -1) {
		sleep(1);
	}
	printf("Initiating disconnect from server side\n");
		close(peer_socket);
		close(server_socket);

		return 0;
}

