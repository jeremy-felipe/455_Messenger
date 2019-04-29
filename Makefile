default : 
	gcc -pthread -o simple_server server2.c -lm
	gcc -pthread -o simple_client client2.c -lm

clean: remove default

remove : 
	rm simple_client
	rm simple_server
