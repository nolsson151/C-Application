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

void *client_handler(void *); 
void send_hello(int);
void sendId_Ip(int socket);
void sendtime(int socket);
void senduname(int socket);
void sendscandir(int socket);


static void handler(int sig, siginfo_t *siginfo, void *context) 
{
    struct timeval tv1, tv2; 
    if (gettimeofday(&tv1, NULL) == -1) { 
	perror("gettimeofday error");
        exit(EXIT_FAILURE);
    }

    clock_t start, end;
    if ((start = clock()) == -1) {
	perror("clock start error");
	exit(EXIT_FAILURE);
    }

    int i;
    int j = 55;
    for (i = 0; i < 3000000; i++) {
	j++;
    }

    if ((end = clock()) == -1) {
	perror("clock end error");
	exit(EXIT_FAILURE);
    }

    printf("Time on CPU = %f seconds\n",
	   ((double) (end - start)) / CLOCKS_PER_SEC);


    if (gettimeofday(&tv2, NULL) == -1) {
	perror("gettimeofday error");
	exit(EXIT_FAILURE);
    }

    printf("Total execution time = %f seconds\n",
	   (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	   (double) (tv2.tv_sec - tv1.tv_sec));
    exit(EXIT_SUCCESS);
}

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

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_sigaction = &handler;
    act.sa_flags = SA_SIGINFO;

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
	perror("Failed to listen");
	exit(EXIT_FAILURE);
    }

    if (sigaction(SIGINT, &act, NULL) == -1) { 
	perror("sigaction");
	exit(EXIT_FAILURE);
    }

    puts("Waiting for incoming connections...");
    while (1) {
	printf("Waiting for a client to connect...\n");
	connfd = accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
	printf("Connection accepted...\n");
	pthread_t sniffer_thread;

	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
	    perror("could not create thread");
	    exit(EXIT_FAILURE);
	}

	printf("Handler assigned\n");
    }

    exit(EXIT_SUCCESS);
} 

void *client_handler(void *socket_desc)
{
    int connfd = *(int *) socket_desc;

    send_hello(connfd); 
    char input; 
    
    do {
    char option_string[2];
    size_t p;

    readn(connfd, (unsigned char *) &p, sizeof(size_t));    
    readn(connfd, (unsigned char *) option_string, p);
    if(option_string == '\0') 
        input = '\0'; 
    printf("Received option: %s\n", option_string);
    option_string[strcspn(option_string, "\n")] = 0; 
    input = option_string[0];

    switch (input) {
    case '0':
        break;
    case '1':
        sendId_Ip(connfd); 
        break;
    case '2':
        sendtime(connfd); 
        break;
    case '3':
        senduname(connfd); 
        break;
    case '4':
        sendscandir(connfd); 
        break;
    case '5':
        printf("Client disconnected!\n"); 
        input = '\0';
        break;
    default:
        printf("Invalid choice!\n");
        break;
    }
    } while (input != '\0');

    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}  

void send_hello(int socket)
{
    char hello_string[] = "Hello Niklas!"; 

    size_t n = strlen(hello_string) + 1; 
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) hello_string, n);	  
} 

void sendId_Ip(int socket1){
    int fd; 
    struct ifreq ifr; 
    char *ip; 
    char studentId[] = "S1434184 "; 

    fd = socket(AF_INET, SOCK_DGRAM, 0); 
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);
    ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr); 

    strcat(studentId, ip); 
    size_t n = strlen(studentId) + 1;
    writen(socket1, (unsigned char *) &n, sizeof(size_t)); 
    writen(socket1, (unsigned char *) studentId, n);  
}

void sendtime(int socket)
{
    char *servertime;
    time_t t;
    if ((t = time(NULL)) == -1) {
	perror("time error");
	exit(EXIT_FAILURE);
    }
    struct tm *tm; 
    if ((tm = localtime(&t)) == NULL) {
	perror("localtime error");
	exit(EXIT_FAILURE);
    }    
    servertime = asctime(tm); 
    size_t q = strlen(servertime) + 1;
    writen(socket, (unsigned char *) &q, sizeof(size_t)); 
    writen(socket, (unsigned char *) servertime, q);  
}

void senduname(int socket)
{
    struct utsname uts; 
    size_t uname_length = sizeof(uts);
    

    if (uname(&uts) == -1) { 
	perror("uname error");
	exit(EXIT_FAILURE);
    }
    writen(socket, (unsigned char *) &uname_length, sizeof(size_t)); 
    writen(socket, (unsigned char *) &uts, uname_length);	

}

void sendscandir(int socket)
{
    char *dirfile; 
    dirfile = malloc(100 * sizeof(char) ); 
    memset(dirfile, '\0', 100); 
    struct dirent **namelist; 
    int n;
    if ((n = scandir("./upload", &namelist, NULL, alphasort)) == -1) 
	perror("Error");
    else {
	while (n--) { 
	    strcat(dirfile, namelist[n]->d_name); 
            strcat(dirfile, "\n"); 
	    free(namelist[n]);	
	}
	free(namelist);	 
    }
    char buffer[100]; 
    strcpy(buffer, dirfile); 
    free(dirfile); 
    size_t h = strlen(buffer) + 1;
    writen(socket, (unsigned char *) &h, sizeof(size_t)); 
    writen(socket, (unsigned char *) buffer, h);
    
}
