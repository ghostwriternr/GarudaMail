#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num = 21436;

#define BUF_SIZE 10000

int main (int argc, char *argv[])
{
while (1)
  {
	int rst; // Return status of functions.
	int cfd; // File descriptor for the client.

	/**************** Create a socket. *******************************/
	int sfd; // Socket file descriptor.
	sfd = socket (AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,
			 * SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
	if (sfd == -1)
	{
	  perror ("Client: socket error");
	  exit (1);
	}
	printf ("Socket fd = %d\n", sfd);
	/***************** Assign an address of the server **************/
	struct sockaddr_in srv_addr, cli_addr; // Addresses of the server and the client.
	socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.

	// Clear the two addresses.
	memset (&srv_addr, 0, addrlen);

	// Assign values to the server address.
	srv_addr.sin_family = AF_INET; // IPv4.
	srv_addr.sin_port   = htons (port_num); // Port Number.

	rst = inet_pton (AF_INET, "127.0.0.1", &srv_addr.sin_addr); /* To
							  * type conversion of the pointer here. */
	if (rst <= 0)
	{
	  perror ("Client Presentation to network address conversion.\n");
	  exit (1);
	}

	/***************** Connect to the server ************************/
	rst = connect (sfd, (struct sockaddr *) &srv_addr, addrlen);
	if (rst == -1)
	{
	  perror ("Client: Connect failed.");
	  exit (1);
	}

	//continue from here onwards
	
	char buf[BUF_SIZE] = {'\0'}, *token, a[3];
	printf("Your Name\n");
	scanf("%s", buf);
	rst = send(sfd, buf, strlen(buf), 0);
	rst = close (sfd);
}
  return 0;
}