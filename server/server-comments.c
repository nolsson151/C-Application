// Cwk2: server.c - multi-threaded server using readn() and writen()

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "rdwrn.h"
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>


// thread function
void *client_handler(void *); //functions declared, to be used in client handler 
void send_hello(int);
void sendId_Ip(int socket);
void sendtime(int socket);
void senduname(int socket);
void sendscandir(int socket);


static void handler(int sig, siginfo_t *siginfo, void *context) //signal hanlder
{
    struct timeval tv1, tv2; // two structs that we will use to get execution time

    // get "wall clock" time at start
    if (gettimeofday(&tv1, NULL) == -1) { 
	perror("gettimeofday error");
        exit(EXIT_FAILURE);
    }
    // set CPU time at start
    clock_t start, end;
    if ((start = clock()) == -1) {
	perror("clock start error");
	exit(EXIT_FAILURE);
    }
    // do something CPU bound - anything really... 
    // I was confused by this, I did not know how to do something that would increase the CPU time
    int i;
    int j = 55;
    for (i = 0; i < 3000000; i++) {
	j++;
    }

    // set CPU time at end
    if ((end = clock()) == -1) {
	perror("clock end error");
	exit(EXIT_FAILURE);
    }

    printf("Time on CPU = %f seconds\n",
	   ((double) (end - start)) / CLOCKS_PER_SEC);

    // get "wall clock" time at end
    if (gettimeofday(&tv2, NULL) == -1) {
	perror("gettimeofday error");
	exit(EXIT_FAILURE);
    }
    // in microseconds...
    // this also confused me, not sure what how to change this to something more applicable
    printf("Total execution time = %f seconds\n",
	   (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	   (double) (tv2.tv_sec - tv1.tv_sec));
    exit(EXIT_SUCCESS);
}

// you shouldn't need to change main() in the server except the port number
int main(void)
{
    int listenfd = 0, connfd = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    socklen_t socksize = sizeof(struct sockaddr_in);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(55456);

    struct sigaction act; // struct for the singal
    memset(&act, '\0', sizeof(act)); //space for the signal saction
    act.sa_sigaction = &handler; // define what handler to use
    act.sa_flags = SA_SIGINFO; // use the handler created

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
	perror("Failed to listen");
	exit(EXIT_FAILURE);
    }
    // end socket setup

    if (sigaction(SIGINT, &act, NULL) == -1) { //performs hanlder on signal interupt
	perror("sigaction");
	exit(EXIT_FAILURE);
    }


    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    while (1) {
	printf("Waiting for a client to connect...\n");
	connfd =
	    accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
	printf("Connection accepted...\n");

	pthread_t sniffer_thread;
        // third parameter is a pointer to the thread function, fourth is its actual parameter
	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
	    perror("could not create thread");
	    exit(EXIT_FAILURE);
	}
	//Now join the thread , so that we dont terminate before the thread
	//pthread_join( sniffer_thread , NULL);
	printf("Handler assigned\n");
    }


    // never reached...
    // ** should include a signal handler to clean up
    exit(EXIT_SUCCESS);
} // end main()

// thread function - one instance of each for each connected client
// this is where the do-while loop will go
void *client_handler(void *socket_desc)
{
    //Get the socket descriptor
    int connfd = *(int *) socket_desc;

    send_hello(connfd); //sends message to establish connection is made
    char input; //a char used for the do while loop, if == 5 then we close the loop and close the program
    
    do {
    char option_string[2]; // set space for a string, this is will be for the client options
    size_t p;

    readn(connfd, (unsigned char *) &p, sizeof(size_t));    
    readn(connfd, (unsigned char *) option_string, p);
    if(option_string == '\0') //if an empty string is read, the client has disconnected
        input = '\0'; // bombs out the loop
    printf("Received option: %s\n", option_string); // prints the send option
    option_string[strcspn(option_string, "\n")] = 0; //gets the first character before return
    input = option_string[0]; // sets input to the first element in array

    switch (input) {
    case '0': //this options never happens, no need to display menu 
        break;
    case '1':
        sendId_Ip(connfd); //sends the student Id and IP address
        break;
    case '2':
        sendtime(connfd); //sends the server time
        break;
    case '3':
        senduname(connfd); //sends the server system details
        break;
    case '4':
        sendscandir(connfd); //sends file names in server directory
        break;
    case '5':
        printf("Client disconnected!\n"); //shows that client has exited
        input = '\0'; //bombs out the loop
        break;
    default:
        printf("Invalid choice!\n");
        break;
    }
    } while (input != '\0');

    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    // always clean up sockets gracefully
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}  // end client_handler()

