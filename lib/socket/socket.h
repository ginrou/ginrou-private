#ifdef __cplusplus
extern "C" {
#endif

void ipc3_sopen( int *port, int *sock );
void ipc3_wait_connection( int sock, int *msgsock );
void ipc3_connect( char servername[], int serverport, int *msgsock );
int ipc3_read( int msgsock, void *buf, int size );
int ipc3_read_atm( int msgsock, void *buf, int size );
int ipc3_write( int msgsock, void *buf, int size );
int ipc3_write_atm( int msgsock, void *buf, int size );
void ipc3_close( int sock );
void ipc3_shutdown( int sock, int how );
int ipc3_wait( int sock_array[], int entry_num, int timeout_msec );
int ipc3_wait2( int sock_array[], int entry_num, int timeout_msec );
void ipc3_nodelay( int sock, int flag );

#ifdef __cplusplus
}
#endif
