/* TCP/IP socket library for UNIX / Windows */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "socket.h"

#if defined(_WIN32)
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#endif

#define  ATM_PACKET       8192*2

static void sockInit( void ) {
#if defined(_WIN32)
	int  nErrorStatus;
	WSADATA wsaData;
	static int init = 0;

	if(init) {
		return;
	}
	/* WinSockの初期化を行う．バージョン 1.1 を要求する */
	nErrorStatus = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (atexit((void (*)(void))(WSACleanup))) {		/* 終了時にWinSockのリソースを解放するようにしておく */
		fprintf(stderr,"atexit(WSACleanup)失敗\n");
		exit(-1);
	}
	if ( nErrorStatus != 0 ) {
		fprintf(stderr,"WinSockの初期化失敗\n");
		exit(-1);
	}
	init = 1;
#endif
}

void ipc3_sopen( int *port, int *sock ) {
    struct sockaddr_in  server;
    int w_sock, w_port, length;

    sockInit();

    w_sock = socket(AF_INET, SOCK_STREAM, 0);
    if( w_sock < 0 ) {
        printf("error :opening stream socket\n");
        exit(0);
    }

    w_port = *port;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( w_port );
    if( bind(w_sock, (struct sockaddr *)&server, sizeof(server) ) < 0 ) {
        printf("error :binding stream socket\n");
        exit(0);
    }

    if( w_port == 0 ) {
        length = sizeof( server );
        if( getsockname(w_sock, (struct sockaddr *)&server, (socklen_t *)&length) < 0 ) {
            printf("error :geting socket name\n");
            exit(0);
        }
        *port = ntohs(server.sin_port);
    }
    *sock = w_sock;

    listen( w_sock, 5 );
}

void ipc3_wait_connection( int sock, int *msgsock ) {
    int     w_msgsock;

    sockInit();

    w_msgsock = accept( sock, (struct sockaddr *)0, (socklen_t *)0);
    if( w_msgsock == -1 ) {
            printf("error :accepting msg socket\n");
            exit(0);
    }

    *msgsock = w_msgsock;
}

void ipc3_connect( char servername[], int serverport, int *msgsock ) {
    struct sockaddr_in  server;
    struct hostent      *hp;
    int                 w_sock;

    sockInit();

    w_sock = socket(AF_INET, SOCK_STREAM, 0);
    if( w_sock < 0 ) {
        printf("error :opening stream socket\n");
        exit(0);
    }

    server.sin_family = AF_INET;
    hp = gethostbyname(servername);
    if( hp == 0 ) {
        printf("unknown host\n");
        exit(0);
    }
    server.sin_addr = *((struct in_addr *)(hp->h_addr_list[0]));
    server.sin_port = htons(serverport);

    if( connect(w_sock, (struct sockaddr *)&server, sizeof(server) ) < 0 ) {
        printf("error :connecting stream socket\n");
        exit(0);
    }

    *msgsock = w_sock;
}

