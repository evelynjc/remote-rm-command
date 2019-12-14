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

#define PORT 13000
#define HOSTLEN 256
#define F_BUFSIZ 4096
#define MAX_LISTEN_QUEUE 5
#define oops(msg)           \
    {                       \
        perror(msg);        \
        printf("\n");       \
        exit(1);            \
    }

char option = 0;
int interact = 0;
struct stat st = {0};

void sanitize(char *);

int main(int ac, char *av[])
{
    struct sockaddr_in saddr; /* build the server's addr */
    struct hostent *hp;        /* part of the server's */
    char hostname[HOSTLEN];    /* address */
    int sock_id, sock_fd;      /* line id, file desc */
    FILE *sock_fpi, *sock_fpo; /* IN and OUT streams */
    FILE *pipe_fp;             /* use popen to run ls */
    char buf[BUFSIZ];
    char filebuf[F_BUFSIZ];
    int n_read, c,;
    FILE *fp;
    int c;
    int i;

    /* create rrmbin directory */
    if (stat("./rrmbin", &st) == -1)
    {
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
    if (listen(sock_id, MAX_LISTEN_QUEUE) != 0)
    {
        oops("listen");
    }
    else
    {
        puts("waiting for calls...");
    }

    /* main loop */
    while (1){
        /******************** ACCEPT ************************/
        sock_fd = accept(sock_id, NULL, NULL); /* wait for call */
        if (sock_fd == -1){
            oops("accept");
        }
        else{
            printf("New Client!\n");
        }

        /* initial read */
        memset(buf, 0, BUFSIZ);
        n_read = read(sock_fd, buf, BUFSIZ);
        printf("%s\n", buf);
        sscanf(buf, "%c %d", &option, &interact);
        //printf("option: %c, iterations: %d\n", option, interact);

        if (option == 'x'){
            // removing file(s)
                    //     /* read file(s) */
        //     for(i = 0; i < interact; i++){
        //         fp = fopen(thisfile, "w+");
        //         while((n_read = read(sock_fd, filebuf, F_BUFSIZ)) > 0){
        //             if(write(fp, filebuf, n_read) < n_read){
        //                 fclose(fp);
        //             }
        //         }
        //         close(fp);
        //     }
        // }
            
        }
        else if (option == 'v'){
            memset(buf, 0, BUFSIZ);
            // list files in 'rrmbin'
            printf("option v!!");
            if((sock_fpi = fdopen(sock_fd, "r")) == NULL)
                oops("fdopen reading");
            if((sock_fpo = fdopen(sock_fd, "w"))== NULL)
                oops("fdopen writing");
            if((pipe_fp = popen("ls -la rrmbin", "r")) == NULL)
                oops("popen");
            
            // data from ls -la to socket
            while((c = getc(pipe_fp)) != EOF)
                putc(c, sock_fpo);
            pclose(pipe_fp);
            fclose(sock_fpo);
            close(sock_id);
            
        }
        else if (option == 'r'){
            // restore file(s)
        }
        else{
            oops("wrong option");
        }
        close(sock_fd);
    
    return 0;
}
/* prevent some bad command injections */
/* like "; rm *" */
void sanitize(char *str){
    char *src, *dest;

    for (src = dest = str; *src; src++){
        if (*src == '/' || isalnum(*src)){
            *dest++ = *src;
        }
    }
    *dest = '\0';
}