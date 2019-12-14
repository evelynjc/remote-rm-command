#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#define PORT                13000
#define HOSTLEN             256
#define F_BUFSIZ            4096
#define MAX_LISTEN_QUEUE    5
#define oops(msg)    \
    {                \
        perror(msg); \
        exit(1);     \
    }

char option = 0;
int interact = 0;
struct stat st = {0};

void sanitize(char *);
void child_handler(int signal){
    int status;
    pid_t spid;

    /* -1 - get any child process
    ** WNOHANG - parent process continues
    ** regardless of the termination of child process */
    while((spid = waitpid(-1, &status, WNOHANG)) > 0){
        printf("Child Process Wait Results \n");
        printf("================================\n");
        printf("PID         : %d\n", spid);
        printf("Exit Value  : %d\n", WEXITSTATUS(status));
        printf("Exit Stat   : %d\n", WIFEXITED(status));
    }
}
int main(int ac, char *av[])
{
    struct sockaddr_in saddr;  /* build the server's addr */
    struct hostent *hp;        /* part of the server's */
    char hostname[HOSTLEN];    /* address */
    int sock_id, sock_fd;      /* line id, file desc */
    FILE *sock_fpi, *sock_fpo; /* IN and OUT streams */
    FILE *pipe_fp;             /* use popen to run ls */
    char dirname[BUFSIZ];      /* from client */
    char command[BUFSIZ];      /* for popen() */
    char buf[BUFSIZ];
    char filebuf[F_BUFSIZ];
    int n_read;
    FILE *fp;
    //int     dirlen, c;
    int i;

    signal(SIGCHLD, (void *)child_handler);

    /* create rrmbin directory */
    if (stat("./rrmbin", &st) == -1){
        mkdir("./rrmbin", 0777);
    }
    /******************** SOCKET ************************/
    sock_id = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_id == -1)
        oops("socket");

    /********************* BIND *************************/
    bzero((void *)&saddr, sizeof(saddr)); /* clear out struct */
    gethostname(hostname, HOSTLEN);       /* where am I? */
    hp = gethostbyname(hostname);
    bcopy((void *)hp->h_addr, (void *)&saddr.sin_addr, hp->h_length);
    saddr.sin_port = htons(PORT);
    saddr.sin_family = AF_INET;
    if (bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
        oops("bind");

    /******************** LISTEN ************************/
    if (listen(sock_id, MAX_LISTEN_QUEUE) != 0){
        oops("listen");
    }
    else{
        puts("waiting for calls...");
    }

    /* main loop */
    while (1){
        /******************** ACCEPT ************************/
        sock_fd = accept(sock_id, NULL, NULL); /* wait for call */
        if (sock_fd == -1){
            oops("accept");
        }
        /*
        else{
            printf("New Client: %s\n", inet_ntoa(clientaddr.sin_addr));
        }
        */

        /* initial read */
        n_read = read(sock_fd, buf, BUFSIZ);
        printf("%s\n", buf);
        sscanf(buf, %c %d, &option, &interact);

        if(option = 'x'){
            /* read file(s) */
            for(i = 0; i < interact; i++){
                fp = open("thisfile", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                while((n_read = read(sock_fd, filebuf, F_BUFSIZ)) > 0){
                    if(write(fp, filebuf, n_read) < n_read){
                        fclose(fp);
                    }
                }
                close(fp);
            }
        }
        
        /*
        // open reading direction as buffered stream //
        if( (sock_fpi = fdopen(sock_fd, "r")) == NULL)
            oops("fdopen reading");
        if( fgets(dirname, BUFSIZ-5, sock_fpi) == NULL )
            oops("reading dirname");
        sanitize(dirname);
        

        // open writing direction as buffered stream //
        if( (sock_fpo = fdopen(sock_fd, "w")) == NULL )
            oops("fdopen writing");
        
        sprintf(command, "ls %s", dirname);
        if( (pipe_fp = popen(command, "r")) == NULL )
            oops("popen");
        
        // transfer data from ls to socket //
        while( (c = getc(pipe_fp)) != EOF)
            putc(c, sock_fpo);
        
        pclose(pipe_fp);
        fclose(sock_fpo);
        fclose(sock_fpi);
        */
    }
    close(sock_id);
    return 0;
}
/* prevent some bad command injections */
/* like "; rm *" */
void sanitize(char *str)
{
    char *src, *dest;

    for (src = dest = str; *src; src++)
    {
        if (*src == '/' || isalnum(*src))
        {
            *dest++ = *src;
        }
    }
    *dest = '\0';
}