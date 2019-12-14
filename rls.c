/* rls.c */
/* a client for a remote directory listing service */
/* usage: rls hostname directory */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define oops(msg)    \
    {                \
        perror(msg); \
        exit(1);     \
    }
#define PORT 13000
#define F_BUFSIZ 4096
char option = 'x';
int main(int ac, char *av[])
{
    struct sockaddr_in servadd;
    struct hostent *hp;
    int sock_id; // sock_fd;
    int n_interact;
    char buf[BUFSIZ];
    char filename[BUFSIZ];
    int n_read, n_write, i;
    FILE *f_to_remove, *f_to_restore;
    char filebuf[F_BUFSIZ];
    int f_n_read;

    /* arguments parsing */
    if (ac < 3)
    {
        printf("** rrm - a remote recycle bin service **\n\n");
        printf(" (1) if you wish to delete file(s)\n");
        printf("    usage: ./rrm [HOSTNAME] [FILE]..\n\n");
        printf(" (2) if you wish to list the file(s) in the bin\n");
        printf("    usage: ./rrm [HOSTNAME] -v\n\n");
        printf(" (3) if you wish to restore file(s) from the bin\n");
        printf("    usage: ./rrm [HOSTNAME] [OPTION].. [FILE]..\n");
        printf("    -r      restore FILE(s) to the current directory\n\
            ./rls [HOSTNAME] -r [FILE]..\n");
        printf("    -p      path, restore FILE(s) to the specified destination\n\
            ./rls [HOSTNAME] -p [PATH] [FILE]..\n\n");
        exit(1);
    }
    else if (ac == 3 && strcmp(av[2], "-v") == 0)
    {
        option = 'v';
    }
    else if (strcmp(av[2], "-r") == 0)
    {
        option = 'r';
    }
    else if (strcmp(av[2], "-p") == 0)
    {
        option = 'p';
    }

    /******************** SOCKET ************************/
    sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_id == -1)
        oops("socket");

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

    /* main command execution depending on the option */
    if (option == 'v')
    {
        // list files
    }
    else if (option == 'r')
    {
        // restore
    }
    else if (option == 'p')
    {
        // set dest path, and restore
    }

    else {// plain rm
        // check if all the file(s) exist
        for (i = 2; i < ac; i++){
            f_to_remove = open(av[i], "rb");
            if (!f_to_remove){
                printf("cannot find file %s. abort rrm\n", av[i]);
                exit(1);
            }
            fclose(f_to_remove);
        }
        // write to server - which option, how many files
        memset(buf, 0, BUFSIZ);
        snprintf(buf, BUFSIZ, "%c %d %s", option, ac - 2, av[i]);
        if( write(sock_id, av[2], strlen(av[2])) == -1 )
            oops("write - option and how many file(s)");
        
        // send file(s)
        for (i = 2; i < ac; i++){
            while (1){
                f_to_remove = open(av[i], "rb");
                memset(filebuf, 0, F_BUFSIZ);
                f_n_read = read(f_to_remove, filebuf, F_BUFSIZ);
                printf("f_n_read: %s\n", filebuf);
                n_write = write(sock_id, filebuf, F_BUFSIZ);
                if(n_write == -1)
                    oops("write - file transfer");
                
                if (f_n_read == EOF | f_n_read == 0){
                    printf("finish reading file %s\n", av[3]);
                    break;
                }
                fclose(f_to_remove);
            }
            
        }
        close(sock_id);
    }
    /*
    // Step 1: get a socket
    sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_id == -1)
        oops("socket");
    
    // Step 2: connect to server
    bzero( &servadd, sizeof(servadd) );
    hp = gethostbyname( av[1] );
    if (hp == NULL)
        oops(av[1]);
    bcopy(hp->h_addr, (struct sockaddr *)&servadd.sin_addr, hp->h_length);
    servadd.sin_port = htons(PORT);
    servadd.sin_family = AF_INET;

    if ( connect(sock_id, (struct sockaddr*)&servadd, sizeof(servadd)) != 0)
        oops("connect");
    
    // Step3: send directory name, then read back results
    if( write(sock_id, av[2], strlen(av[2])) == -1 )
        oops("write");
    if( write(sock_id, "\n", 1) == -1 )
        oops("write");
    
    while( (n_read = read(sock_id, buffer, BUFSIZ)) > 0){
        if ( write(1, buffer, n_read)  == -1 ){
            oops("write");
        }
    }
    close(sock_id);
    */
    return 0;
}