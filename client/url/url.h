#ifndef URL_H
#define URL_H

typedef struct url
{
  char *hostname;
  char *resource_id;
  int portnumber;
} url_t;

int init_url(url_t *url, char *input);
void destroy_url(url_t *url);
void print_url(const url_t url);

#endif
