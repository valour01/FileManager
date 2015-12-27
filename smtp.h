#ifndef SMTP_INCLUDED
#define SMTP_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#define SIZE64   64
#define SIZE512  512
#define SOCKET_ERROR -1

enum ERRCODE {
        ERROR_CONNECT_SMTP = 1,
        ERROR_SEND_MESSAGE,
        ERROR_RECV_MESSAGE,
        ERROR_ILLEGAL_ADDRESS,
        ERROR_ALLOC_MEMORY
};

typedef struct _content {
        char from_name[SIZE64], from_addr[SIZE64];
        char to_name[SIZE64], to_addr[SIZE64];
        char subject[SIZE512];
        char *text;
} content_t;

extern int send_mail(const char* s_server, const char* username, const char* password, content_t* ct, char* receiver);

#endif
