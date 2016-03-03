#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>   // For the socket (), bind () etc. functions.
#include <netinet/in.h>  // For IPv4 data struct..
#include <string.h>     // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

#define BUF_SIZE  10000
char  path[100] = {"./articles/"}, data[1000] = {"./data/"};
int   port_num  = 23464;

int main( )
{
    int rst; // Return status of functions.
    int cfd; // File descriptor for the client.

    /*
        Create a socket
    */
    
    int sfd;                                // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_STREAM, 0); /*  AF_INET --> IPv4, * SOCK_STREAM --> TCP Protocol,
                                                 0 --> for the protocol. */
    if (sfd == -1)
    {
      perror ("Server: socket error");
      exit (1);
    }

    printf ("Socket fd = %d\n", sfd);

    /***************** Assign an address to the server **************/

    struct sockaddr_in srv_addr, cli_addr; // Addresses of the server and the client.

    socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.

    // Clear the two addresses.
    memset (&srv_addr, 0, addrlen);
    memset (&cli_addr, 0, addrlen);

    // Assign values to the server address.
    srv_addr.sin_family = AF_INET; // IPv4.
    //printf("port: %d\n",22436+atoi(argv[1]));
    srv_addr.sin_port   = htons (21436); // Port Number.

    rst = inet_pton (AF_INET, "127.0.0.1", &srv_addr.sin_addr); /* To
                              * type conversion of the pointer here. */
    if (rst <= 0)
    {
      perror ("Server Presentation to network address conversion.\n");
      exit (1);
    }

    /*
        Bind the server to an address. 
    */
    rst = bind (sfd, (struct sockaddr *) &srv_addr, addrlen); /* Note
        * the type casting of the pointer to the server's address. */

    if (rst == -1)
    {
      perror ("Server: Bind failed");
      exit (1);
    }

    /*
        listen (): Mark the socket as passive.
    */
    
    printf ("Max connections allowed to wait: %d\n", SOMAXCONN);
    rst = listen (sfd, SOMAXCONN);

    if (rst == -1)
    {
      perror ("Server: Listen failed");
      exit (1);
    }

    while (1)
    {
        /*
            accept (): Waiting for connections
        */
        
        cfd = accept (sfd, (struct sockaddr *) &cli_addr, &addrlen);
        
        /*
            Returns the file descriptor for the client.
            Stores the address of the client in cli_addr.
        */
        if (cfd == -1)
        {
            perror ("In Server: Accept failed");
            exit (1);
        }
      
    if (fork() == 0)
      {
        close(sfd);
        char buf[BUF_SIZE] = {'\0'};
        rst = recv(cfd, buf, BUF_SIZE, 0);
        if(rst==-1)
            printf("receive failed!\n");
        else
            printf("Received : %s\n",buf);
        rst = close(cfd);
        if (rst == -1)
        {
          perror ("Server close failed");
        }
        exit(0);
      }
      close(cfd);
    }
  return 0;
}