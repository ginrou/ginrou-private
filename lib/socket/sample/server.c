#include <stdio.h>
#include <stdlib.h>
#include "ipc_sub3.h"

int main(void) {
  int     port, sock, msgsock;
  int     rval;
  char    buf[256], *a;
  int     i;

  port = 5000;
  ipc3_sopen( &port, &sock );
  printf("Socket port = %d\n", port );

  for(;;) {
    ipc3_wait_connection( sock, &msgsock );
    printf("Connected.\n");
    for(;;) {
      if( (rval = ipc3_read( msgsock, buf, 256 )) == 0 )
	break;
      if( rval != 256 ) {
	printf("error :reading msg socket\n");
	exit(0);
      }
      printf("---> %s\n", buf);

      for( a = &buf[0]; *a != '\0'; a++ ) {
	if( *a >= 'a' && *a <= 'z' )
	  *a = *a - 'a' + 'A';
	else if( *a >= 'A' && *a <= 'Z' )
	  *a = *a - 'A' + 'a';
      }
      if( ipc3_write(msgsock, buf, 256 ) < 0 ) {
	printf("error: writing data\n");
	exit(0);
      }
    }
    printf("connection close.\n");
  }
}
