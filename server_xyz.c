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

int   port_num  = 21001;
int   port_num_copy = 21001;
int   port_pop = 22001;
int   other_server_port = 23001;

char  mydomain[20]={"xyz.com"};
char  otherdomain[20]={"abc.com"};

char ip_xyz[20]={"10.117.11.124"};
char ip_abc[20]={"10.117.11.106"};

void serviceready( int cfd );
void receive(int cfd, char buf[]);
void HELO(int cfd,char buf[],char domain[]);
void MAILFROM(int cfd,char buf[],char sender[]);    
void RCPTTO(int cfd,char buf[],char receivers[][500],int *rsize);
void DATA(int cfd,char data[],int isOther,char list[][1000],int *m);
void done(int cfd,char domain[],char sender[],char receivers[][500],int rsize,char data[]);
void receive_pop3(int cfd, char buf[]);
int  check_login_pop3(int cfd, char buf[],char usrname[]);
void send_details_pop3(int cfd,char user[],int *r,int *u);
int  list_pop3(int cfd, char buf[],char user[],int *r, int *u);
void retrieve_pop3(int cfd, char buf[],char user[]);
void pop3(int cfd );
void smtp(int cfd );
int  connect_to_other_server();

int main( )
{
    int rst; // Return status of functions.
    int cfd; // File descriptor for the client.
    int f;
    f = fork();
    if(f==0)
        port_num  = port_pop;
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

    rst = inet_pton (AF_INET, ip_xyz, &srv_addr.sin_addr); /* To
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
        * the type casting of the 
        pointer to the server's address. */

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
        printf("connected");
        if(port_num==port_num_copy)
            smtp(cfd);
        else
            pop3(cfd);
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

void smtp(int cfd)
{
    printf("HOLA");
    int transfer = -1;
    int isOther = 0;
    char buf[BUF_SIZE] = {'\0'};    
    serviceready(cfd);
    char domain[100] = {'\0'};
    char sender[500] = {'\0'};
    char receivers[20][500] = {'\0'};
    char data[10000] = {'\0'};
    char tt[10000] = {'\0'};
    char list[100][1000]={'\0'};

    strcpy(tt,"HELO abc.com");
    int m=1;
    int rsize=0,rst;
    while(1)
    {
        if(isOther)
            printf("\n\tisOther is set\n");
        receive(cfd,buf);
        //sleep(10);
        // if(strlen(buf)==0)
        //     continue;
        if(strstr(buf,"RCPT")==NULL)
            {
                memset(list[m],'\0',strlen(list[m]));
                if(strlen(buf)>1 && strstr(buf,"HELO")==NULL)
                    {
                        strcpy(list[m],buf);
                        m+=1;
                    }
            }
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
            if(strstr(buf,otherdomain)!=NULL)
                isOther=1;
            memset(data , '\0', 10000);
            RCPTTO(cfd, buf, receivers, &rsize);
        }
        else if(strcmp(buf,"DATA")==0) //content will be entered from next line onwards
        {
            DATA(cfd,data,isOther,list,&m);
            done(cfd, domain, sender, receivers, rsize, data);
            if(isOther==0)
            {
                rsize=0;
                m=1;
                continue;
            }
            int ll,p,k;
            if(transfer==-1 && isOther==1)
            {
                transfer = connect_to_other_server();
                receive(transfer,buf);
                printf("transfer Received %s\n",buf);
            }
            for(ll=0;ll<m;ll++)
                printf("list[%d] %s\n",ll,ll==0 ? tt : list[ll] );
            for(k=0;k<rsize && isOther==1;k++)
            {   if(strstr(receivers[k],otherdomain)==NULL)
                    continue;
                for(p=0;p<m;p+=1)
                {   usleep(100000);
                    printf("\n : %d : ",p);
                    if(p==0)
                        {
                            rst = send(transfer, tt, strlen(tt), 0);
                            printf("sent to server: %s\n",tt);
                        }
                    else if(p==2)
                        {
                            memset(buf,'\0',sizeof(buf));
                            strcpy(buf,"RCPT TO ");
                            strcat(buf,receivers[k]);
                            rst = send(transfer, buf, strlen(buf),0);
                            printf("sent to server: %s\n",buf);
                            memset(buf,'\0',sizeof(buf));
                            strcpy(buf,"DATA");
                            usleep(100000);
                            rst = send(transfer, buf, strlen(buf),0);
                            printf("sent to server: %s\n",buf);
                        }  
                    else if(p!=2)
                        {
                            rst = send(transfer, list[p], strlen(list[p]), 0);
                            printf("sent to server: %s\n",list[p]);
                        }
                    if(p<4)
                        receive(transfer, buf);
                }
            }
            for(p=0;p<m;p+=1)
                memset(list[p],'\0',1000);
            for(p=0;p<rsize;p+=1)
                memset(receivers[p],'\0',500);
            memset(domain , '\0', 100);
            memset(sender , '\0', 500);
            memset(data , '\0', 10000);
            isOther=0;
            rsize=0;
            m=1;
        }
        else if(strcmp(buf,"QUIT")==0 ) //content will be entered from next line onwards
        {
            if(transfer != -1 )
            {
                usleep(100000);
                rst = send(transfer, buf, strlen(buf), 0);
                receive(transfer, buf);
            }
            memset(buf,'\0',strlen(buf));
            strcpy(buf,"221 service closed");
            usleep(100000);
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
            usleep(100000);
            rst = send(cfd, buf, strlen(buf), 0);
            memset(buf,'\0',strlen(buf));
            rsize=0;
        }
    }
    if(transfer!=-1)
        close(transfer);
}

