#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num ;	//send email port num
int port_num_xyz = 21000;
int	port_pop_xyz = 22000;
int port_num_abc = 23000;
int port_pop_abc = 24000;
int port_pop;

#define BUF_SIZE 10000

char ip_xyz[20]={"10.117.11.124"};
char ip_abc[20]={"10.117.11.106"};
char ip[20];
char givendomain[100];

void receive(int sfd, char buf[]);
void interact(int sfd, char S[]);
void DATA(int sfd);
void send_mail(int sfd);
int	 mailfrom_helo(int sfd, char S[]);
void retrieve_mail(int sfd);
void set(char buf[], int *r, int *u);

int main (int argc, char *argv[])
{
	int i;
   	printf("> Do you want to send emails(1) or retrieve emails(2) ?\n> ");
    scanf("%d",&i);
	printf("Domain Name?\n> ");
	scanf("%s",givendomain);
	if(strcmp(givendomain,"xyz.com") == 0)
		{
			strcpy(ip,ip_xyz);
			if(i==2)
				port_num = port_pop_xyz;
			else
				port_num = port_num_xyz;
		}
	else if(strcmp(givendomain,"abc.com") == 0)
		{
			strcpy(ip,ip_abc);
			if(i==2)
				port_num = port_pop_abc;
			else
				port_num = port_num_abc;
			}
	else
	{
		printf("Incorrect Domain Name!\nAborting the execution!\n");
		return 1;
	}
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

	rst = inet_pton (AF_INET, ip, &srv_addr.sin_addr); /* To
							  * type conversion of the pointer here. */
	if (rst <= 0)
	{
	  perror ("Client Presentation to network address conversion.\n");
	  exit (1);
	}
	/***************** Connect to the server ************************/
	rst = connect (sfd, (struct sockaddr *) &srv_addr, addrlen);
	if(i==1)
    	send_mail(sfd);
    else
    	retrieve_mail(sfd);
    if (rst == -1)
	{
	  perror ("Client: Connect failed.");
	  exit (1);
	}

	rst = close (sfd);
	return 0;
}

int mailfrom_helo(int sfd, char S[])
{
	char helostr[100]={"HELO "};
	int i,j,n = strlen(S);
	for(i=1;S[i-1] != '@' && i<n;i++);
	if(S[i-1]!='@')
		return 0;
	for(j=strlen(helostr);i<n;i++,j++)
	{
		helostr[j]=S[i];
	}
	helostr[j] = '\0';
	interact(sfd,helostr);
	//printf("Here\n");
	char mailstr[1000] = { "MAIL FROM " };
	strcat(mailstr,S);
	interact(sfd,mailstr);
	return 1;
}

void send_mail(int sfd)
{
	char S[10000],temp[1000];
	int i;
    while(1)
    {
    	do{
    		printf("> Mail from?\n> ");
    		scanf("%s",S);
    		if(strstr(S,givendomain)==NULL)
    			printf("> Error : Domain name and Email mismatch!\n> ");
    	}while(strstr(S,givendomain)==NULL || mailfrom_helo(sfd, S) != 1);
    	printf("> Number of receipants?\n> ");
    	scanf("%d",&i);
    	while(i)
    	{
    		memset(temp,'\0',strlen(temp));
    		strcpy(temp,"RCPT TO ");
    		printf("> Mail to?\n> ");
    		scanf("%s",S);
    		strcat(temp,S);
    		interact(sfd,temp);
    		usleep(100000);
    		i-=1;
    	}
    	printf("> Enter mail body [end with a single dot(.)]\n> ");
    	DATA(sfd);
    	printf("> Mail sent successfully!\n> ");
    	printf("> Do you want to send more mails? (Y) Yes (N) NO\n> ");
    	char ch[2] ;
    	scanf("%s",ch);
    	if( ch[0] != 'Y')
    	{
    		printf("> Bye!\n");
    		memset(S,'\0',strlen(S));
    		strcpy(S,"QUIT");
    		interact(sfd,S);
    		exit(1);
    	}
    }
    //printf("exiting\n");
}

