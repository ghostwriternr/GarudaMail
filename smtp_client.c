#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num = 21439;

#define BUF_SIZE 10000

void interact(int sfd, char S[]);
void DATA(int sfd, char S[]);

int main (int argc, char *argv[])
{
// while (1)
//   {
	char buf[10000] = {"\0"};
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
	rst = recv(sfd, buf, BUF_SIZE, 0);
    printf("%s\n",buf);
    memset(buf,'\0',strlen(buf));
    char S[10000],ch;
    int i,flag=0,check=1;
    while(1)
    {
    	i=0;
    	while(1)
    		{
    			ch=getchar();
    			if(ch=='\n')
    				break;
    			S[i++]=ch;
    		}
    	S[i]='\0';
    	printf("scanned %s\n",S);
    	if(strstr(S,"HELO ") != NULL && flag==0)
    	{
    		interact(sfd, S);
    		flag=1;
    	}
    	else if(strstr(S,"MAIL FROM ") != NULL && flag==1)
    	{
    		interact(sfd, S);
    		flag==2;
    		check=0;
    	}
    	else if(strstr(S,"RCPT TO ") != NULL && check==0)	// can be multiple receivers
    	{
    		interact(sfd, S);
    		flag=3;
    	}
    	else if(strcmp(S,"DATA")==0 && flag==3)	//content will be entered from next line onwards
    	{
    		DATA(sfd, S);
    		flag=4;
    	}
    	else if(strcmp(S,"QUIT")==0)	//content will be entered from next line onwards
    	{
    		interact(sfd, S);
    		break;
    	}
    	else
    		printf("> command not found!\n");
    	printf("> ");
    }
    if (rst == -1)
	{
	  perror ("Client: Connect failed.");
	  exit (1);
	}

	rst = close (sfd);
	return 0;
}

void interact(int sfd,char S[])
{
    int rst;
	char buf[BUF_SIZE] = {'\0'};
    strcpy(buf,S);
	rst = send(sfd, buf, strlen(buf), 0);
    memset(buf,'\0',strlen(buf));
    rst = recv(sfd, buf, BUF_SIZE, 0);
    printf("%s\n",buf);
}

void DATA(int sfd, char S[])
{
	int i, rst;
	char ch;
	interact(sfd,S);
	printf("> ");
	while(1)
	{
		memset(S,'\0',strlen(S));
    	i=0;
    	while(1)
    		{
    			ch=getchar();
    			if(ch=='\n')
    				break;
    			S[i++]=ch;
    		}
    	S[i]='\0';
		printf("> ");
		rst = send(sfd, S, strlen(S), 0);
		for(i=0;i<strlen(S);i++)
		if(S[i]=='.')
			break;
	}
}
