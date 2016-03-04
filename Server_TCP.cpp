#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include "news.h"
#include <wait.h>
#include <fcntl.h>

int port_num = 23465;
#define BUF_SIZE 3000

int check_recieve(int rst){
    if(rst==-1){
        perror("Collection error");
        exit(-1);
    }
}

int check_send(int rst){
    if(rst==-1){
        perror("SENDING FAILED\n");
        exit(-1);
    }
}

int check_close(int rst){
    if(rst==-1){
        perror("CLOSING FAILED");
        exit(-1);
    }
}

int check_connect(int rst){
    if(rst==-1){
        perror("CONNECTION FAILED");
        exit(-1);
    }
}

void fill_academic(std::vector<News> &Academic){
        system("ls Academic > temp.txt");
        std::ifstream readFile("temp.txt");
        std::string line;
        int counter1=0;
        while(std::getline(readFile,line)){
            line="/home/sandeep/Desktop/Networks/Academic/"+line;
            int size1=line.size();
            std::cout<<line<<"\n";
            if(line[size1-1]=='s' && line[size1-2]=='w' && line[size1-3]=='e' && line[size1-4]=='n'){
                News khabar;
                khabar.deserialise(line);
                std::cout<<khabar.get_content()<<"\n";
                Academic.push_back(khabar);
                counter1++;
            }
        }
}

void fill_nacademic(std::vector<News> &nAcademic){
        std::ifstream readFile("temp.txt");
        std::string line;
        int counter1=0;
        system("ls Non-Academic > temp.txt");
        while(std::getline(readFile,line)){
            line="/home/sandeep/Desktop/Networks/Non-Academic/"+line;
            int size1=line.size();
            std::cout<<line<<"\n";
            if(line[size1-1]=='s' && line[size1-2]=='w' && line[size1-3]=='e' && line[size1-4]=='n'){
                News khabar;
                khabar.deserialise(line);
                std::cout<<khabar.get_content()<<"\n";
                nAcademic.push_back(khabar);
                counter1++;
            }
        } 
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s,char delim){
    std::vector<std::string> elems;
    split(s,delim,elems);
    return elems;
}

void check_access(){
    while(true){
        FILE *ftr=fopen("temp.txt","r");
        int temp;
        scanf("%d",&temp);
        printf("%d..\n",temp);
        if(temp==0){
            break;
        }
        else{
            sleep(1);
        }
    }
}

int isgreater(News khabar,std::vector<int> date){
    struct tm timek=khabar.get_time();
    if(timek.tm_year>date[0]){
        return 1;
    }
    else if(timek.tm_year<date[0]){
        return 0;
    }
    else if(timek.tm_year==date[0]){
        if(timek.tm_mon>date[1]){
            return 1;
        }
        else if(timek.tm_mon<date[1]){
            return 0;
        }
        else if(timek.tm_mon==date[1]){
            if(timek.tm_mday>=date[2]){
                return 1;
            }
            else if(timek.tm_mday<date[2]){
                return 0;
            }
        }
    }
}

