default : 
	gcc -pthread -o simple_server server.c -lm
	gcc -pthread -o simple_client client.c -lm

clean: remove default

remove : 
	rm simple_client
	rm simple_server
	rm outputfile.html
