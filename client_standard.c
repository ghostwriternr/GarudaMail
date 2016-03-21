#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>   // For the socket (), bind () etc. functions.
#include <netinet/in.h>  // For IPv4 data struct..
#include <string.h> 	// For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num ;	//send email port num
int port_num_xyz = 21001;
int	port_pop_xyz = 22001;
int port_num_garudaserver = 25;
int port_pop_garudaserver = 110;
int port_pop;

#define BUF_SIZE 100000

char ip_xyz[20] = {"10.117.11.124"}; //10.117.11.124"};
char ip_garudaserver[20] = {"10.5.30.131"}; //10.117.11.106"};
char ip[20];
char givendomain[100];
int first = 1;

int interact1(int sfd, char S[], char C[]);
int check(int cfd, char user1[]);
void receive(int sfd, char buf[]);
void interact(int sfd, char S[]);
void DATA(int sfd);
void send_mail(int sfd);
int	 mailfrom_helo(int sfd, char S[]);
void retrieve_mail(int sfd);
void set(char buf[], int *r, int *u);
void set1(char buf[], int*r);

int main (int argc, char *argv[])
{
	int i, b, kk, mm;
	char cha;
	if (argc == 1)
	{
		printf("> How many accounts do you have ?\n>");
		scanf("%d", &kk);
		for (mm = 1; mm < kk; mm++)
		{
			b = fork();
			if (b == 0)
			{
				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", "./c c", (void*)NULL);
			}
		}
	}
	printf("> Do you want to send emails(1) or retrieve emails(2) ?\n> ");
	scanf("%d", &i);
	printf("Domain Name?\n> ");
	scanf("%s", givendomain);
	if (strcmp(givendomain, "xyz.com") == 0)
	{
		strcpy(ip, ip_xyz);
		if (i == 2)
			port_num = port_pop_xyz;
		else
			port_num = port_num_xyz;
	}
	else if (strcmp(givendomain, "garudaserver.com") == 0)
	{
		strcpy(ip, ip_garudaserver);
		if (i == 2)
			port_num = port_pop_garudaserver;
		else
			port_num = port_num_garudaserver;
	}
	else
	{
		printf("Incorrect Domain Name!\nAborting the execution!\n");
		return 1;
	}
	char buf[100000] = {"\0"};
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
	if (i == 1)
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
	// if (first == 1)
	// {
	int rst;
	char buf[BUF_SIZE] = {'\0'};
	memset(buf, '\0', strlen(buf));
	rst = recv(sfd, buf, BUF_SIZE, 0);
	printf("%s\n", buf);
	char helostr[100] = {"HELO "};
	int i, j, n = strlen(S);
	for (i = 1; S[i - 1] != '@' && i < n; i++);
	if (S[i - 1] != '@')
		return 0;
	for (j = strlen(helostr); i < n; i++, j++)
	{
		helostr[j] = S[i];
	}
	helostr[j] = '\0';
	strcat(helostr, "\r\n");
	interact(sfd, helostr);
	// first = 0;
	// }
	char mailstr[1000] = { "MAIL FROM:" };
	strcat(mailstr, "<");
	strcat(mailstr, S);
	strcat(mailstr, ">");
	strcat(mailstr, "\r\n");
	interact(sfd, mailstr);
	return 1;
}

void send_mail(int sfd)
{
	char S[100000], temp[1000], C[1000];
	int i, fl, rst;
	while (1)
	{
		memset(S, '\0', strlen(S));
		do {
			printf("> Mail from?\n> ");
			scanf("%s", S);
			if (strstr(S, givendomain) == NULL)
				printf("> Error : Domain name and Email mismatch!\n> ");
		} while (strstr(S, givendomain) == NULL || mailfrom_helo(sfd, S) != 1);
		printf("> Number of receipants?\n> ");
		scanf("%d", &i);
		while (i)
		{
			memset(temp, '\0', strlen(temp));
			strcpy(temp, "RCPT TO:");
			printf("> Mail to?\n> ");
			scanf("%s", S);
			strcat(temp, "<");
			strcat(temp, S);
			strcat(temp, ">");
			if (interact1(sfd, temp, S) == 0)
			{
				printf("> No such user exists!\n> ");
				continue;
			}
			usleep(100000);
			i -= 1;
		}
		printf("> Enter mail body [end with a single dot(.)]\n> ");
		DATA(sfd);
		printf("> Mail sent successfully!\n> ");
		printf("> Bye!\n");
		memset(S, '\0', strlen(S));
		strcpy(S, "QUIT");
		strcat(S, "\r\n");
		interact(sfd, S);
		exit(1);
	}
}

void interact(int sfd, char S[])
{
	int rst;
	printf("%s\n", S);
	char buf[BUF_SIZE] = {'\0'};
	strcpy(buf, S);
	usleep(100000);
	rst = send(sfd, buf, strlen(buf), 0);
	memset(buf, '\0', strlen(buf));
	rst = recv(sfd, buf, BUF_SIZE, 0);
	// printf("%s\n", buf);
}

void DATA(int sfd)
{
	int i, rst;
	char ch, S[100000] = {"DATA"};
	strcat(S, "\r\n");
	interact(sfd, S);
	while (1)
	{
		memset(S, '\0', strlen(S));
		i = 0;
		while (1)
		{
			ch = getchar();
			if (ch == '\n')
				break;
			S[i++] = ch;
		}
		S[i] = '\0';
		printf("> ");
		usleep(100000);
		if (i == 1 && S[0] == '.')
		{
			char buf[BUF_SIZE] = {'\0'};
			strcpy(buf, S);
			strcat(buf, "\r\n");
			rst = send(sfd, buf, strlen(buf), 0);
			memset(buf, '\0', strlen(buf));
			rst = recv(sfd, buf, BUF_SIZE, 0);
			printf("%s\n", buf);
			break;
		}
		else
		{
			strcat(S, "\r\n");
			rst = send(sfd, S, strlen(S), 0);
		}
	}
}