void interact(int sfd,char S[])
{
	//printf("inside interact %s\n",S);
    int rst;
    // printf("1\n");
	char buf[BUF_SIZE] = {'\0'};
    // printf("2\n");
	strcpy(buf,S);
	// printf("3 sfd %d \n",sfd);
	//sleep(1);
	usleep(100000);
	rst = send(sfd, buf, strlen(buf), 0);
    // printf("4\n");
	//printf("Sent %s, rst %d\n",S,rst);
    // printf("5\n");
	memset(buf,'\0',strlen(buf));
    // printf("6\n");
	rst = recv(sfd, buf, BUF_SIZE, 0);
    // printf("7\n");
	//printf("%s\n",buf);
	// printf("8\n");
}

void DATA(int sfd)
{
	int i, rst;
	char ch,S[10000]={"DATA"};
	interact(sfd,S);
	// printf("> ");
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
		usleep(100000);
    	rst = send(sfd, S, strlen(S), 0);
		if(i==1 && S[0]=='.')
				break;
	}
}

void retrieve_mail(int sfd)
{
	char buf[10000],temp[10000];
	int i,j,k,l,r,u,rst;
	do{
		do{
			printf("> username?\n> ");
			memset(buf,'\0',10000);
			memset(temp,'\0',10000);
			strcpy(buf,"USER ");
			scanf("%s",temp);
			if(strstr(temp,givendomain)==NULL)
    			{
    				printf("> Error : User Name and Domain mismatch!\n> ");
    			}
    		else
			{
				break;
			}
		}while(1);
		strcat(buf,temp);
	    usleep(100000);
    	rst = send(sfd, buf, strlen(buf), 0);
		receive(sfd,buf);
		printf("> Password?\n> ");
		memset(buf,'\0',10000);
		memset(temp,'\0',10000);
		strcpy(buf,"PASS ");
		scanf("%s",temp);
		strcat(buf,temp);
	    usleep(100000);
    	rst = send(sfd, buf, strlen(buf), 0);
	    receive(sfd,buf);
	    if(strstr(buf,"ERR")!=NULL)
	    {
	    	printf("> Error : Wrong Credentials!\n> ");
	    }
	    else
	    	break;
	}while(1);
	while(1)
	{	
		memset(buf,'\0',10000);
		strcpy(buf,"LIST");
		usleep(100000);
    	rst = send(sfd, buf, strlen(buf), 0);
	    //sleep(2);
		receive(sfd,buf);
	    set(buf,&r,&u);
	    if(r!=0)
	    	printf("> Already Read Emails :\n> ");
	    for(i=0,j=1;i<r;i++,j++)
	    {
	    	receive(sfd,buf);
	    	printf("\t%d %sbytes\n> ",j,buf);
	    }
	    if(u==0)
	    	printf("No Unread Emails!\n> ");
	    else
	    	printf("Unread Emails :\n> ");
	    for(i=0;i<u;i++,j++)
	    {
	    	receive(sfd,buf);
	    	printf("\t%d %sbytes\n> ",j,buf);
	    }
	    if((r+u)!=0)
	    {
	    	do{
		    	printf("Which one do you want to read?\n> ");
		    	scanf("%d",&i);
			}while(i>(r+u));
	    	memset(buf,'\0',10000);
			sprintf(buf,"%d",i);
			usleep(100000);
    		rst = send(sfd, buf, strlen(buf), 0);
	    	receive(sfd,buf);
	    	printf("> Mail :\n> %s",buf);
	    	printf("> Do you want to read more ? (Y) Yes (N) NO\n> ");
	    }
	    char ch[2];
	    if((r+u)!=0)
	    	scanf("%s",ch);
	    if(ch[0]!='Y' || (r+u)==0)
	    {
		    memset(buf,'\0',10000);
			strcpy(buf,"QUIT");
			usleep(100000);
    		rst = send(sfd, buf, strlen(buf), 0);		
	    	exit(1);
	    }
	}
}

void set(char buf[], int *r, int *u)
{
	int j,k=0,n=strlen(buf);
	for(j=0;buf[j]!=' ';j++)
	{
		k = 10 * k + buf[j] - '0';
	}
	*r=k;
	for(j=j+1,k=0;j<n;j++)
	{
		k = 10 * k + buf[j] - '0';
	}
	*u=k;
}

void receive(int sfd, char buf[])
{
    int rst;
    do{
    	memset(buf,'\0',strlen(buf));
    	rst = recv(sfd, buf, BUF_SIZE, 0);
	}while(strlen(buf)==0);
    // printf("Received : %s\n",buf);
}
