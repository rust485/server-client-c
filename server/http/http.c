#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "http.h"
#include "../utils/utils.h"

// port to run my server on
#define PORT 8080

#define HTTP "HTTP/1.1 "

#define NOT_FOUND "404 NOT FOUND\r\n"
#define ENF 1

#define BAD_REQ "400 BAD REQUEST\r\n"
#define EBR 2

#define NOT_MODIFIED "304 NOT MODIFIED\r\n"
#define NM 3

#define OK "200 OK\r\n"


#define DATE "Date: "
#define LAST_MOD "Last-Modified: "
#define END_HEAD "\r\n\r\n"

/**
 * Sets up the HTTP server on portnumber PORT
 * @method setup_server
 * @return -1 if failure, otherwise the file descriptor of the generated socket
 */
int setup_server()
{
  int socketfd;
  struct sockaddr_in sadd;

  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Socket error\n");
    return -1;
  }

  memset(&sadd, 0, sizeof(sadd));
  sadd.sin_family = AF_INET;
  sadd.sin_addr.s_addr = INADDR_ANY;
  sadd.sin_port = htons(PORT);

  if (bind(socketfd, (struct sockaddr *) &sadd, sizeof(sadd)) < 0)
  {
    printf("Error binding\n");
    close(socketfd);
    return -1;
  }

  if (listen(socketfd, 5) < 0)
  {
    printf("Unable to listen\n");
    close(socketfd);
    return -1;
  }

  return socketfd;
}

/**
 * Reads the request until "\r\n\r\n" or disonnect from connfd
 * @method read_request
 * @param  connfd       the connection to read through
 * @return              the request sent by the client
 */
char *read_request(int connfd)
{
  int buff_size = 8;
  int current_size = 2 * buff_size;
  int left = current_size;
  int bytes;
  char buff[buff_size];
  memset(&buff, 0, buff_size);
  char *request = calloc(current_size, sizeof(char));
  int cont = 1;
  while (cont && (bytes = read(connfd, buff, buff_size - 1)) > 0)
  {
    if (bytes > left)
    {
      request = realloc(request, current_size * 2);
      left = current_size;
      current_size *= 2;
    }

    left -= bytes;
    buff[bytes] = '\0';
    strcat(request, buff);
    memset(&buff, 0, buff_size);

    cont = 0;
    for (int i = strlen(request) - strlen(END_HEAD), j = 0; i < strlen(request); i++, j++)
    {
      if (request[i] != END_HEAD[j])
      {
        cont = 1;
        break;
      }
    }
  }
  printf("\nRequest:\n%s", request);
  return request;
}

/**
 * Writes the requested object back through the connection socket
 * @method write_object
 * @param  connfd       connection to write object through
 * @param  f            File object to write through the connection
 * @return              0 if an error occurred, 1 otherwise
 */
int write_object(int connfd, FILE *f)
{
  int size = 20;
  int i = 0;
  char c;
  char *out = calloc(size, sizeof(char));
  while ((c = fgetc(f)) != EOF)
  {
    out[i++] = c;
    if (i == size)
    {
      if (write(connfd, out, i) < 0)
      {
        printf("Write error\n");
        return 0;
      }
      free(out);
      out = calloc(size, sizeof(char));
      i = 0;
    }
  }

  if (i > 0 && write(connfd, out, i) < 0)
  {
    printf("Write error\n");
    return 0;
  }
  free(out);

  return 1;
}

/**
 * Builds the header from the given
 * @method build_header
 * @param  path         The path of the file object requested
 * @param  err          error information, 0 if no error
 * @return              the header created
 */