void retrieve_mail(int sfd)
{
	char buf[100000], temp[100000], *ptr;
	int i, j, k, l, r, u, rst;
	do {
		do {
			printf("> username?\n> ");
			memset(buf, '\0', 100000);
			memset(temp, '\0', 100000);
			strcpy(buf, "USER ");
			scanf("%s", temp);
			if (strstr(temp, givendomain) == NULL)
			{
				printf("> Error : User Name and Domain mismatch!\n> ");
			}
			else
			{
				break;
			}
		} while (1);
		strcat(buf, temp);
		strcat(buf, "\r\n");
		usleep(100000);
		rst = send(sfd, buf, strlen(buf), 0);
		receive(sfd, buf);
		printf("%s\n", buf);
		receive(sfd, buf);
		printf("%s\n", buf);
		memset(buf, '\0', 100000);
		memset(temp, '\0', 100000);
		strcpy(buf, "PASS ");
		ptr = getpass("> Password?\n> ");
		strcat(buf, ptr);
		strcat(buf, "\r\n");
		usleep(100000);
		rst = send(sfd, buf, strlen(buf), 0);
		memset(buf, '\0', 100000);
		receive(sfd, buf);
		printf("%s\n", buf);
		if (strstr(buf, "ERR") != NULL)
		{
			printf("> Error : Wrong Credentials!\n> ");
		}
		else
			break;
	} while (1);
	while (1)
	{
		memset(buf, '\0', 100000);
		strcpy(buf, "LIST");
		strcat(buf, "\r\n");
		usleep(100000);
		rst = send(sfd, buf, strlen(buf), 0);
		receive(sfd, buf);
		printf("%s\n", buf);
		set1(buf, &r);
		if (r != 0)
		{
			do {
				printf("Which one do you want to read?\n> ");
				scanf("%d", &i);
			} while (i > r);
			memset(buf, '\0', 100000);
			strcpy(buf, "RETR ");
			char buf1[5] = {'\0'};
			sprintf(buf1, "%d", i);
			strcat(buf, buf1);
			strcat(buf, "\r\n");
			usleep(100000);
			printf("%s\n", buf);
			rst = send(sfd, buf, strlen(buf), 0);
			int r = 0;
			while (1)
			{
				r = 0;
				memset(buf, '\0', strlen(buf));
				rst = recv(sfd, buf, BUF_SIZE, 0);
				while (r < sizeof(buf) - 1)
				{
					if (buf[r] != 0)
						printf("%c", buf[r]);
					r++;
				}
				if (rst < BUF_SIZE)
					break;
			}
			printf("> Do you want to read more ? (Y) Yes (N) NO\n> ");
		}
		char ch[2];
		if ((r + u) != 0)
			scanf("%s", ch);
		if (ch[0] != 'Y' || (r + u) == 0)
		{
			memset(buf, '\0', 100000);
			strcpy(buf, "QUIT");
			strcat(buf, "\r\n");
			usleep(100000);
			rst = send(sfd, buf, strlen(buf), 0);
			receive(sfd, buf);
			printf("%s\n", buf);
			exit(1);
		}
	}
}

void set(char buf[], int *r, int *u)
{
	int j, k = 0, n = strlen(buf);
	for (j = 0; buf[j] != ' '; j++)
	{
		k = 10 * k + buf[j] - '0';
	}
	*r = k;
	for (j = j + 1, k = 0; j < n; j++)
	{
		k = 10 * k + buf[j] - '0';
	}
	*u = k;
}

void set1(char buf[], int* r)
{
	int j = 0, k, n = strlen(buf);
	for (k = 0; k < n; k++)
	{
		if (buf[k] == '\n')
			j++;
	}
	*r = j - 2;
}

void receive(int sfd, char buf[])
{
	int rst;
	do {
		memset(buf, '\0', strlen(buf));
		rst = recv(sfd, buf, BUF_SIZE, 0);
	} while (strlen(buf) == 0);
	// printf("Received : %s\n",buf);
}

int check(int cfd, char user1[])
{
	//buf : "USER name@garudaserver.com"
	char temp[1000];
	char user[1000] = {'\0'};
	strcpy(user, user1);
	int flag = 0, rst;
	strcat(user, "??");
	// printf("string : %s\n",user);
	FILE *fp;
	fp = fopen("./data/logininfo.txt", "r");
	while (fscanf(fp, "%s", temp) != EOF && flag == 0)
	{
		// printf("temp:.%s., user :.%s.",temp,user);
		if (strstr(temp, user) != NULL)
			flag = 1;
	}
	fclose(fp);
	return flag;
}

int interact1(int sfd, char S[], char C[])
{
	// if (check(sfd, C) == 0)
	// 	return 1;
	int rst;
	char buf[BUF_SIZE] = {'\0'};
	strcpy(buf, S);
	strcat(buf, "\r\n");
	usleep(100000);
	rst = send(sfd, buf, strlen(buf), 0);
	memset(buf, '\0', strlen(buf));
	rst = recv(sfd, buf, BUF_SIZE, 0);
	printf("%s\n", buf);
	return 1;
}
