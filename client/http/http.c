#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "http.h"

#define GET "GET "
#define HEAD "HEAD "
#define HTTPV " HTTP/1.1\r\n"
#define HOST "Host: "
#define END_HEAD "\r\n\r\n"
#define IF_MOD "\r\nIf-Modified-Since: "


/**
 * Builds an http request based of the url, the desired time interval, and if the
 * user wants the header only. Format is:
 *
 * <command> <resource identifier> HTTP/1.1
 * Host: <hostname>
 * [If-Modified-Since: <date>]
 *
 * @method build_request
 * @param  url           the url object containing the port number, resource id, and host name
 * @param  time_interval the date to check if the file has been altered since
 * @param  header_only   true if the client only wishes to receive the header without the object
 * @return               the message built of the above format
 */
char *build_request(const url_t url, char *time_interval, int header_only)
{
  // size of the header command
  int command_size = strlen(GET);
  if (header_only) command_size = strlen(HEAD);

  // only use extra for the time_interval, including If-Modified-Since: <date>
  int extra = 0;
  if (time_interval) extra = strlen(time_interval) + strlen(IF_MOD);

  char *request = calloc(command_size + strlen(HTTPV) + strlen(HOST) + strlen(END_HEAD) +
                  strlen(url.hostname) + strlen(url.resource_id) + extra,
                  sizeof(char));

  if (header_only) strcat(request, HEAD);
  else strcat(request, GET);

  strcat(request, url.resource_id);
  strcat(request, HTTPV);
  strcat(request, HOST);
  strcat(request, url.hostname);
  if (time_interval)
  {
    strcat(request, IF_MOD);
    strcat(request, time_interval);
  }
  strcat(request, END_HEAD);

  return request;
}

/**
 * Establishes a TCP connection between this client and the host residing at url.hostname with
 * port number url.portnumber
 * @method establish_connection
 * @param  url                  URL containing the hostname and port information to connect with
 * @return                      the socket descriptor to use for communication with the server
 */
int establish_connection(const url_t url)
{
  printf("Attempting to connect to: ");
  print_url(url);

  int socketfd;
  struct sockaddr_in servadd;
  struct hostent *server;

  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("socket failed\n");
    return -1;
  }

  if (!(server = gethostbyname(url.hostname)))
  {
    printf("No such host\n");
    close(socketfd);
    return -1;
  }

  memset(&servadd, 0, sizeof(servadd));
  servadd.sin_family = AF_INET;
  servadd.sin_port = htons(url.portnumber);
  memcpy(&servadd.sin_addr, server->h_addr, server->h_length);

  if (connect(socketfd, (struct sockaddr *)&servadd, sizeof(servadd)) < 0)
  {
    printf("Connection failed\n");
    close(socketfd);
    return -1;
  }

  printf("Established connection\n");

  return socketfd;
}

/**
 * Sends the generated request through the socket file descriptor
 * @method send_request
 * @param  socketfd     the socket descriptor to send the message through
 * @param  request      the pre-generated request to be sent through the socket
 * @return              0 if an error occurred, 1 otherwise
 */
int send_request(int socketfd, char *request)
{
  int total = strlen(request);
  int bytes = 0;
  int size;
  while (bytes < total)
  {
    if ((size = write(socketfd, request + bytes, total - bytes)) < 0)
    {
      printf("Error sending message\n");
      return 0;
    }

    bytes += size;
  }

  printf("request sent\n");
  return 1;
}

/**
 * Reads the servers response through the socket descriptor and stores the response
 * inside of the opened file with file descriptor filefd
 * @method receive_response
 * @param  socketfd         the socket to read the message through
 * @param  filefd           the descriptor for the open file to write to
 * @return                  void
 */
void receive_response(int socketfd, FILE *filefd)
{
  int size;
  int length = 4096;
  char *buff = calloc(length, sizeof(char));
  while ((size = read(socketfd, buff, length)) > 0)
  {
    fwrite(buff, sizeof(char), size, filefd);
    memset(buff, 0, length);
  }
  free(buff);
}