void pop3(int cfd)
{
    char buf[BUF_SIZE] = {'\0'};    
    receive(cfd,buf);
    char user[1000];
    int r,u;
    int fl=0,rst;
    do{
        if(fl!=0)
            receive_pop3(cfd,buf);
        fl=0;
    }while(!check_login_pop3(cfd,buf,user));
    receive_pop3(cfd,buf);
    while(1)
    {   
        list_pop3(cfd,buf,user,&r,&u);
        retrieve_pop3(cfd,buf,user);
        receive_pop3(cfd,buf);
        if(strcmp(buf,"QUIT")==0)
            break;
    }

}
void serviceready( int cfd )
{
    int rst;
    char buf[BUF_SIZE] = {'\0'};
    strcpy(buf,"220 service ready");
    usleep(100000);
    rst = send(cfd, buf, strlen(buf), 0);
}

void receive(int cfd, char buf[])
{
    int rst;
    memset(buf,'\0',strlen(buf));
    do
    {
        rst = recv(cfd, buf, BUF_SIZE, 0);
    }while(rst==-1 || strlen(buf)==0);
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
    usleep(100000);
    rst = send(cfd, buf, strlen(buf), 0);
}

void MAILFROM(int cfd,char buf[],char sender[])
{
    int rst;
    int i;
    for(i=0;buf[i]!=' ';i++);
    for(i+=1;buf[i]!=' ';i++);
    strcpy(sender,&buf[i+1]);
    memset(buf,'\0',strlen(buf));
    strcpy(buf,"250 OK");
    usleep(100000);
    rst = send(cfd, buf, strlen(buf), 0);
}    

void RCPTTO(int cfd,char buf[],char receivers[][500],int *rsize)
{
    int rst;
    int i;
    for(i=0;buf[i]!=' ';i++);
    for(i+=1;buf[i]!=' ';i++);
    strcpy(receivers[*rsize],&buf[i+1]);
    *rsize = *rsize + 1;
    memset(buf,'\0',strlen(buf));
    strcpy(buf,"250 OK");
    usleep(100000);
    rst = send(cfd, buf, strlen(buf), 0);
}

void DATA(int cfd,char data[],int isOther,char list[][1000],int *m)
{
    char buf[1000];
    int rst;
    strcpy(buf,"354 start mail input");
    usleep(100000);
    rst = send(cfd, buf, strlen(buf), 0);
    do{
        memset(buf,'\0',strlen(buf));
        receive(cfd,buf);
        if(isOther==1)
        {   
            memset(list,'\0',1000);
            strcpy(list[*m],buf);
            *m = *m + 1;
        }
        strcat(data,"\t");
        strcat(data,buf);
        strcat(data,"\n"); 
    }while(!(strlen(buf)==1 && buf[strlen(buf) - 1] == '.'));
}

