#ifndef HTTP_H
#define HTTP_H

#include "../url/url.h"

char *build_request(const url_t, char *, int);
int establish_connection(const url_t url);
int send_request(int, char *);
void receive_response(int, FILE *);

#endif