int main ()
{
    pid_t child_tcp=fork();
    if(child_tcp==0)
    {
        int rst; // Return status of functions.
        int cfd; // File descriptor for the client.
        /***************DESERIALISE****************/
        std::vector<News> Academic;
        fill_academic(Academic);

        std::vector<News> nAcademic;
        fill_nacademic(nAcademic);
        /**************** Create a socket. *******************************/
        int sfd; // Socket file descriptor.
        sfd = socket (AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,
                    * SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
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
        srv_addr.sin_port   = htons (28912); // Port Number.
        
        rst = inet_pton (AF_INET, "127.50.30.11", &srv_addr.sin_addr); /* To 
                                  * type conversion of the pointer here. */
        if (rst <= 0)
        {
            perror ("Server Presentation to network address conversion.\n");
            exit (1);
        }    
        
        
        
        /****************** Bind the server to an address. ***************/
        rst = bind (sfd, (struct sockaddr *) &srv_addr, addrlen); /* Note
            * the type casting of the pointer to the server's address. */
        if (rst == -1)
        {
            perror ("Server: Bind failed");
            exit (1);
        }


        /***************** listen (): Mark the socket as passive. *******/
        for(int counter1=0;counter1>=0;counter1++){
            printf ("Max connections allowed to wait: %d\n", SOMAXCONN);
            rst = listen (sfd, 6);
            if (rst == -1)
            {
                perror ("Server: Listen failed");
                exit (1);
            }
            
            /***************** accept (): Waiting for connections ************/
            printf("waiting___waiting\n");
            cfd = accept (sfd, (struct sockaddr *) &cli_addr, &addrlen); /* 
                * Returns the file descriptor for the client. 
                * Stores the address of the client in cli_addr. */
            if (cfd == -1)
            {
                perror ("Server: Accept failed");
                exit (1);
            }
            /**********INTERACTION IO*********/
            pid_t child1=fork();
            if(child1==0){
                printf("ENTERING CHILD with counter->%d\n",counter1);//debugging
                char buf[BUF_SIZE]={'\0'};
                rst=recv(cfd,buf,BUF_SIZE,0);
                printf("Recieved from client->|%s|\n",buf);//what type of process is accessing            
                if(buf[0]=='r' && buf[1]=='e' && buf[2]=='a'){//reader
                    rst=write(cfd,"Welcome!! Please select between academic and non-academic by entering 0 and 1 respectively\n",120);
                    check_send(rst);
                    rst=recv(cfd,buf,BUF_SIZE,0);
                    check_recieve(rst);
                    Academic.clear();
                    nAcademic.clear();
                    //PUT A READ LOCK//
                    struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
                    int fd;
                    fl.l_type = F_RDLCK;
                    if ((fd = open("temp.txt", O_RDWR)) == -1)
                    {
                        perror("open");
                        exit(1);
                    }
                    if (fcntl(fd, F_SETLKW, &fl) == -1)
                    {
                        perror("fcntl");
                        exit(1);
                    }
                    printf("GOT LOCK\n");
                    //COMPLETED//
                    fill_academic(Academic);
                    fill_nacademic(nAcademic);
                    //FINISHED REMOVE THE LOCK//
                    fl.l_type = F_UNLCK;
                    if (fcntl(fd, F_SETLK, &fl) == -1)
                    {
                        perror("fctl");
                        exit(1);
                    }
                    printf("unlocked");
                    //COMPLETED
                    sort(Academic.begin(),Academic.end());
                    sort(nAcademic.begin(),nAcademic.end());
                    reverse(Academic.begin(),Academic.end());
                    reverse(nAcademic.begin(),nAcademic.end());
                    if(buf[0]=='0'){
                        rst=write(cfd,"You selected academic",23);
                        check_send(rst);
                        std::string s_string;
                        for(int index=0;index<Academic.size();index++){
                            s_string=s_string+std::to_string(index+1)+"{";
                            s_string=s_string+Academic[index].get_title();
                            if(index!=(int)Academic.size()-1){
                                s_string=s_string+"{";
                            }
                        }
                        std::cout<<"sending string "<<s_string<<"\n";
                        strcpy(buf,s_string.c_str());
                        rst=write(cfd,buf,300000);
                        check_send(rst);
                        rst=recv(cfd,buf,BUF_SIZE,0);
                        check_recieve(rst);
                        std::string indexs=std::string(buf);
                        int index_selected;
                        std::istringstream ss1(indexs);
                        ss1>>index_selected;
                        printf("CLIENT selected :%d\n",index_selected);
                        //strcpy(buf,Academic[index_selected].get_content().c_str());
                        //checking here
                        std::string send_s=Academic[index_selected].get_content();
                        std::cout<<"sending-->"<<send_s<<"\n";
                        //sending string as packets
                        int lens=send_s.size();
                        int begin1=0;
                        while(lens>0){
                            int tempnum=-1;
                            if(255>lens){
                                tempnum=lens;
                            }
                            else{
                                tempnum=256;
                            }
                            std::string temp1=send_s.substr(begin1,tempnum);
                            temp1=temp1+"&";
                            strcpy(buf,temp1.c_str());
                            printf("sending->>>> %s\n",buf);
                            rst=write(cfd,buf,256);
                            check_send(rst);
                            lens-=tempnum;
                            begin1+=tempnum;
                        }
                        if(lens==0){
                            std::string temp1="{";
                            strcpy(buf,temp1.c_str());
                            rst=write(cfd,buf,temp1.size());
                            check_send(rst);
                        }
                        //ending here
                        //rst=write(cfd,buf,BUF_SIZE);
                        //check_send(rst);
                    }
                    else if(buf[0]=='1'){
                        rst=write(cfd,"You selected non-academic",27);
                        check_send(rst);
                        std::string s_string;
                        for(int index=0;index<nAcademic.size();index++){
                            s_string=s_string+std::to_string(index+1)+"{";
                            s_string=s_string+nAcademic[index].get_title();
                            if(index!=(int)nAcademic.size()-1){
                                s_string=s_string+"{";
                            }
                        }
                        std::cout<<"sending string "<<s_string<<"\n";
                        strcpy(buf,s_string.c_str());
                        rst=write(cfd,buf,100000);
                        check_send(rst);
                        rst=recv(cfd,buf,BUF_SIZE,0);
                        check_recieve(rst);
                        std::string indexs=std::string(buf);
                        int index_selected;
                        std::istringstream ss1(indexs);
                        ss1>>index_selected;
                        std::string send_s=nAcademic[index_selected].get_content();
                        std::cout<<"sending-->"<<send_s<<"\n";
                        //sending string as packets
                        int lens=send_s.size();
                        int begin1=0;
                        while(lens>0){
                            int tempnum=-1;
                            if(255>lens){
                                tempnum=lens;
                            }
                            else{
                                tempnum=256;
                            }
                            std::string temp1=send_s.substr(begin1,tempnum);
                            temp1=temp1+"&";
                            strcpy(buf,temp1.c_str());
                            printf("sending->>>> %s\n",buf);
                            rst=write(cfd,buf,256);
                            check_send(rst);
                            lens-=tempnum;
                            begin1+=tempnum;
                        }
                        if(lens==0){
                            std::string temp1="{";
                            strcpy(buf,temp1.c_str());
                            rst=write(cfd,buf,temp1.size());
                            check_send(rst);
                        }
                    }
                }
                else if(buf[0]=='r' && buf[1]=='p'){//reporter
                    printf("ENTERING CHILD with counter->%d\n",counter1);//debugging
                    rst=write(cfd,"Welcome Reporter!! Please select between academic and non-academic",120);
                    check_send(rst);
                    rst=recv(cfd,buf,BUF_SIZE,0);
                    check_recieve(rst);
                    if(buf[0]=='0'){
                        rst=write(cfd,"You selected academic",23);
                        check_send(rst);
                        rst=recv(cfd,buf,BUF_SIZE,0);
                        check_recieve(rst);
                        printf("%s\n",buf);
                        std::string buffer(buf);
                        std::vector<std::string> splitb=split(buffer,',');
                        News khabar;
                        std::istringstream ss1(splitb[0]);
                        int id_;
                        ss1>>id_;
                        khabar.set_ID(id_);
                        khabar.set_title(splitb[1]);
                        khabar.set_content(splitb[2]);
                        khabar.set_time_now();
                        //PUT A WRITE LOCK//
                        khabar.serialise_a();
                        struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
                        int fd;
                        if ((fd = open("temp.txt", O_RDWR)) == -1)
                        {
                            perror("open");
                            exit(1);
                        }
                        if (fcntl(fd, F_SETLKW, &fl) == -1)
                        {
                            perror("fcntl");
                            exit(1);
                        }
                        printf("GOT LOCK\n");
                        //FINISH//
                        //khabar.serialise_a();
                        //REMOVE WRITE LOCK
                        fl.l_type = F_UNLCK;
                        if (fcntl(fd, F_SETLK, &fl) == -1)
                        {
                            perror("fctl");
                            exit(1);
                        }
                        printf("unlocked");
                        //COMPLETED
                        printf("DONE!!\n");
                        rst=write(cfd,"ADDED WITHOUT ERRORS",150);
                        check_send(rst);
                        //qwe now here the data recieved has to be again parsed into a .news item
                    }
                    else{
                        rst=write(cfd,"You selected non-academic",34);
                        check_send(rst);
                        rst=recv(cfd,buf,BUF_SIZE,0);
                        check_recieve(rst);
                        printf("%s\n",buf);
                        std::string buffer(buf);
                        std::vector<std::string> splitb=split(buffer,',');
                        News khabar;
                        std::istringstream ss1(splitb[0]);
                        int id_;
                        ss1>>id_;
                        khabar.set_ID(id_);
                        khabar.set_title(splitb[1]);
                        khabar.set_content(splitb[2]);
                        khabar.set_time_now();
                        //PUT A WRITE LOCK//
                        khabar.serialise_na();
                        struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
                        int fd;
                        if ((fd = open("temp.txt", O_RDWR)) == -1)
                        {
                            perror("open");
                            exit(1);
                        }
                        if (fcntl(fd, F_SETLKW, &fl) == -1)
                        {
                            perror("fcntl");
                            exit(1);
                        }
                        printf("GOT LOCK\n");
                        //FINISH//
                        //khabar.serialise_na();
                        //REMOVE WRITE LOCK
                        fl.l_type = F_UNLCK;
                        if (fcntl(fd, F_SETLK, &fl) == -1)
                        {
                            perror("fctl");
                            exit(1);
                        }
                        printf("unlocked");
                        //COMPLETED
                        printf("DONE!!\n");
                        rst=write(cfd,"ADDED WITHOUT ERRORS",150);
                        check_send(rst);
                        //qwe now here the data recieved has to be parsed into a .news item and saved properly
                    }
                }
                printf("Exiting child with counter->%d\n",counter1);
                return 0;
            }
            //printf("PARENT\n");
        }   

        rst = close (sfd); // Close the socket file descriptor.
        check_close(rst);
    }
    else
    {
        pid_t child_udp=fork();
        if(child_udp==0)
        {
            int rst; // Return status of functions.
            /***************** Create a socket *******************************/
            int sfd; // Socket file descriptor.
            sfd = socket (AF_INET, SOCK_DGRAM, 0); 
            check_connect(sfd);
            printf ("Socket_ufd fd = %d\n", sfd);
            /***************** Binding the server to an address. *************/
            struct sockaddr_in srv_addr, cl_addr; // For sever and client addresses.
            socklen_t addrlen = sizeof (struct sockaddr_in);
            
            
            /* Clear the two addresses. */
            memset (&srv_addr, 0, addrlen);
            memset (&cl_addr, 0, addrlen);
            
            /* Assign a server address. */
            srv_addr.sin_family = AF_INET; // IPv4
            srv_addr.sin_port   = htons (24021);
            /* The servers IP address. */
            rst = inet_pton (AF_INET, "127.0.0.1", &srv_addr.sin_addr);
            if (rst <= 0)
            {
                perror ("Server Presentation to network address conversion.\n");
                exit (1);
            }
            rst = bind (sfd, (struct sockaddr *) &srv_addr, addrlen);
            if (rst < 0)
            {
                perror ("Server: bind failed");
                exit (1);
            }
            /****************** Send - receive messages **********************/
            for(int counter1=0;counter1>=0;counter1++){
                /* Buffer for receiving a message. */
                char buf[BUF_SIZE] = {'\0'};
                int b_recv   = 0; // Number of bytes received
                int flags = 0;


                //recieved password
                b_recv=recvfrom(sfd,buf,BUF_SIZE,flags,(struct sockaddr *)&cl_addr,&addrlen);
                check_recieve(b_recv);
                const char *msg;
                if(buf[0]=='a' && buf[1]=='a'){
                    msg="1";
                }
                else{
                    msg="0";
                }
                //done

                //authenticate
                rst=sendto(sfd,msg,sizeof(msg),flags,(struct sockaddr *)&cl_addr,sizeof(struct sockaddr_in));
                check_send(rst);
                //if not authenticated
                if(msg[0]=='0'){
                    continue;
                }

                //if authenticated
                b_recv=recvfrom(sfd,buf,BUF_SIZE,flags,(struct sockaddr *)&cl_addr,&addrlen);
                check_recieve(b_recv);
                //DATE PARSING BEGINS
                printf("date recieved->%s\n",buf);
                std::string buffer_(buf);
                std::vector<std::string> date_=split(buffer_,'-');
                std::vector<int> date_integer;
                for(int i=0;i<date_.size();i++){
                    int temp;
                    std::istringstream ss(date_[i]);
                    ss>>temp;
                    printf("%d----\n",temp);
                    date_integer.push_back(temp);
                }
                //GOT THE DATE IN THE VECTOR
                std::vector<News> Academic;
                std::vector<News> nAcademic;
                //PUT A READ LOCK//
                struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
                int fd;
                fl.l_type = F_RDLCK;
                if ((fd = open("temp.txt", O_RDWR)) == -1)
                {
                    perror("open");
                    exit(1);
                }
                if (fcntl(fd, F_SETLKW, &fl) == -1)
                {
                    perror("fcntl");
                    exit(1);
                }
                printf("GOT LOCK\n");
                //COMPLETED//
                Academic.clear();
                nAcademic.clear();
                fill_academic(Academic);
                fill_nacademic(nAcademic);
                //FINISHED REMOVE THE LOCK//
                fl.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &fl) == -1)
                {
                    perror("fctl");
                    exit(1);
                }
                printf("unlocked");
                //COMPLETED
                //REMOVING THE ENTRIES AS REQUIRED
                std::vector<News> temp_a;
                std::vector<News> temp_na;
                for(int i=0;i<Academic.size();i++){
                    if(isgreater(Academic[i],date_integer)){
                        temp_a.push_back(Academic[i]);
                        std::cout<<"hereh"<<Academic[i].get_title()<<"\n";
                    }
                }
                for(int i=0;i<nAcademic.size();i++){
                    if(isgreater(nAcademic[i],date_integer)){
                        temp_na.push_back(nAcademic[i]);
                    }
                }
                //DONE
                system("exec rm -r /home/sandeep/Desktop/Networks/Academic/*.news");
                system("exec rm -r /home/sandeep/Desktop/Networks/Non-Academic/*.news");
                for(int i=0;i<temp_a.size();i++){
                    temp_a[i].serialise_a();
                }
                for(int i=0;i<temp_na.size();i++){
                    temp_na[i].serialise_na();
                }
                const char *buf2="SUCCESS";
                rst=sendto(sfd,buf2,sizeof(buf2),flags,(struct sockaddr *)&cl_addr,sizeof(struct sockaddr_in));
                check_send(rst);

                //ADMINISTRATOR REMOVES LOCK
                fl.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &fl) == -1)
                {
                    perror("fctl");
                    exit(1);
                }
                printf("unlocked");
                //LOCK REMOVED
            }

            /****************** Close ****************************************/
            rst = close (sfd); // Close the socket file descriptor.
            if (rst < 0)
            {
                perror ("Server close failed");
                exit (1);
            }
        }
        else
        {
            int status_tcp=0;
            int status_udp=0;
            waitpid(child_tcp,&status_tcp,0);
            waitpid(child_udp,&status_tcp,0);
            printf("DONE\n");
        }
    }
    
return 0;}