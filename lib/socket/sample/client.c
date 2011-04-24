#include <stdio.h>
#include <stdlib.h>
#include "ipc_sub3.h"

int main( int argc, char *argv[] ) {
  int     sock;
  char    buf[256];
  int     i;

  ipc3_connect( argv[1], atoi(argv[2]), &sock );

  for(;;) {
    printf("-->");
    gets( buf );
    if( strcmp( buf, "q" ) == 0 ) break;

    if( ipc3_write(sock, buf, 256 ) != 256 ) {
      printf("error: writing data\n");
      exit(0);
    }

    if( ipc3_read( sock, buf, 256 ) != 256 ) {
      printf("error: writing data\n");
      exit(0);
    }
    printf("===> %s\n", buf);
  }
  ipc3_close( sock );
}