void done(int cfd,char domain[],char sender[],char receivers[][500],int rsize,char data[])
{
    printf("domain : %s\n",domain);
    printf("sender : %s\n",sender);
    int i;
    for(i=0; i < rsize;i++)
        printf("receivers[%d] : %s\n",i,receivers[i]);
    printf("data : %s\n",data);
    char buffer[26]={'\0'};
    char buffer2[26]={'\0'};
    time_t timer;
    struct tm* tm_info;
    struct tm* tm_info2;
    time(&timer);
    tm_info = localtime(&timer);
    tm_info2 = localtime(&timer);
    strftime(buffer, 26, "%Y_%m_%d_%H_%M_%S", tm_info);
    strftime(buffer2, 26, "%Y/%m/%d %H:%M:%S", tm_info2);
    char final[10000],filename[1000];
    memset(final,'\0',sizeof(final));
    strcpy(final,"\nEmail From : ");
    strcat(final,sender);
    strcat(final,"\n\nSent on : ");
    strcat(final, buffer2);
    strcat(final,"\n\nContent :\n");
    strcat(final,data);
    FILE *fp,*lp;
    printf("beforeloop");
    for(i=0;i<rsize;i++)
    {
        if(strstr(receivers[i],mydomain)!=NULL)
        {
            memset(filename,'\0',sizeof(filename));
            strcpy(filename,receivers[i]);
            strcat(filename,buffer);
            char path[500]={"./data/"};
            strcat(path,receivers[i]);
            strcat(path,"list.txt");
            printf("path1 : %s\n",path);
            lp=fopen(path,"a");
            fprintf(lp,"uu%s\n",filename);
            fclose(lp);
            strcpy(path,"./data/");
            strcat(path,filename);
            printf("path2 : %s\n",path);
            fp = fopen(path,"w+");
            fprintf(fp,"%s\n", final);
            fclose(fp);
            printf("\nWrote : \n\n%s\n\n",final);
        }
        else
            printf("IS NULL %s,%s\n",receivers[i],mydomain);
    }
    printf("afterloop");
}


void retrieve_pop3(int cfd, char buf[],char user[])
{
    int ret=0,jj=0;
    int rs;
    memset(buf,'\0',strlen(buf));
    rs = recv(cfd, buf, BUF_SIZE, 0);
    for(jj=0;jj<strlen(buf);jj++)
        ret = 10*ret + buf[jj]-'0';
    // printf("Received : %s, %d\n",buf,ret);
    // printf("ret %d",ret);
    FILE *fp,*mp;
    char path[100]={"./data/"},l1[100],mail[10000],msize[100],temp[100],buffer[10000];
    int i=0,j=0,k=0,size,rst;
    strcat(path,user);
    strcat(path,"list.txt");
    fp = fopen(path,"r");
    memset(l1,'\0',100);
    for(i=0;i<=strlen(path);i++)
        l1[i]=path[i];
    printf("path %s\n", path);
    memset(path,'\0',100);
    strcpy(path,"./data/");
    // FILE *fp2;
    // printf("\n\n\tl1 : %s\n,\n path2 : %s\n\n\n",l1,path2);
    // fp2 = fopen(path2,"w+");
    memset(buffer,'\0',10000);
    i=0;
    while(fscanf(fp,"%s",temp) != EOF )
    {
        printf("%s\n\n",temp);
        i+=1;
        if(i==ret)
            {
                int l=0,m=strlen(temp);
                temp[0]='r';
                temp[1]='r';
                // fprintf(fp2, "%s\n",temp );
                strcat(buffer,temp);
                strcat(buffer,"\n");
                for( l=0; l <= (m-2); l++)
                {
                    temp[l]=temp[l+2];
                }
                strcat(path,temp);
                // break;
            }
        else
        {
            strcat(buffer,temp);
            strcat(buffer,"\n");
            // fprintf(fp2, "%s\n",temp );
        }
    }
    fclose(fp);
    printf("final path %s\n",path);
    fp=fopen(path,"r");
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, fp);
    fclose(fp);
    string[fsize] = '\0';
    printf("l1 : %s,buffer %s\n",l1,buffer );
    fp = fopen(l1,"w+");
    fprintf(fp, "%s",buffer );
    fclose(fp);
    usleep(100000);
    rst = send(cfd, string, strlen(string), 0);
    free(string);
}

