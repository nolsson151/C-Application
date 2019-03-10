// Cwk2: client.c - message length headers with variable sized payloads
//  also use of readn() and writen() implemented in separate code module

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
// gets a hello statement from server to show connection
void get_hello(int socket)
{
    char hello_string[32]; // space for incoming string
    size_t k; 
    readn(socket, (unsigned char *) &k, sizeof(size_t)); 	
    readn(socket, (unsigned char *) hello_string, k); // reads the actual string

    printf("Hello String: %s\n", hello_string); // prints string to screen
    printf("Received: %zu bytes\n\n", k); // prints the size of the string
} // end get_hello()

//gets the student Id and IP address
void getId_Ip(int socket){

    char studentId_Ip[40]; // space for incoming string
    size_t n;
    readn(socket, (unsigned char *) &n, sizeof(size_t)); // reads the length of string 
    readn(socket, (unsigned char *) studentId_Ip, n); // prints string to screen

    printf("Student: %s\n", studentId_Ip); // prints the student id and the ip of client
    printf("Received: %zu bytes\n\n", n); // prints string to screen
}
//gets server time
void gettime(int socket)
{
    char servertime[40]; //same as before 
    size_t q;
    readn(socket, (unsigned char *) &q, sizeof(size_t));    
    readn(socket, (unsigned char *) servertime, q);

    printf("Server time: %s\n", servertime); // same as before, formating is done in server
    printf("Received: %zu bytes\n\n", q);
}
//gets server system detials
void getuname(int socket)
{
    struct utsname uts; // declare a utsname struct and name it uts
    size_t uname_length; 
    readn(socket, (unsigned char *) &uname_length, sizeof(size_t)); // get the length of the struct   
    readn(socket, (unsigned char *) &uts, uname_length); // read in the struct 
    // prints out all the struct variables, we can do this since we have the library for utsname
    printf("Node name:    %s\n", uts.nodename); 
    printf("System name:  %s\n", uts.sysname);
    printf("Release:      %s\n", uts.release);
    printf("Version:      %s\n", uts.version);
    printf("Machine:      %s\n", uts.machine);
}
void getscandir(int socket)
{
    char *dirfile = malloc(100 * sizeof(char) ); // creating some memory because we have a bit bigger string, to stop stack smashing
    size_t h;
    readn(socket, (unsigned char *) &h, sizeof(size_t));    
    readn(socket, (unsigned char *) dirfile, h);

    printf("Files in server:\n%s\n", dirfile);
    printf("Received: %zu bytes\n", h);
    free(dirfile); // since we malloc, we must free the space
}
void firstoption(int socket)
{
    printf("Get student ID and IP address...\n");    

    char option[] = "1"; // this the string we will send to the server to select an option
    size_t n = strlen(option) + 1; // have to include the \0.
    writen(socket, (unsigned char *) &n, sizeof(size_t)); //send the size of string	
    writen(socket, (unsigned char *) option, n); // send the actual string
    getId_Ip(socket); // then call the method we want that first option to perform.
}
void secondoption(int socket)
{
    printf("Get the server time...\n"); // follows the same as above
    
    char option[] = "2";
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);	  
    gettime(socket);
}
void thirdoption(int socket)
{
    printf("Get server system information...\n"); // same again

    char option[] = "3";
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);
    getuname(socket);	  
}
void fourthoption(int socket)
{
    printf("Get the names of server files...\n"); // and again

    char option[] = "4";
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);
    getscandir(socket);
}
void fifthoption(int socket)
{
    char option[] = "5"; // this is sent to tell the server we wish to disconnect
    size_t n = strlen(option) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) option, n);
}
void displaymenu() // displays a menu of the options 
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
    // *** this code down to the next "// ***" does not need to be changed except the port number
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;

    // IP address and port of server we want to connect to
    serv_addr.sin_port = htons(55456);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // try to connect...
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");

    get_hello(sockfd); //gets an inital message to show we are connected

    char input; //a char used for the do while loop, if == 5 then we close the loop and close the porgram
    char name[INPUTSIZ]; //a buffer to hold user options

    displaymenu(); // displays the menu

    do {
	printf("option> ");
	fgets(name, INPUTSIZ, stdin); //gets user input from standard input
	name[strcspn(name, "\n")] = 0; //gets the first character before return
	input = name[0]; // sets input to the first element in array
	if (strlen(name) > 1) // tests if the string is too long
	    input = 'x'; // unrecognized character	

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
    // *** make sure sockets are cleaned up

    close(sockfd);

    exit(EXIT_SUCCESS);
} // end main()
