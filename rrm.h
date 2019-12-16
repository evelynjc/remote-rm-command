#define PORT 13000
#define F_BUFSIZ 4096
#define oops(msg)           \
    {                       \
        perror(msg);        \
        printf("\n");       \
        exit(1);            \
    }

#define HOSTLEN 256
#define MAX_LISTEN_QUEUE 5

ssize_t recv_file(int, FILE *);
ssize_t send_file(int, FILE *);

ssize_t send_file(int sock, FILE *fp){
    int n, f_len = 0;
    char fbuf[F_BUFSIZ];
    memset(fbuf, 0, F_BUFSIZ);
    while ((n = fread(fbuf, sizeof(char), F_BUFSIZ, fp)) > 0){
        f_len += n;
        if (n != BUFSIZ && ferror(fp)){
            oops("Read File Error");
        }
        //printf("buf: %s\n", fbuf);
        if (send(sock, fbuf, n, 0) == -1){
            oops("Can't send file");
        }
        memset(fbuf, 0, F_BUFSIZ);
    }
    return f_len;
}
ssize_t recv_file(int sock, FILE *fp){
    ssize_t n, f_len = 0;
    char fbuf[F_BUFSIZ];
    memset(fbuf, 0, F_BUFSIZ);
    while ((n = recv(sock, fbuf, F_BUFSIZ, 0)) > 0){
	    f_len += n;
        if (n == -1){
            oops("recv_file() - recv() error");
        }
        //printf("buf: %s\n", fbuf);
        if (fwrite(fbuf, sizeof(char), n, fp) != n){
            oops("recv_file() - fwrite() error");
        }
        memset(fbuf, 0, F_BUFSIZ);
    }
    return f_len;
}