/************** rrm_serv.c **************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "rrm.h"

char option = 0;
int interact = 0;
struct stat st = {0};

int main(int ac, char *av[]){
    struct sockaddr_in saddr, caddr; /* build the server's addr */
    socklen_t caddrlen;
    struct hostent *hp;        /* part of the server's */
    char hostname[HOSTLEN];    /* address */
    int sock_id, sock_fd;      /* line id, file desc */
    char clntaddr[INET_ADDRSTRLEN];
    FILE *sock_fpi, *sock_fpo; /* IN and OUT streams */
    FILE *pipe_fp;             /* use popen to run ls */
    FILE *fp;
    char buf[BUFSIZ];
    char filename[BUFSIZ] ="rrmbin/", tempfname[BUFSIZ];
    int c, i, n_read;
    ssize_t fsize;
    
    if(!n_read){}
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
        sock_fd = accept(sock_id, (struct sockaddr *) &caddr, &caddrlen); /* wait for call */
        if (sock_fd == -1){
            oops("accept");
        }
        else{
            caddrlen = sizeof(caddr);
            getpeername(sock_fd, (struct sockaddr *)&caddr, &caddrlen);
            inet_ntop(AF_INET, &caddr.sin_addr, clntaddr, sizeof(clntaddr));
            printf("New Client: %s\n", clntaddr);
        }

        /* initial read */
        memset(buf, 0, BUFSIZ);
        n_read = read(sock_fd, buf, BUFSIZ);
        sscanf(buf, "%c %d", &option, &interact);
        printf("option: %c, file(s) to be transferred of restored: %d\n\n", option, interact);

        if (option == 'x'){
            // file(s) from client to the rrmbin
            /* read file(s) */
            for(i = 0; i < interact; i++){
                // receive filename
                memset(buf, 0, BUFSIZ);
                n_read = read(sock_fd, buf, BUFSIZ);
                strcpy(tempfname, buf);
                strcat(filename, tempfname);
                //printf("temp fname: %s, filename: %s\n", tempfname, filename);
                // receive file and save into rrmbin
                fp = fopen(filename, "wb");
                fsize = recv_file(sock_fd, fp);
                printf("client: %s, file: %s, file size received: %lu\n", clntaddr, tempfname, fsize);
                fclose(fp);
                strcpy(filename,"rrmbin/");
            }
            close(sock_fd); 
        }
        else if (option == 'v'){
            memset(buf, 0, BUFSIZ);
            // list files in 'rrmbin'
            if((sock_fpi = fdopen(sock_fd, "r")) == NULL)
                oops("fdopen reading");
            if((sock_fpo = fdopen(sock_fd, "w")) == NULL)
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
            for(i = 0; i < interact; i++){
                // receive filename
                memset(buf, 0, BUFSIZ);
                strcpy(filename, "rrmbin/");
                n_read = read(sock_fd, buf, BUFSIZ);
                strcat(filename, buf);
                printf("filename %s\n", filename);

                // check if the requested file exists
                if (stat(filename, &st) == -1){
                    oops("requested file doesn't exist");
                }
                // send file
                fp = fopen(filename, "rb");
                fsize = send_file(sock_fd, fp);
                printf("File %s (size: %lu) has been restored to client %s\n", filename, fsize, clntaddr);
                fclose(fp);

                // delete file after transfer
                if (remove(filename) != 0) 
                    oops("file delete error");
            }
            close(sock_fd);
        }
        else{
            oops("wrong option");
        }
        close(sock_fd);
    }
    close(sock_id);
    return 0;
}