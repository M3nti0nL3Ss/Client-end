#include<sys/socket.h>
#include<sys/types.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdarg.h> //variadic function: va_*
#include<errno.h>
#include<fcntl.h>
#include<sys/time.h>
#include<sys/ioctl.h>
#include<netdb.h>

#define SERVER_PORT 80 // Default port

#define MAXLINE 4096 // 4 MB
#define SA struct sockaddr
#define IP_SIZE 16 // 16 = size of XXX.XXX.XXX.XXX + 0 wich is the maximium amount of data

void err_n_die(const char *fmt, ...);
int isValidIpAddress(char *ipAddress);
void hostname_to_ip(char*,char*);

int main(int argc, char** argv){
    int sockfd, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    char ip[IP_SIZE];
    
    if(argc !=2)
        err_n_die("usage: %s <server addr>", argv[0]);
    
    //getting the host from the url
    char* rest;
    if(strstr(argv[1],"/")){
        char* tmp=strtok(argv[1],"/");
        rest=strtok(NULL,"");
        argv[1]=tmp;
    }
    
    //create a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("Error while creating the socket!");
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT); // chat serv
    
    if(!isValidIpAddress(argv[1])){
        hostname_to_ip(argv[1], ip);
        //fprintf(stdout,"%s\n%s\n", argv[1],rest);//just for debugging
    }else strcpy(ip,argv[1]);
    
    if(inet_pton(AF_INET, ip, &servaddr.sin_addr)<=0)
        err_n_die("inet_pton error for %s",ip);
    
    if(connect(sockfd,(SA *) &servaddr, sizeof(servaddr)) <0)
        err_n_die("Connection faild!");
    
    char message[100];
    strcat(message,"GET /");
    if(rest)
        strcat(message,rest);
    strcat(message," HTTP/1.1\r\n\r\n");
    //we're connected. Prepare the message
    sprintf(sendline, message);// Getting the root directory
    sendbytes = strlen(sendline);
    
    //write to socket
    if(write(sockfd, sendline, sendbytes) != sendbytes)
        err_n_die("write error");
    
    //init recvline
    memset(recvline, 0, MAXLINE);
    
    
    while((n = read(sockfd, recvline, MAXLINE-1))>0){
        printf("%s", recvline);
    }
    if(n<0)
        err_n_die("read error");
}

void err_n_die(const char *fmt, ...){
    int errno_save;
    va_list ap;
    
    errno_save = errno;
    
    va_start(ap, fmt);
    vfprintf(stdout, fmt,ap);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    if(errno_save != 0){
        fprintf(stdout, "(errno = %d) : %s\n",errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    va_end(ap);
    
    exit(1);
}

int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

void hostname_to_ip(char *hostname , char* ip){
    struct hostent *he;
    struct in_addr **addr_list;
    if (!(he = gethostbyname(hostname)))
        // get the host info
        herror("gethostbyname");
    addr_list = (struct in_addr **) he->h_addr_list;
    strcpy(ip , inet_ntoa(*addr_list[0]));
}
