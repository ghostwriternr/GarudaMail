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
int   port_num  = 21439;

void serviceready( int cfd );
void receive(int cfd, char buf[]);
void HELO(int cfd,char buf[],char domain[]);
void MAILFROM(int cfd,char buf[],char sender[]);    
void RCPTTO(int cfd,char buf[],char receivers[][500],int *rsize);
void DATA(int cfd,char data[]);
void done(int cfd,char domain[],char sender[],char receivers[][500],int rsize,char data[]);

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
    srv_addr.sin_port   = htons (port_num); // Port Number.

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
        char domain[100] = {'\0'};
        char sender[500] = {'\0'};
        char receivers[20][500] = {'\0'};
        char data[10000] = {'\0'};
        int rsize=0;
        serviceready(cfd);
        while(1)
        {
            receive(cfd,buf);
            if(strstr(buf,"HELO ") != NULL)
            {
                HELO(cfd, buf, domain);
            }
            else if(strstr(buf,"MAIL FROM ") != NULL)
            {
                MAILFROM(cfd, buf, sender);    
            }
            else if(strstr(buf,"RCPT TO ") != NULL)   // can be multiple receivers
            {
                RCPTTO(cfd, buf, receivers, &rsize);
            }
            else if(strcmp(buf,"DATA")==0) //content will be entered from next line onwards
            {
                DATA(cfd,data);
                done(cfd, domain, sender, receivers, rsize, data);
            }
            else if(strcmp(buf,"QUIT")==0 ) //content will be entered from next line onwards
            {
                memset(buf,'\0',strlen(buf));
                strcpy(buf,"221 service closed");
                rst = send(cfd, buf, strlen(buf), 0);
                break;
            }
            else if(strcmp(buf,"RSET")==0 )
            {
                int j;
                memset(buf,'\0',strlen(buf));
                memset(domain,'\0',strlen(domain));
                memset(sender,'\0',strlen(sender));
                for(j=0;j<rsize;j++)
                    memset(receivers[j],'\0',strlen(receivers[j]));
                memset(data,'\0',strlen(data));
                strcpy(buf,"250 OK");
                rst = send(cfd, buf, strlen(buf), 0);
                memset(buf,'\0',strlen(buf));
                rsize=0;
            }
        }
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

void serviceready( int cfd )
{
    int rst;
    char buf[BUF_SIZE] = {'\0'};
    strcpy(buf,"220 service ready");
    rst = send(cfd, buf, strlen(buf), 0);
}

void receive(int cfd, char buf[])
{
    int rst;
    memset(buf,'\0',strlen(buf));
    rst = recv(cfd, buf, BUF_SIZE, 0);
    printf("Received : %s\n",buf);
}

void HELO(int cfd,char buf[],char domain[])
{
    int rst;
    int i;
    for(i=0;buf[i]!=' ';i++);
    strcpy(domain,&buf[i+1]);
    memset(buf,'\0',strlen(buf));
    strcpy(buf,"250 OK");
    rst = send(cfd, buf, strlen(buf), 0);
}

void MAILFROM(int cfd,char buf[],char sender[])
{
    int rst;
    int i;
    for(i=0;buf[i]!=' ';i++);
    strcpy(sender,&buf[i+1]);
    memset(buf,'\0',strlen(buf));
    strcpy(buf,"250 OK");
    rst = send(cfd, buf, strlen(buf), 0);
}    

void RCPTTO(int cfd,char buf[],char receivers[][500],int *rsize)
{
    int rst;
    int i;
    for(i=0;buf[i]!=' ';i++);
    strcpy(receivers[*rsize],&buf[i+1]);
    *rsize = *rsize + 1;
    memset(buf,'\0',strlen(buf));
    strcpy(buf,"250 OK");
    rst = send(cfd, buf, strlen(buf), 0);
}

void DATA(int cfd,char data[])
{
    char buf[1000];
    int rst;
    strcpy(buf,"354 start mail input");
    rst = send(cfd, buf, strlen(buf), 0);
    do{
        memset(buf,'\0',strlen(buf));
        receive(cfd,buf);
        strcat(data,buf);
    }while(strstr(buf,".")==NULL);
}

void done(int cfd,char domain[],char sender[],char receivers[][500],int rsize,char data[])
{
    printf("domain : %s\n",domain);
    printf("sender : %s\n",sender);
    int i;
    for(i=0; i < rsize;i++)
    printf("receivers[%d] : %s\n",i+1,receivers[i]);
    printf("data : %s\n",data);
}

/**
    int rst;
    char buf[BUF_SIZE] = {'\0'};
    strcpy(buf,"220 service ready");
    rst = send(cfd, buf, strlen(buf), 0);
    memset(buf,'\0',strlen(buf));
    rst = recv(cfd, buf, BUF_SIZE, 0);
    if(rst==-1)
        printf("Sending failed!\n");
    printf("%s\n",buf);
    memset(buf,'\0',strlen(buf));
    strcpy(buf,"250 OK");
    rst = send(cfd, buf, strlen(buf), 0);
*/