#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "http/http.h"

int main(int argc, char *argv[])
{
  int socketfd = setup_server();
  if (socketfd < 0)
    return 0;
  run(socketfd);
  return 1;
}