int list_pop3(int cfd, char buf[],char user[],int *r, int *u)
{
    FILE *fp;
    char path[1000],msg_count[100],temp[4000];
    int i=0,j=0,k=0,rst;
    strcpy(path,"./data/");
    strcat(path,user);
    strcat(path,"list.txt");
    fp = fopen(path,"r");
    while(fscanf(fp,"%s",temp) != EOF )
    {
        if(temp[0]=='r' && temp[1]=='r')
            j++;
        else if(temp[0]=='u' && temp[1]=='u')
            k++;
    }
    fclose(fp);
    memset(msg_count,'\0',100);
    sprintf(msg_count,"%d ",j);
    *r = j;
    memset(temp,'\0',40);
    *u = k;
    sprintf(temp,"%d",k);
    strcat(msg_count,temp);
    usleep(100000);
    rst = send(cfd, msg_count, strlen(msg_count), 0);
    printf("sent : %s\n",msg_count);
    send_details_pop3(cfd,user,r,u);
}

void send_details_pop3(int cfd,char user[],int *r,int *u)
{
    printf("inside send_details\n");
    FILE *fp,*mp;
    char path[1000]={"./data/"},mail[1000],msize[100],temp[1000];
    int i=0,j=0,k=0,size,rst;
    strcat(path,user);
    strcat(path,"list.txt");
    printf("about to open file %s\n",path);
    fp = fopen(path,"r");
    printf("about to enter loop \n");
    while(fscanf(fp,"%s",temp) != EOF )
    {
        printf("%d\n",i++);
        printf("loop.temp : %s\n",temp);
        j=strlen(temp);
        for(i=0;i <= (j-2);i++)
            temp[i]=temp[i+2];
        memset(mail,'\0',1000);
        strcpy(mail,"./data/");
        strcat(mail,temp);
        mp = fopen(mail,"r");
        fseek(mp, 0, 2);
        size = ftell(mp);
        memset(msize,'\0',100);
        sprintf(msize,"%d",size);
        printf("send_details : %s\n",msize);
        usleep(100000);
        while(send(cfd, msize, strlen(msize), 0) == -1);
        fclose(mp);
    }
    fclose(fp);
}

int check_login_pop3(int cfd, char buf[],char usrname[])
{
    //buf : "USER name@abc.com"
    char user[1000];
    char pass[1000];
    char temp[1000];
    int flag=0,rst;
    strcpy(user,&buf[5]);
    strcpy(usrname,&buf[5]);
    memset(buf,'\0',strlen(buf));
    strcpy(buf,"OK");
    usleep(100000);
    rst = send(cfd, buf, strlen(buf), 0);
    receive_pop3(cfd,buf);
    //buf : "PASS passwords"
    strcpy(pass,buf);
    strcat(user,"??");
    strcat(user,&pass[5]);
    printf("string : %s\n",user);
    FILE *fp;
    fp = fopen("./data/logininfo.txt","r");
    while(fscanf(fp,"%s",temp) != EOF && flag==0)
    {
        printf("temp:.%s., user :.%s.",temp,user);
        if(strcmp(temp,user)==0)
            flag=1;
    }
    memset(buf,'\0',strlen(buf));
    if(flag)
        strcpy(buf,"OK");
    else
        strcpy(buf,"ERR");
    usleep(100000);
    rst = send(cfd, buf, strlen(buf), 0);
    printf("sent %s\n",buf);
    fclose(fp);
    return flag;
}

void receive_pop3(int cfd, char buf[])
{
    int rst;
    memset(buf,'\0',strlen(buf));
    rst = recv(cfd, buf, BUF_SIZE, 0);
    printf("Received : %s\n",buf);
}

int connect_to_other_server()
{
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
    srv_addr.sin_port   = htons (other_server_port); // Port Number.

    rst = inet_pton (AF_INET, ip_abc, &srv_addr.sin_addr); /* To
                              * type conversion of the pointer here. */
    if (rst <= 0)
    {
      perror ("Client Presentation to network address conversion.\n");
      exit (1);
    }
    /***************** Connect to the server ************************/
    rst = connect (sfd, (struct sockaddr *) &srv_addr, addrlen);
    return sfd;
}