int ipc3_read( int msgsock, void *vbuf, int size ) {
    int             valid_size;
    int             rval;
    unsigned char *buf = (unsigned char *)vbuf;
	
    valid_size = 0;
    while( valid_size < size ) {
#if defined(_WIN32)
        if( (rval=recv(msgsock, (char *)&buf[valid_size], size-valid_size, 0)) < 0 ) {
#else
        if( (rval=read(msgsock, (char *)&buf[valid_size], size-valid_size)) < 0 ) {
#endif
            return( 0 );
        }
        if( rval == 0 ) {
#if defined(_WIN32)
            closesocket( msgsock );
#else
            close( msgsock );
#endif
            return( valid_size );
        }
        valid_size += rval;
    }

    return( valid_size );
}

int ipc3_read_atm( int msgsock, void *vbuf, int size ) {
    unsigned char   *data;
    unsigned char   ack[8];
    int             r_size, s_size;
    int             rval;
    unsigned char *buf = (unsigned char *)vbuf;

    data = buf;
    r_size = size;
    while( r_size ) {
        if( r_size >= ATM_PACKET ) s_size = ATM_PACKET;
         else                      s_size = r_size;
        rval = ipc3_read(msgsock, (char *)data, s_size );
        if( rval == 0 ) return( 0 );

        r_size -= s_size;
        data += s_size;
        if( ipc3_write(msgsock, ack, 4 ) != 4 ) return( 0 );
    }

    return( size );
}

int ipc3_write( int msgsock, void *vbuf, int size ) {
    int             valid_size;
    int             rval;
    unsigned char   *buf = (unsigned char *)vbuf;

    valid_size = 0;
    while( valid_size < size ) {
#if defined(_WIN32)
        if( (rval=send(msgsock, (char *)&buf[valid_size], size-valid_size, 0)) < 0 ) {
#else
        if( (rval=write(msgsock, &buf[valid_size], size-valid_size)) < 0 ) {
#endif
            return( 0 );
        }
        if( rval == 0 ) {
#if defined(_WIN32)
            closesocket( msgsock );
#else
            close( msgsock );
#endif
            return( valid_size );
        }
        valid_size += rval;
    }

    return( valid_size );
}

int ipc3_write_atm( int msgsock, void *vbuf, int size ) {
    unsigned char   *data;
    unsigned char   ack[4];
    int             r_size, s_size;
    int             rval;

    data = (unsigned char *)vbuf;
    r_size = size;
    while( r_size ) {
        if( r_size >= ATM_PACKET ) s_size = ATM_PACKET;
         else                      s_size = r_size;
        rval = ipc3_write(msgsock, data, s_size );
        if( rval == 0 ) return( 0 );

        r_size -= s_size;
        data += s_size;
        if( ipc3_read(msgsock, ack, 4 ) != 4 ) return( 0 );
    }

    return( size );
}

void ipc3_close( int sock ) {
#if defined(_WIN32)
            closesocket( sock );
#else
            close( sock );
#endif
}

void ipc3_shutdown( int sock, int how ) {
    shutdown( sock, how );
}

int ipc3_wait( int sock_array[], int entry_num, int timeout_msec ) {
    fd_set          readfds, writefds, exceptfds;
    struct timeval  timeout;
    int             nfds;
    int             ret;
    int             i;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    nfds = 0;
    for( i = 0; i < entry_num; i++ ) {
        FD_SET( sock_array[i], &readfds );
        if( sock_array[i] > nfds ) nfds = sock_array[i];
    }
    nfds++;
    timeout.tv_sec = timeout_msec/1000;
    timeout.tv_usec = (timeout_msec%1000)*1000;

    ret = select( nfds, &readfds, &writefds, &exceptfds, &timeout );
    if( ret <= 0 ) {
        return(0);
    }

    for( i = 0; i < entry_num; i++ ) {
        if( FD_ISSET( sock_array[i], &readfds ) ) return( i+1 );
    }
    return(0);
}

int ipc3_wait2( int sock_array[], int entry_num, int timeout_msec ) {
    fd_set          readfds, writefds, exceptfds;
    struct timeval  timeout;
    int             nfds;
    int             ret;
    int             i;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    nfds = 0;
    for( i = 0; i < entry_num; i++ ) {
        FD_SET( sock_array[i], &readfds );
        if( sock_array[i] > nfds ) nfds = sock_array[i];
    }
    nfds++;
    timeout.tv_sec = timeout_msec/1000;
    timeout.tv_usec = (timeout_msec%1000)*1000;

    ret = select( nfds, &readfds, &writefds, &exceptfds, &timeout );
    if( ret <= 0 ) {
        return(0);
    }

    for( i = 0; i < entry_num; i++ ) {
        sock_array[i] = (FD_ISSET(sock_array[i], &readfds))? 1: 0;
    }
    return(1);
}

void ipc3_nodelay( int sock, int flag ) {
    int     on_off;

    if( flag ) on_off = 1;
     else      on_off = 0;

#if defined(_WIN32)
    setsockopt( sock, 6, TCP_NODELAY, (char FAR *)&on_off, 4 );
#else
    setsockopt( sock, 6, TCP_NODELAY, &on_off, 4 );
#endif
}
