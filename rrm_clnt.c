/************** rrm_clnt.c **************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include "rrm.h"

char option = 'x';
int n_files = 0;
char destpath[BUFSIZ] = "";
int fname_idx = 0;

void opt_parse(int, char **);
int main(int ac, char *av[]){
    struct sockaddr_in myaddr, servadd;
    struct hostent *hp;
    int sock_id;
    char buf[BUFSIZ];
    int n_read, n_write, i;
    FILE *fp;
    ssize_t fsize;

    /* arguments parsing */
    opt_parse(ac, av);

    /******************** SOCKET ************************/
    sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_id == -1)
        oops("socket");
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(32768);
    /********************** BIND ************************/
    if(bind(sock_id, (struct sockaddr*)&myaddr, sizeof(myaddr)) != 0)
        oops("bind");
    /******************** CONNECT ************************/
    bzero(&servadd, sizeof(servadd));
    hp = gethostbyname(av[1]);
    if (hp == NULL)
        oops(av[1]);
    bcopy(hp->h_addr, (struct sockaddr *)&servadd.sin_addr, hp->h_length);
    servadd.sin_port = htons(PORT);
    servadd.sin_family = AF_INET;
    if (connect(sock_id, (struct sockaddr *)&servadd, sizeof(servadd)) != 0)
        oops("connect");

    // initial write() - option and the number of files to be handled
    memset(buf, 0, BUFSIZ);
    sprintf(buf, "%c %d", option, n_files);
    n_write = write(sock_id, buf, BUFSIZ);
    if(n_write == -1)
        oops("failed ending OPTION and C-S iteration count");
    
    /* main command execution depending on the option */
    if (option == 'x'){
        /* removing file(s) */
        // check if all the files exist
        for (i = 2; i < ac; i++){
            fp = fopen(av[i], "rb");
            if (!fp){
                printf("cannot find file %s. abort rrm\n", av[i]);
                exit(1);
            }
            fclose(fp);
        }
        // send file(s)
        for (i = 2; i < ac; i++){
            fp = fopen(av[i], "rb");
            if(!fp) oops("can't open file");
            // send the filename to the server
            memset(buf, 0, BUFSIZ);
            sprintf(buf, "%s", av[i]);
            n_write = write(sock_id, buf, BUFSIZ);
            if(n_write == -1)
                oops("send filename fail");
            // send the file to the server
            fsize = send_file(sock_id, fp);
            printf("File %s (size: %lu) has been removed\n", av[i], fsize);
            fclose(fp);
            // delete file after transfer
            if (remove(av[i]) != 0) 
                oops("file delete error");
        }
    }
    else if (option == 'v'){
        printf("\n/**************** Recycle Bin *******************/\n");
        while((n_read = read(sock_id, buf, BUFSIZ)) > 0)
            if(write(1, buf, n_read) == -1)
                oops("write ls -la results");
    }
    else if (option == 'r'){
        // restore file(s)
        for (i = fname_idx; i < ac; i++){
            // send the filename to the server
            char filename[BUFSIZ] = "";
            memset(buf, 0, BUFSIZ);
            sprintf(buf, "%s", av[i]);
            n_write = write(sock_id, buf, BUFSIZ);
            if(n_write == -1)
                oops("send filename fail");
            
            // if destination path is given
            if(strcmp(destpath, "") != 0){ 
                
                if(destpath[strlen(destpath) - 1] != '/'){
                    strcat(destpath, "/");
                }
            }
            // receive file
            strcpy(filename, destpath);
            strcat(filename, av[i]);
            fp = fopen(filename, "wb");
            fsize = recv_file(sock_id, fp);
            printf("file: %s, file size received: %lu\n", av[i], fsize);
            fclose(fp);
        }
    }
    else{
        oops("wrong option");
    }
    close(sock_id);
    return 0;
}
void opt_parse(int ac, char **av){
    int i, k = 0;

    if (ac < 3){
        printf("** rrm - a remote recycle bin service **\n\n");
        printf(" (1) if you wish to delete file(s)\n");
        printf("    usage: ./rrm [HOSTNAME] [FILE]..\n\n");
        printf(" (2) if you wish to list the file(s) in the bin\n");
        printf("    usage: ./rrm [HOSTNAME] -v\n\n");
        printf(" (3) if you wish to restore file(s) from the bin\n");
        printf("    usage: ./rrm [HOSTNAME] [OPTION].. [FILE]..\n");
        printf("    -r      restore FILE(s) to the current directory\n\
            ./rrm [HOSTNAME] -r [FILE]..\n");
        printf("    -p      path, restore FILE(s) to the specified destination\n\
            ./rrm [HOSTNAME] .. -p [PATH] ..\n\n");
        printf("    To use both of the options,\n    ./rrm [HOSTNAME] -r [FILE].. -p [PATH]\n    OR\n    ./rrm [HOSTNAME] -p [PATH] -r [FILE]..\n\n");
        exit(1);
    }
    else if (ac == 3 && (strcmp(av[2], "-v") == 0) ){
        option = 'v';
        return;
    }
    else{
        if(strcmp(av[2], "-r") == 0){
            fname_idx = 3;
            if(!av[3] || strcmp(av[3], "-p") == 0 )  oops("missing filename(s) to restore");
            option = 'r';
            // -p scan
            for(i = 4; i < ac; i++){
                if( strcmp(av[i], "-p") == 0 ){
                    if(av[i + 1]){
                        n_files = i - 3;
                        strcpy(destpath, av[i + 1]);
                        k = 1;
                        break;
                    }
                    else{
                        oops("the destination filepath is missing");
                    }
                }
            }
            if (k == 0){
                n_files = ac - 3;
            }
        }
        else if(strcmp(av[2], "-p") == 0){
            if(ac < 6)  oops("need more arguments");
            if(strcmp(av[4], "-r") == 0){
                option = 'r';
                n_files = ac - 5;
                fname_idx = 5;
                strcpy(destpath, av[3]);
            }
            else{
                oops("option error. -r missing or more than one destination path");
            }
        }
        else{
            for(i = 3; i < ac; i++){
                if(strcmp(av[i], "-p") == 0 || strcmp(av[i], "-r") == 0)
                    oops("option arguments must come after ./rrm [HOSTNAME]");
            }
            n_files = ac - 2;
        }
    }
    printf("option: %c, -p?: %d, file arg starts at: %d, n_files: %d, destpath: %s\n", option, k, fname_idx, n_files, destpath);
}