#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num = 23465;
#define BUF_SIZE 300000

using namespace std;

int check_recieve(int rst)
{
    if (rst == -1)
    {
        perror("Collection error");
        exit(-1);
    }
}

int check_send(int rst)
{
    if (rst == -1)
    {
        perror("SENDING FAILED\n");
        exit(-1);
    }
}

int check_close(int rst)
{
    if (rst == -1)
    {
        perror("CLOSING FAILED");
        exit(-1);
    }
    else
    {
        //printf("CONNECTION CLOSED\n");
    }
}

int check_connect(int rst)
{
    if (rst == -1)
    {
        printf("Client: Connect failed\\n");
        exit(1);
    }
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

int main ()
{
    //INITIAL PART...MORE LIKE A BLACK-BOX ;)
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
    srv_addr.sin_port   = htons (28912); // Port Number.

    rst = inet_pton (AF_INET, "127.50.30.11", &srv_addr.sin_addr); /* To
                                  * type conversion of the pointer here. */
    if (rst <= 0)
    {
        perror ("Client Presentation to network address conversion.\n");
        exit (1);
    }
    //INITIAL PART NOT THAT IMPORTANT


    /***************** Connect to the server ************************/
    rst = connect (sfd, (struct sockaddr *) &srv_addr, addrlen);
    check_connect(rst);
    printf("CONNECTION SUCCESSFUL!!\n");
    printf("Welcome READER\n");

    /**************** Send-Receive messages ************************/
    while (true)
    {
        //tells server that reader is accessing
        char buf[BUF_SIZE] = {'\0'};
        rst = write(sfd, "rea", 4);
        check_send(rst);
        rst = recv (sfd, buf, BUF_SIZE, 0);
        check_recieve(rst);
        printf ("%s\n", buf);
        //server pinging done

        //Selects the type of news to present
        int type;
        printf(">");
        scanf("%d", &type);
        buf[0] = '0' + type;
        rst = write(sfd, buf, 2);
        rst = recv(sfd, buf, BUF_SIZE, 0);
        printf("%s\n", buf);
        //selection done

        //data recieved
        rst = recv(sfd, buf, BUF_SIZE, 0);
        check_recieve(rst);
        std::string list_of_all(buf);
        std::vector<std::string> List_split = split(list_of_all, '{');
        for (int i = 0; i < List_split.size() - 1; i = i + 2)
        {
            std::cout << List_split[i] << ". " << List_split[i + 1] << "\n";
        }
        printf("Please select the ID of the news you want to be displayed-\n");
        int listid;
        printf(">");
        scanf("%d", &listid);
        listid--;
        std::string listids = std::to_string(listid);
        strcpy(buf, listids.c_str());
        rst = write(sfd, buf, 300000);
        check_send(rst);
        //data sub-category selection done

        //display the contents
        //rst=recv(sfd,buf,BUF_SIZE,0);
        //check_recieve(rst);
        FILE * fp = fopen("/home/sandeep/.content","w");
        printf("CONTENTS-\n");
        while (true)
        {
            //printf("here\n");
            rst = recv(sfd, buf, 256, 0);
            //printf("here too\n");
            check_recieve(rst);
            if (buf[0] == '{')      // A { is sent at the end of all packets
            {
                break;
            }
            else
            {
                //printf("recieved->");
                for (int l = 0; buf[l] != '&' && l <= 256; l++)
                {
                    if(buf[l]==NULL){
                        continue;
                    }
                    if (buf[l] == '&')
                    {
                        break;
                    }
                    printf("%c",buf[l]);
                    fprintf(fp, "%c", buf[l]);
                }
            }
        }
        fflush(fp);
        //done
        char *args[] = {"xterm","-hold","-e","less","/home/sandeep/.content","&", (char *)0};
        pid_t cpid = fork();
        if (cpid == 0) {
            execvp("xterm", args);
        }

        //close socket
        rst = close(sfd);
        check_close(rst);
        //done

        printf("Do you wish to continue Y or n\n");
        char s[2];
        printf(">");
        scanf("%s", s);
        if (s[0] == 'Y')
        {
            printf("Connecting you again...Hold on\n");
            {
                sfd = socket (AF_INET, SOCK_STREAM, 0);
                addrlen = sizeof (struct sockaddr_in);
                memset (&srv_addr, 0, addrlen);
                srv_addr.sin_family = AF_INET; // IPv4.
                srv_addr.sin_port   = htons (28912); // Port Number.
                rst = inet_pton (AF_INET, "127.50.30.11", &srv_addr.sin_addr);
                rst = connect (sfd, (struct sockaddr *) &srv_addr, addrlen);
                check_connect(rst);
            }
            //Connecting again
        }
        else if (s[0] == 'n')
        {
            printf("Thank you\n");
            return 0;
        }
    }
}