char *build_header(char *path, int err)
{
  char *last_modified;

  // only add the last modified time if there are no errors
  // or if the file exists, but was not modified within the
  // given interval
  if (!err || err == NM)
  {
    struct stat attr;
    stat(path, &attr);
    char *tmp = ctime(&attr.st_mtime);
    last_modified = calloc(strlen(tmp) + 1, sizeof(char));
    strcpy(last_modified, tmp);
  }

  time_t cur = time(0);
  char *date = ctime(&cur);
  int size = strlen(HTTP) + strlen(DATE) + strlen(END_HEAD) + strlen(date);

  if (!err) size += strlen(OK) + strlen(LAST_MOD) + strlen(last_modified);
  else if (err == NM)  size += strlen(NOT_MODIFIED) + strlen(LAST_MOD) + strlen(last_modified);
  else if (err == ENF) size += strlen(NOT_FOUND);
  else if (err == EBR) size += strlen(BAD_REQ);

  char *header = calloc(size + 1, sizeof(char));
  strcat(header, HTTP);

  if (!err)
  {
    strcat(header, OK);
    strcat(header, DATE);
    strcat(header, date);
    strcat(header, LAST_MOD);
    strcat(header, last_modified);
  }
  else if (err == NM)
  {
    strcat(header, NOT_MODIFIED);
    strcat(header, DATE);
    strcat(header, date);
    strcat(header, LAST_MOD);
    strcat(header, last_modified);
  }
  else if (err == ENF)
  {
    strcat(header, NOT_FOUND);
    strcat(header, DATE);
    strcat(header, date);
  }
  else if (err == EBR)
  {
    strcat(header, BAD_REQ);
    strcat(header, DATE);
    strcat(header, date);
  }

  strcat(header, END_HEAD);
  return header;
}

/**
 * Writes a message through the connection given
 * @method write_msg
 * @param  connfd    socket to write through
 * @param  msg       the message to write
 * @return           0 if an error, 1 otherwise
 */
int write_msg(int connfd, const char *msg)
{
  int total = strlen(msg);
  int sent = 0;
  while (total > sent)
  {
    int bytes = write(connfd, msg, total - sent);
    if (bytes < 0)
    {
      printf("Write error\n");
      return 0;
    }
    sent += bytes;
  }

  return 1;
}

/**
 * Handles the request with the given socket that requested the given resource identifier
 * @method handle_request
 * @param  connfd         connection to write to and read from
 * @param  resource_id    resource requested by the client
 * @param  time_interval  null if no time interval specified, otherwise represents the
 *                        how recent the resource must be in order to return it
 * @param header_only     0 if sending body, 1 if only sending head
 * @param  err            0 if no error
 */
void handle_request(int connfd, const char *resource_id, char *time_interval, int header_only, int err)
{
  char *file = calloc(strlen(resource_id) + 2, sizeof(char));
  strcat(file, ".\0");
  strcat(file, resource_id);

  FILE *f = fopen(file, "r");
  if (!f && !err) err = ENF;

  if (!err)
  {
    // check if-modified-by
    if (time_interval)
    {
      struct stat attr;
      stat(file, &attr);
      time_t last_modified = attr.st_mtime; // last time the file was modified
      time_t *if_mod_by = parse_date(time_interval);

      // performs last_modified - if_mod_by. If last_modified is less than if_mod_by,
      // will output < 0 and the user does not want the requested object
      if (difftime(*if_mod_by, last_modified) > 0)
        err = NM;
    }
  }
  char *header = build_header(file, err);
  write_msg(connfd, header);


  if (!err && !header_only)
    write_object(connfd, f);
  //clean up
  if (f) fclose(f);
  free(header);
  free(file);
}

/**
 * Handles the passed client by reading the request, parsing the request, and responding to the request
 * @method handle_client
 * @param  conn          descriptor of the socket connection for the desired client
 */
void *handle_client(void *conn)
{
  int err = 0;
  int header_only;
  char *time_interval;
  char *resource;
  int connfd = (uintptr_t) conn;
  printf("client connected\n");
  char *req = read_request(connfd);
  // if there was a format error
  if (!(resource = parse_request(req, &time_interval, &header_only))) err = EBR;
  handle_request(connfd, resource, time_interval, header_only, err);

  free(req);
  free(time_interval);
  free(resource);
  close(connfd);
  pthread_exit(NULL);
}

/**
 * Runs the server with the given socket descriptor. Accepts clients and starts
 * a new thread to handle the client requests while the main thread listens for
 * the next client
 * @method run
 * @param  socketfd the file descriptor of the socket for this server
 * @return          0 if an error occurred
 */
int run(int socketfd)
{
  printf("Server runnin on port %d\n", PORT);
  for (;;)
  {
    struct sockaddr_in cliaddr;
    pthread_t thread;

    int connfd;
    uint size = sizeof(cliaddr);
    if ((connfd = accept(socketfd, (struct sockaddr *) &cliaddr, &size)) < 0)
    {
      printf("Error accepting\n");
      return 0;
    }
    pthread_create(&thread, NULL, handle_client, (void *)(uintptr_t)connfd);
  }

  return 1;
}
