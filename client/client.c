#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "http/http.h"
#include "utils/utils.h"
#include "url/url.h"

// file to output to
#define OUTPUT "response"

int main(int argc, char *argv[])
{
  url_t *url = malloc(sizeof(url));
  char *time_interval = NULL;
  char *req;
  int header_only, socketfd;

  // parse the user input
  if (!parse_input(argc, argv, url, &header_only, &time_interval)) return 0;
  // establish the connection
  if ((socketfd = establish_connection(*url)) < 0)
    return 0;

  // builds the request based on user input
  if (!(req = build_request(*url, time_interval, header_only)))
  {
    close(socketfd);
    return 0;
  }

  // sends the request to the requested server
  if (!send_request(socketfd, req))
  {
    close(socketfd);
    destroy_url(url);
    return 0;
  }

  // open file to write response to
  remove(OUTPUT);
  FILE *out = fopen(OUTPUT, "a");

  // receive the response from the server
  receive_response(socketfd, out);

  // clean up
  fclose(out);
  close(socketfd);
  destroy_url(url);

  return 1;
}
