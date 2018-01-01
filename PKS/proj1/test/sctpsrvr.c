/*
 *  sctpsrvr.c
 *
 *  SCTP multi-stream server.
 *
 *  M. Tim Jones <mtj@mtjones.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include "common.h"

char *read_file(const char *filename);

int main()
{
  int listenSock, connSock, ret;
  struct sockaddr_in servaddr;
  struct sctp_initmsg initmsg;
  char buffer[MAX_BUFFER+1];
  char *serverc = NULL;
  char *clientc = NULL;
  time_t currentTime;

  serverc = read_file("sctpsrvr.c");
  clientc = read_file("sctpclnt.c");

  /* Create SCTP TCP-Style Socket */
  listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

  /* Accept connections from any interface */
  bzero( (void *)&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
  servaddr.sin_port = htons(MY_PORT_NUM);

  ret = bind( listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );

  /* Specify that a maximum of 5 streams will be available per socket */
  memset( &initmsg, 0, sizeof(initmsg) );
  initmsg.sinit_num_ostreams = 5;
  initmsg.sinit_max_instreams = 5;
  initmsg.sinit_max_attempts = 4;
  ret = setsockopt( listenSock, IPPROTO_SCTP, SCTP_INITMSG,
                     &initmsg, sizeof(initmsg) );

  /* Place the server socket into the listening state */
  listen( listenSock, 5 );

  /* Server loop... */
  while( 1 ) {

    /* Await a new client connection */
    printf("Awaiting a new connection\n");
    connSock = accept( listenSock, (struct sockaddr *)NULL, (int *)NULL );

    /* New client socket has connected */

    /* Grab the current time */
    currentTime = time(NULL);

    /* Send local time on stream 0 (local time stream) */
    snprintf( buffer, MAX_BUFFER, "%s\n", ctime(&currentTime) );
    ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer),
                         NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );
    /* Send GMT on stream 1 (GMT stream) */
    snprintf( buffer, MAX_BUFFER, "%s\n", asctime( gmtime( &currentTime ) ) );
    ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer),
                         NULL, 0, 0, 0, GMT_STREAM, 0, 0 );

    /* Send server source code on stream 0 */
    for(size_t i = 0; i < strlen(serverc); i += MAX_BUFFER) {
     ret = sctp_sendmsg( connSock, (void *)&serverc[i], MAX_BUFFER,
                          NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );
    }
    /* Send client source code on stream 1 */
    for(size_t i = 0; i < strlen(clientc); i += MAX_BUFFER) {
     ret = sctp_sendmsg( connSock, (void *)&clientc[i], MAX_BUFFER,
                           NULL, 0, 0, 0, GMT_STREAM, 0, 0 );
    }

    /* Close the client connection */
    close( connSock );

  }

  /* Unreachable, but "just in case" */
  free(serverc);
  free(clientc);

  return 0;
}

char *read_file(const char *filename)
{
  FILE *fd = NULL;
  const size_t chunk_size = 1024;
  size_t buf_size = chunk_size;
  size_t idx = 0;
  char *buf = NULL;
  char c = '\0';

  if((fd = fopen(filename, "r")) == NULL) {
    perror("fopen()");
    exit(EXIT_FAILURE);
  }

  buf = malloc(sizeof(*buf) * chunk_size);
  if(buf == NULL) {
    perror("malloc()");
    exit(EXIT_FAILURE);
  }

  while((c = getc(fd)) != EOF) {
    buf[idx++] = c;
    if(idx >= buf_size - 1) {
      buf_size += chunk_size;
      buf = realloc(buf,  buf_size);

      if(buf == NULL) {
          perror("realloc()");
          exit(EXIT_FAILURE);
      }
    }
  }

  buf[idx] = '\0';

  return buf;
}
