#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "rdwrn.h"
#include <sys/utsname.h>
#include <dirent.h>
#include <sys/stat.h>

#define INPUTSIZ 128

void get_hello(int socket)
{
    char hello_string[32];
    size_t k; 
    readn(socket, (unsigned char *) &k, sizeof(size_t)); 	
    readn(socket, (unsigned char *) hello_string, k); 
    printf("Hello String: %s\n", hello_string); 
    printf("Received: %zu bytes\n\n", k); 
}

void getId_Ip(int socket){

    char studentId_Ip[40]; 
    size_t n;
    readn(socket, (unsigned char *) &n, sizeof(size_t)); 
    readn(socket, (unsigned char *) studentId_Ip, n); 

    printf("Student: %s\n", studentId_Ip); 
    printf("Received: %zu bytes\n\n", n); 
}

void gettime(int socket)
{
    char servertime[40];  
    size_t q;
    readn(socket, (unsigned char *) &q, sizeof(size_t));    
    readn(socket, (unsigned char *) servertime, q);

    printf("Server time: %s\n", servertime); 
    printf("Received: %zu bytes\n\n", q);
}

void getuname(int socket)
{
    struct utsname uts; 
    size_t uname_length; 
    readn(socket, (unsigned char *) &uname_length, sizeof(size_t)); 
    readn(socket, (unsigned char *) &uts, uname_length); 
    
    printf("Node name:    %s\n", uts.nodename); 
    printf("System name:  %s\n", uts.sysname);
    printf("Release:      %s\n", uts.release);
    printf("Version:      %s\n", uts.version);
    printf("Machine:      %s\n", uts.machine);
}
void getscandir(int socket)
{
    char *dirfile = malloc(100 * sizeof(char) ); 
    size_t h;
    readn(socket, (unsigned char *) &h, sizeof(size_t));    
    readn(socket, (unsigned char *) dirfile, h);

    printf("Files in server:\n%s\n", dirfile);
    printf("Received: %zu bytes\n", h);
    free(dirfile); 
}
void firstoption(int socket)
{
    printf("Get student ID and IP address...\n");    

    char option[] = "1"; 
    size_t n = strlen(option) + 1; 
    writen(socket, (unsigned char *) &n, sizeof(size_t)); 	
    writen(socket, (unsigned char *) option, n); 
    getId_Ip(socket); 
}
void secondoption(int socket)
{
    printf("Get the server time...\n");
    
    char option[] = "2";
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);	  
    gettime(socket);
}
void thirdoption(int socket)
{
    printf("Get server system information...\n"); 

    char option[] = "3";
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);
    getuname(socket);	  
}
void fourthoption(int socket)
{
    printf("Get the names of server files...\n"); 

    char option[] = "4";
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);
    getscandir(socket);
}
void fifthoption(int socket)
{
    char option[] = "5"; 
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);
}
void displaymenu() 
{
    printf("0. Display menu\n");
    printf("1. Get Student Id and Client IP address\n");
    printf("2. Get Server time\n");
    printf("3. Get System information of Server\n");
    printf("4. Get list of files in the Server upload Directory\n");
    printf("5. Exit\n");
}

int main(void)
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(55456);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");

    get_hello(sockfd); 

    char input; 
    char name[INPUTSIZ]; 
    displaymenu(); 

    do {
	printf("option> ");
	fgets(name, INPUTSIZ, stdin); 
	name[strcspn(name, "\n")] = 0; 
	input = name[0]; 
	if (strlen(name) > 1) 
	    input = 'x'; 	

	switch (input) {
	case '0':
	    displaymenu();
	    break;
	case '1':
	    firstoption(sockfd);
	    break;
	case '2':
	    secondoption(sockfd);
	    break;
	case '3':
	    thirdoption(sockfd);
	    break;
	case '4':
	    fourthoption(sockfd);
	    break;
	case '5':
	    printf("Goodbye!\n");
            fifthoption(sockfd);
	    break;
	default:
	    printf("Invalid choice - 0 displays options...!\n");
	    break;
	}
    } while (input != '5');

    close(sockfd);

    exit(EXIT_SUCCESS);
} 