// how to send a string
void send_hello(int socket)
{
    char hello_string[] = "Hello Niklas!"; // string sent to welcome user

    size_t n = strlen(hello_string) + 1; //size is equal to string plus 1 for \0
    writen(socket, (unsigned char *) &n, sizeof(size_t)); //writes to client	
    writen(socket, (unsigned char *) hello_string, n);	  
} // end send_hello()

void sendId_Ip(int socket1){ // have to make sure we don't confuse the sockets, hence socket1
    int fd; // int for the socket
    struct ifreq ifr; //struct to hold get IP info
    char *ip; //pointer to string to hold the IP 
    char studentId[] = "S1434184 "; // hardcoded student Id string

    fd = socket(AF_INET, SOCK_DGRAM, 0); // assigns socket to fd
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want an IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd); // clost up the socket
    ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr); // deference pointer assign the IP address to ip char

    strcat(studentId, ip); // put together IP and student Id 
    size_t n = strlen(studentId) + 1;
    writen(socket1, (unsigned char *) &n, sizeof(size_t)); // send the studentIp with the IP address
    writen(socket1, (unsigned char *) studentId, n);  
}

void sendtime(int socket)
{
    char *servertime; //space for string
    time_t t; // time variable
    if ((t = time(NULL)) == -1) {
	perror("time error");
	exit(EXIT_FAILURE);
    }
    struct tm *tm; //creates a time struct 
    if ((tm = localtime(&t)) == NULL) { // gets 
	perror("localtime error");
	exit(EXIT_FAILURE);
    }    
    servertime = asctime(tm); // set the time to a string 
    size_t q = strlen(servertime) + 1;
    writen(socket, (unsigned char *) &q, sizeof(size_t)); // send string to client  
    writen(socket, (unsigned char *) servertime, q);  
}

void senduname(int socket)
{
    struct utsname uts; // define a uts struct
    size_t uname_length = sizeof(uts);
    

    if (uname(&uts) == -1) { 
	perror("uname error");
	exit(EXIT_FAILURE);
    }
    writen(socket, (unsigned char *) &uname_length, sizeof(size_t)); //send struct to client
    writen(socket, (unsigned char *) &uts, uname_length);	

}

void sendscandir(int socket)
{
    char *dirfile; // define a pointer, this will hold the string of the filenames
    dirfile = malloc(100 * sizeof(char) ); // make space for it, 100 bytes will do 
    memset(dirfile, '\0', 100); // define the string in memory first, or it will not be initialized. set to nothing for now
    struct dirent **namelist; // this is the list of names of files in the directory
    int n;
// this scans the upload directory in server
    if ((n = scandir("./upload", &namelist, NULL, alphasort)) == -1) // checks if the directory is there
	perror("Error");
    else {
	while (n--) { //loops while there is anything in directory
	    strcat(dirfile, namelist[n]->d_name); //this adds the filname name of the to the dirfile string 
            strcat(dirfile, "\n"); // adds a return to that dirfile string
	    free(namelist[n]);	// frees the memory used for the file
	}
	free(namelist);	// frees the struct 
    }
    char buffer[100]; // this will be used as a buffer for the dirfile
    strcpy(buffer, dirfile); //we copy the string to test, so we can free the memory at dirfile
    free(dirfile); // allocated memory is freed
    size_t h = strlen(buffer) + 1;
    writen(socket, (unsigned char *) &h, sizeof(size_t)); // send to client.
    writen(socket, (unsigned char *) buffer, h);
    
}
