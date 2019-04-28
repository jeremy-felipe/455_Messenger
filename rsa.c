#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>


int first_prime, second_prime, nonce, phi_primes, i, flag;
long int e[500], d[500], temp[500], j, m[500], en[500];
char msg[1000];

int prime(long int, long int*);
void encryption_key(int, int, int, int*, long int *, long int *, long int *,int*);
long int cd(long int, int);
void encrypt(long int *, char *, int, long int *, long int *);
void decrypt(long int, long int *, long int *, int, long int *);

int main()
{
	//Take user input to get first prime
	printf("\nENTER FIRST PRIME NUMBER\n");
	scanf("%d", &first_prime);
	flag = prime(first_prime, &j);
	if(flag == 0)
	{
		printf("\nINVALID INPUT\n");
		exit(0);
	}
	
	//Take user input to get second prime
	printf("\nENTER SECOND PRIME NUMBER\n");
	scanf("%d", &second_prime);
	flag = prime(second_prime, &j);
	if(flag == 0 || first_prime == second_prime)
	{
		printf("\nINVALID INPUT\n");
		exit(0);
	}
	
	//Take user input to get message/string to encrypt
	memset(msg,0, 1000);
	printf("\nENTER MESSAGE OR STRING TO ENCRYPT\n");
	//fscanf(stdin,"%s",msg);
	scanf(" %[^\n]", msg);
	
	for(i = 0; msg[i] != '\0'; i++)
	{
		m[i] = msg[i];
	}
	
	
	nonce = first_prime * second_prime;
	phi_primes = (first_prime-1) * (second_prime-1);
	encryption_key(first_prime,second_prime,phi_primes,&flag,e,d,&j,&i);
	printf("\nPOSSIBLE VALUES OF e AND d ARE\n");
	for(i = 0; i < j-1; i++)
	{
		printf("\n%ld\t%ld", e[i], d[i]);
	}
	encrypt(e,msg,nonce,temp,en);
	decrypt(j,temp,en,nonce,m);
	return 0;
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
void encryption_key(int first_prime, int second_prime, int phi_primes, int *flag, long int *e, long int *d, long int * j, int *i)
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
void encrypt(long int *e, char *msg, int nonce, long int *temp, long int *en)
{
	long int pt, ct, key = e[0], k, len, i;
	i = 0;
	len = strlen(msg);
	while(i != len)
	{
		pt = m[i];
		pt = pt - 96;
		k = 1;
		for(j = 0; j < key; j++)
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
}





//function to decrypt the message
void decrypt(long int j, long int *temp, long int *en, int nonce, long int *m)
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
}